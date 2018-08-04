#include "manager.hpp"
#include "aux/equations.hpp"

Manager::Manager() : data(0), width(0), height(0), frameCounter(0), msg_thread(&Manager::send_cmd_thread, this) {
    fm.set_label("ImageView");
    fm.add(interface.imageView);

    notebook.append_page(interface, "Capture");
    notebook.append_page(interface.robotGUI, "Robot");
    notebook.append_page(interface.visionGUI, "Vision");
    notebook.append_page(control, "Control");
    notebook.append_page(strategyGUI, "Strategy");

    KalmanFilter kf;
    for(int i = 0; i < 4; i++) {
        KF_RobotBall.push_back(kf);
    }

    for(int i = 0; i < 3; i++) {
        fixed_ball[i]=false;
        // valores absolutos pois são só pra incializar
        virtual_robots_orientations[i] = cv::Point(320, 240);
        virtual_robots_positions[i] = cv::Point(200, 480 / 6 + (i + 1) * 480 / 6);
    }

    for(int i = 0; i < 4; i++) {
        interface.imageView.adjust_mat[i][0] = -1;
        interface.imageView.adjust_mat[i][1] = -1;
    }

    camera_vbox.pack_start(fm, false, true, 10);
    camera_vbox.pack_start(info_fm, false, true, 10);
    info_fm.add(interface.info_hbox);

    pack_start(camera_vbox, true, true, 10);
    pack_start(notebook, false, false, 10);

    interface.signal_start().connect(sigc::mem_fun(*this, &Manager::_hdl_start_capture));
    interface._evt_play.connect(boost::bind(&Manager::_hdl_game_play, this, _1));
}

Manager::~Manager(){
    con.disconnect();
    interface.imageView.disable_image_show();
    free(data);

    data = 0;

    msg_thread.interrupt();
	if (msg_thread.joinable()) msg_thread.join();

	interface._evt_play.disconnect(this);
}

bool Manager::_hdl_start_capture(bool b) {
    if (b) {
        debug_log("Starting capture...");

        if (data) {
            interface.imageView.disable_image_show();
            free(data);
            data = 0;
        }
        width = interface.vcap.format_dest.fmt.pix.width;
        height = interface.vcap.format_dest.fmt.pix.height;
        strategyGUI.strategy.set_constants(width,height);

        interface.visionGUI.setFrameSize(width, height);

        // Liberar os botões de edit
        interface.robotGUI.enable_buttons();
        strategyGUI.formation_box.set_sensitive(true);
        strategyGUI.bt_createFormation.set_sensitive(true);

        data = (unsigned char *) calloc(interface.vcap.format_dest.fmt.pix.sizeimage, sizeof(unsigned char));

        interface.imageView.set_size_request(width, height);
        con = Glib::signal_idle().connect(sigc::mem_fun(*this, &Manager::idle_loop));

        debug_log("Capture started.");

    } else {
        con.disconnect();

        // Travar os botões de edit
        interface.robotGUI.disable_buttons();
        strategyGUI.formation_box.set_sensitive(false);
        strategyGUI.bt_createFormation.set_sensitive(false);

        debug_log("Capture Stopped.");
    }

    interface.visionGUI.vision->preloadHSV();

    interface.__event_bt_quick_load_clicked();

    return true;
}

bool Manager::idle_loop() {
    // ----------------- CONFIGURAÇÃO INICIAL -----------------//
    if (!data) return false;
    if (frameCounter == 0) timer.start();

    frameCounter++;

    // ----------------- CAPTURA -----------------//

    interface.vcap.grab_rgb(data); // pega dados da câmera
    interface.imageView.set_data(data, width, height); // coloca dados no canvas da interface
    interface.imageView.refresh(); // mostra os dados

    cv::Mat cameraFlow(height, width, CV_8UC3, data); // cria a imagem que será utilizada na visão

    // ----------------- FLAGS DE INTERFACE -----------------//

    if(interface.imageView.hold_warp) {
        interface.warped = true;
        interface.bt_adjust.set_state(Gtk::STATE_NORMAL);
        interface.imageView.warp_event_flag = false;
        interface.imageView.hold_warp = false;
    }

    if (interface.reset_warp_flag) {
        interface.imageView.warp_counter = 0;
        interface.reset_warp_flag = false;
    }

    interface.imageView.split_flag = interface.visionGUI.getIsSplitView();
    interface.imageView.PID_test_flag = control.PID_test_flag;
    interface.imageView.formation_flag = strategyGUI.formation_flag;
    interface.imageView.adjust_event_flag = interface.adjust_event_flag;

    if (interface.imageView.sector != -1) {
        interface.visionGUI.selectFrame(interface.imageView.sector);
        interface.imageView.sector = -1;
    }

    // ----------------- PREPARAÇÃO DA VISÃO -----------------//

    if(interface.warped) {
        interface.bt_warp.set_active(false);
        interface.bt_warp.set_state(Gtk::STATE_INSENSITIVE);
        warp_transform(cameraFlow);
        interface.imageView.warp_event_flag=false;

        if(interface.invert_image_flag)
            cv::flip(cameraFlow,cameraFlow, -1);
    }

    // ----------------- VISÃO -----------------//

    interface.visionGUI.vision->run(cameraFlow); // executa a visão

    advRobots = interface.visionGUI.vision->getAdvs();

    // atualiza a interface de acordo com o tipo de view (simples ou split)
    if (interface.visionGUI.getIsSplitView()) {
        interface.imageView.set_data(interface.visionGUI.vision->getSplitFrame().clone().data, width, height);
        interface.imageView.refresh();
    } else if (interface.visionGUI.HSV_calib_event_flag) {
        interface.imageView.set_data(interface.visionGUI.vision->getThreshold(interface.visionGUI.Img_id).data, width, height);
        interface.imageView.refresh();
    }

    // roda os filtros e atualiza a posição dos robos na interface
    filter_positions();

    // ----------------- CONTROLE -----------------//

    // !TODO criar um evento "disable test controls" que é chamado quando os botões de PID test ou Formation Creation forem pressionados
    // se não está em estado de jogo
    if(!interface.get_start_game_flag()) {
        // se a flag de pid test está ativa
        if (control.PID_test_flag) {
            control.button_PID_Test.set_active(true);
            Robots::reset_commands();
            PID_test();
        }
        // se a flag de formação está ativa
        else if(strategyGUI.formation_flag) {
            // se o "load" foi pressionado
            if (strategyGUI.updating_formation_flag)
                updating_formation();

            formation_creation();
        }
        // se flags de controle não foram ativadas
        else {
            Robots::reset_commands();
            Selec_index = -1;
            strategyGUI.updating_formation_flag = false;
            // control.PID_test_flag = false;
        }
    }

    // ----------------- ESTRATEGIA -----------------//

    // só roda a estratégia se o play tiver sido dado
    else {
        strategyGUI.strategy.set_Ball(Ball_kf_est); // interface.visionGUI.vision->getBall()
        Ball_Est = strategyGUI.strategy.get_Ball_Est();

        // roda a estratégia
        // debug_log("Running strategy.");
        strategyGUI.strategy.get_targets(advRobots);

        // atualiza a interface
        interface.robotGUI.update_speed_progressBars();
        interface.robotGUI.update_robot_functions();
    } // if start_game_flag

    // ----------------- COMUNICAÇÃO -----------------//
    if (interface.get_start_game_flag() || control.PID_test_flag || strategyGUI.updating_formation_flag) {
        control.update_msg_time();
        notify_data_ready();
    }

    // ----------------- DESENHA SOBRE A IMAGEM  -----------------//

    if(!interface.draw_info_flag && !interface.visionGUI.getIsSplitView()) {

        // ----------- DESENHOS DA VISÃO ----------- //

        if (!interface.visionGUI.HSV_calib_event_flag) {
            // bola
            circle(cameraFlow, Ball_kf_est, 7, cv::Scalar(255,255,255), 2);

            for (int i = 0; i < Robots::SIZE; i++) {
                // orientação do robô
                line(cameraFlow, Robots::get_position(i), Robots::get_secondary_tag(i),cv::Scalar(255,255,0), 2);
                // id do robô
                putText(cameraFlow, std::to_string(i+1),cv::Point(Robots::get_position(i).x-5,Robots::get_position(i).y-17),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(255,255,0),2);
                // marcação do robô
                circle(cameraFlow, Robots::get_position(i), 15, cv::Scalar(255,255,0), 2);

                // vetor que todos os robos estão executando
                cv::Point aux_point;
                aux_point.x = int(100*cos(Robots::get_transAngle(i)));
                aux_point.y = - int(100*sin(Robots::get_transAngle(i)));
                aux_point += Robots::get_position(i);
                arrowed_line(cameraFlow, Robots::get_position(i), aux_point, cv::Scalar(255, 0, 0), 2);
            }

            // adversários
            for(int i = 0; i < advRobots.size(); i++)
                circle(cameraFlow, advRobots.at(i), 15, cv::Scalar(0,0,255), 2);
        }

        // ----------- DESENHOS DA ESTRATÉGIA ---------- //

        if(interface.get_start_game_flag()) {
            // desenha a estimativa da bola
            circle(cameraFlow, Ball_Est, 7, cv::Scalar(255,140,0), 2);

            // desenha os alvos dos robôs
            for(int i = 0; i < Robots::SIZE; i++) {
                circle(cameraFlow, Robots::get_target(i), 7, cv::Scalar(127,255,127), 2);
                putText(cameraFlow, std::to_string(i+1), cv::Point(Robots::get_target(i).x-5, Robots::get_target(i).y-17),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(127,255,127),2);
            }
        }

        // ----------- DESENHOS DO CONTROLE ---------- //

        if(strategyGUI.formation_flag) {
            // exibe os robos virtuais
            for(int i = 0; i < Robots::SIZE; i++) {
                if(virtual_robot_selected == i)
                    circle(cameraFlow,virtual_robots_positions[i], 20, cv::Scalar(0,255,100), 3);

                // posição
                circle(cameraFlow,virtual_robots_positions[i], 17, cv::Scalar(0,255,0), 2);
                // orientação
                arrowed_line(cameraFlow, virtual_robots_positions[i], virtual_robots_orientations[i], cv::Scalar(0, 255, 0), 2);
                // identificação
                putText(cameraFlow, std::to_string(i+1),virtual_robots_positions[i] + cv::Point(-14,10),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(0,255,0),2);
            }
        }

        if (interface.imageView.PID_test_flag) {
            for(int i = 0; i < Robots::SIZE; i++) {
                if(Robots::is_target_set(i)) {
                    // linha branca no alvo sendo executado
                    line(cameraFlow, Robots::get_position(i), Robots::get_target(i), cv::Scalar(255,255,255),2);
                }
                // círculo branco no alvo sendo executado
                circle(cameraFlow,Robots::get_target(i), 9, cv::Scalar(255,255,255), 2);
            }

            if(Selec_index != -1)
                circle(cameraFlow, Robots::get_position(Selec_index), 17, cv::Scalar(255,255,255), 2);
        }
    } // if !draw_info_flag

    // ----------------------------------------//

    if (frameCounter == 30) {
        timer.stop();
        fps_average = 30 / timer.getCronoTotalSecs();
        // cout<<"CPU Time: "<<timer.getCPUTotalSecs()<<",	\"CPU FPS\": "<<30/timer.getCPUTotalSecs()<<endl;
        // cout<<"FPS Time: "<<timer.getCronoTotalSecs()<<", FPS: "<<30/timer.getCronoTotalSecs()<<endl;
        timer.reset();
        frameCounter = 0;
    }

    return true;
} // idle_loop

void Manager::send_cmd_thread() {
	boost::unique_lock<boost::mutex> lock(data_ready_mutex);
	while (true) {
		try {
			data_ready_cond.wait(lock, [this]() { return data_ready_flag; });
		} catch (...) {
			lock.unlock();
			return;
		}
		data_ready_flag = false;
		control.FM.sendCMDs(Robots::get_commands());
	}
}

void Manager::notify_data_ready() {
	data_ready_flag = true;
	data_ready_cond.notify_all();
}

void Manager::_hdl_game_play(bool started) {
    if(started) {
        debug_log("Game Started");
    } else {
        debug_log("Game Paused");
        control.check_robot_status(); // pede pro controle verificar o status dos robôs
    }
}

void Manager::arrowed_line(cv::Mat img, cv::Point pt1, cv::Point pt2, const cv::Scalar &color,
                           int thickness, int line_type, int shift, double tipLength) {

    const double tipSize = norm(pt1-pt2)*tipLength;
    line(img, pt1, pt2, color, thickness, line_type, shift);
    const double angle = atan2( (double) pt1.y - pt2.y, (double) pt1.x - pt2.x );
    cv::Point p(cvRound(pt2.x + tipSize * cos(angle + CV_PI / 4)),
    cvRound(pt2.y + tipSize * sin(angle + CV_PI / 4)));
    line(img, p, pt2, color, thickness, line_type, shift);
    p.x = cvRound(pt2.x + tipSize * cos(angle - CV_PI / 4));
    p.y = cvRound(pt2.y + tipSize * sin(angle - CV_PI / 4));
    line(img, p, pt2, color, thickness, line_type, shift);

}

void Manager::updating_formation() {
    for(int i = 0; i < Robots::SIZE; i++) {
        if(EQ::distance(Robots::get_position(i), virtual_robots_positions[i]) > ROBOT_RADIUS/3.0) {
            Robots::set_command(i, virtual_robots_positions[i]);
            Robots::manage_velocity(i, virtual_robots_positions[i]);
        } else {
            // olhar pro alvo até o operador cancelar o posicionamento (caso o robô se posicione antes, a mensagem é ignorada)
            Robots::look_at(i, virtual_robots_orientations[i]);
        }
    }
}

void Manager::formation_creation() {
    if (interface.get_start_game_flag() || control.PID_test_flag) return;

    // !TODO usar evento
    // atualiza a posição dos robôs virtuais caso seja carregado uma configuração diferente
    if(strategyGUI.update_interface_flag) {
        for(int i = 0; i < Robots::SIZE; i++) {
            virtual_robots_positions[i] = strategyGUI.formation_positions[i];
            virtual_robots_orientations[i] = strategyGUI.formation_orientations[i];
        }
        strategyGUI.update_interface_flag = false;
    }

    // marca o robô mais próximo do clique
    for(int i = 0; i < Robots::SIZE; i++) {
        if(EQ::distance(cv::Point(interface.imageView.robot_pos[0], interface.imageView.robot_pos[1]), virtual_robots_positions[i]) <= 17) {
            // reseta a posição do clique, alvo de posicionamento e orientação alvo
            for (int j = 0; j < 2; j++) {
                interface.imageView.robot_pos[j] =
                interface.imageView.tar_pos[j] =
                interface.imageView.look_pos[j] = -1;
            }

            // se o robo em questão já for o que está selecionado, desseleciona
            if(virtual_robot_selected == i) {
                virtual_robot_selected = -1;
            }
            // senão, seleciona esse mesmo
            else {
                virtual_robot_selected = i;
            }

            // o primeiro robô encontrado no raio é suficiente
            break;
        }
    }

    // atualiza a posição ou orientação do robô selecionado
    if(virtual_robot_selected > -1) {
        // se a orientação foi setada (se pelo menos o x é maior que 0, sabemos que o y também)
        if(interface.imageView.look_pos[0] >= 0) {
            // atualiza os valores no camcap e na estratégia
            virtual_robots_orientations[virtual_robot_selected] =
            strategyGUI.formation_orientations[virtual_robot_selected] =
                    cv::Point(interface.imageView.look_pos[0], interface.imageView.look_pos[1]);

            // desseleciona o robô e reseta o clique
            virtual_robot_selected =
            interface.imageView.look_pos[0] =
            interface.imageView.look_pos[1] = -1;
        }
        // se a posição alvo foi setada
        else if(interface.imageView.tar_pos[0] >= 0) {
            // atualiza a posição do robô no camcap e na estratégia
            virtual_robots_positions[virtual_robot_selected] =
            strategyGUI.formation_positions[virtual_robot_selected] =
                    cv::Point(interface.imageView.tar_pos[0], interface.imageView.tar_pos[1]);

            // desseleciona o robô e reseta o clique
            virtual_robot_selected =
            interface.imageView.tar_pos[0] =
            interface.imageView.tar_pos[1] = -1;
        }
    }
}

void Manager::update_formation_information() { // atualiza as informações dadas pela interface na estratégia
    // reseta robo selecionado
    virtual_robot_selected = -1;

    // copia os dados pra estratégia
    for(int i = 0; i < 3; i++) {
        strategyGUI.formation_positions[i] = virtual_robots_positions[i];
        strategyGUI.formation_orientations[i] = virtual_robots_orientations[i];
    }
}

void Manager::PID_test() {
    // se o botão play estiver pressionado, não faz o pid test
    if (interface.get_start_game_flag() || strategyGUI.formation_flag) return;

    // pra cada robô, olha se o clique na imagem foi nele
    for(int i = 0; i < Robots::SIZE; i++) {
        // se a distancia do robô i com o clique da tela for menor que 1 raio de robô
        if(EQ::distance(Robots::get_position(i), cv::Point(interface.imageView.robot_pos[0], interface.imageView.robot_pos[1])) <= ROBOT_RADIUS) {
            Selec_index = i; // seleciona o robô i entre as chamadas de PID_test()
            interface.imageView.tar_pos[0] = -1; // reseta o alvo x da tela
            interface.imageView.tar_pos[1] = -1; // reseta o alvo y da tela
            interface.imageView.robot_pos[0] = -1; // reseta o clique x da tela
            interface.imageView.robot_pos[1] = -1; // reseta o clique y da tela
            Robots::stop(Selec_index); // para o robô selecionado
            fixed_ball[Selec_index] = false; // desmarca a flag de seguir a bola
        }
    }

    // se tem um robô selecionado na interface
    if(Selec_index > -1) {
        // se a distancia dele é menor que um threshold pra bola, segue a bola
        if(EQ::distance(Ball_kf_est, Robots::get_target(Selec_index)) <= ROBOT_RADIUS*2)
            fixed_ball[Selec_index] = true;
        else { // senão, coloca o alvo pro segundo clique
            fixed_ball[Selec_index] = false;
            Robots::set_target(Selec_index, cv::Point(interface.imageView.tar_pos[0],interface.imageView.tar_pos[1]));

            /* comando por POSITION */
            //Robots::set_command(i, cv::Point(interface.imageView.tar_pos[0],interface.imageView.tar_pos[1]));
        }
    }

    for(int i = 0; i < Robots::SIZE; i++) {
        if(fixed_ball[i]) { // se a bola tá fixa, vai pra bola
            Robots::set_command(i, Ball_kf_est);
        } else {
            // se chegou no alvo, para
            if(EQ::distance(Robots::get_position(i), Robots::get_target(i)) <= ROBOT_RADIUS) {
                // Robots::look_at(i, Ball_kf_est); // faz o robô olhar pra bola ao chegar no alvo (deve ser o único comando nesse if)
                Robots::stop(i); // para o robô
                interface.imageView.tar_pos[0]=-1; // reseta a posição x do alvo da interface
                interface.imageView.tar_pos[1]=-1; // reseta a posição y do alvo da interface
            }
            // senão, segue o passeio (se o alvo for (-1, -1), o flyingMessenger ignora)
            else if(Robots::is_target_set(i)) {
                /* converte o comando pra VECTOR */
                double transAngle = atan2(double(Robots::get_position(i).y - Robots::get_target(i).y), -double(Robots::get_position(i).x - Robots::get_target(i).x));
                Robots::set_command(i, transAngle);
            }
        }
    }
} // PID_test

void Manager::warp_transform(cv::Mat cameraFlow){
    cv::Point2f inputQuad[4];
    cv::Point2f outputQuad[4];
    cv::Mat lambda = cv::Mat::zeros(cameraFlow.rows, cameraFlow.cols, cameraFlow.type());

    // !TODO Otimização do código::Usar laço de repetição
    inputQuad[0] = cv::Point2f( interface.imageView.warp_mat[0][0]-interface.offsetL,interface.imageView.warp_mat[0][1]);
    inputQuad[1] = cv::Point2f( interface.imageView.warp_mat[1][0]+interface.offsetR,interface.imageView.warp_mat[1][1]);
    inputQuad[2] = cv::Point2f( interface.imageView.warp_mat[2][0]+interface.offsetR,interface.imageView.warp_mat[2][1]);
    inputQuad[3] = cv::Point2f( interface.imageView.warp_mat[3][0]-interface.offsetL,interface.imageView.warp_mat[3][1]);

    outputQuad[0] = cv::Point2f( 0,0 );
    outputQuad[1] = cv::Point2f( width-1,0);
    outputQuad[2] = cv::Point2f( width-1,height-1);
    outputQuad[3] = cv::Point2f( 0,height-1  );
    lambda = getPerspectiveTransform( inputQuad, outputQuad );
    warpPerspective(cameraFlow,cameraFlow,lambda,cameraFlow.size());

    if(interface.imageView.adjust_rdy) {
        interface.bt_adjust.set_active(false);
        interface.bt_adjust.set_state(Gtk::STATE_INSENSITIVE);
        interface.adjust_event_flag = false;
        interface.imageView.adjust_event_flag = false;

        for(int i = 0; i < interface.imageView.adjust_mat[0][1]; i++) {
            for(int j = 0; j < 3*interface.imageView.adjust_mat[0][0]; j++) {
                cameraFlow.at<uchar>(i, j) = 0;
            }
        }

        for(int i = height; i > interface.imageView.adjust_mat[1][1]; i--) {
            for(int j = 0; j < 3*interface.imageView.adjust_mat[1][0]; j++) {
                cameraFlow.at<uchar>(i, j) = 0;
            }
        }

        for(int i = 0; i < interface.imageView.adjust_mat[2][1]; i++) {
            for(int j = 3*width; j > 3*interface.imageView.adjust_mat[2][0]; j--) {
                cameraFlow.at<uchar>(i, j) = 0;
            }
        }

        for(int i = height; i>interface.imageView.adjust_mat[3][1]; i--) {
            for(int j = 3*width; j > 3*interface.imageView.adjust_mat[3][0]; j--) {
                cameraFlow.at<uchar>(i, j) = 0;
            }
        }
    }
} // warp_transform

void Manager::filter_positions() {
    cv::Point ballPosition;

    // !TODO trocar estas variáveis para um Point e guardar localmente também
    ballPosition = interface.visionGUI.vision->getBall();
    interface.ballX = ballPosition.x;
    interface.ballY = ballPosition.y;

    interface.updateRobotLabels();
    interface.updateFPS(fps_average);

    // KALMAN FILTER
    if(KF_FIRST) {
        //KALMAN FILTER INIT
        for(unsigned short i = 0; i < Robots::SIZE; i++) {
            //if(Robots::is_posistion_set(i))
            //KF_RobotBall.at(i).KF_init(Robots::get_position(i));
        }

        KF_RobotBall.at(3).KF_init(ballPosition);
        KF_FIRST = false;
    }

    for (unsigned short i = 0; i < Robots::SIZE; i++) {
        //if(Robots::is_posistion_set(i))
        //    Robots::set_position(i, KF_RobotBall[i].KF_Prediction(Robots::get_position(i)));
    }
    Ball_kf_est = KF_RobotBall.at(Robots::SIZE).KF_Prediction(ballPosition);

} // filter_positions
