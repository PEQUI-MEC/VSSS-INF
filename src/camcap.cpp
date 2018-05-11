#include "camcap.hpp"

CamCap::CamCap() : data(0), width(0), height(0), frameCounter(0), 
msg_thread(&CamCap::send_cmd_thread, this, boost::ref(interface.robotGUI.robot_list)) {    
    fm.set_label("ImageView");
    fm.add(interface.imageView);

    notebook.append_page(interface, "Capture");
    notebook.append_page(interface.robotGUI, "Robot");
    notebook.append_page(interface.visionGUI, "Vision");
    notebook.append_page(control, "Control");
    notebook.append_page(strategyGUI, "Strategy");

    KalmanFilter kf;
    for(int i = 0; i < 4; i++) {
        robot_raw_pos.push_back(Ball_Est); // inicializa com um cv point em 0,0
        KF_RobotBall.push_back(kf); 
    }

    for(int i = 0; i < 3; i++) {
        fixed_ball[i]=false;
        virtual_robots_orientations[i] = 0;
        // valores absolutos pois são só pra incializar
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

    interface.signal_start().connect(sigc::mem_fun(*this, &CamCap::start_signal));
}

CamCap::~CamCap(){
    con.disconnect();
    interface.imageView.disable_image_show();
    free(data);

    data = 0;
    
    msg_thread.interrupt();
	if (msg_thread.joinable()) msg_thread.join();
}

bool CamCap::start_signal(bool b) {
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
        interface.robots_id_edit_bt.set_state(Gtk::STATE_NORMAL);
        interface.robotGUI.robots_speed_edit_bt.set_state(Gtk::STATE_NORMAL);
        interface.robotGUI.robots_function_edit_bt.set_state(Gtk::STATE_NORMAL);
        strategyGUI.formation_box.set_sensitive(true);
        strategyGUI.bt_createFormation.set_sensitive(true);

        data = (unsigned char *) calloc(interface.vcap.format_dest.fmt.pix.sizeimage, sizeof(unsigned char));

        interface.imageView.set_size_request(width, height);
        con = Glib::signal_idle().connect(sigc::mem_fun(*this, &CamCap::capture_and_show));

        debug_log("Capture started.");

    } else {
        con.disconnect();

        // Travar os botões de edit
        interface.robots_id_edit_bt.set_state(Gtk::STATE_INSENSITIVE);
        interface.robotGUI.robots_speed_edit_bt.set_state(Gtk::STATE_INSENSITIVE);
        interface.robotGUI.robots_function_edit_bt.set_state(Gtk::STATE_INSENSITIVE);
        strategyGUI.formation_box.set_sensitive(false);
        strategyGUI.bt_createFormation.set_sensitive(false);

        debug_log("Capture Stopped.");
    }

    interface.visionGUI.vision->preloadHSV();

    interface.__event_bt_quick_load_clicked();

    return true;
} // start_signal

bool CamCap::capture_and_show() {

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

    // atualiza a interface de acordo com o tipo de view (simples ou split)
    if (interface.visionGUI.getIsSplitView()) {
        interface.imageView.set_data(interface.visionGUI.vision->getSplitFrame().clone().data, width, height);
        interface.imageView.refresh();
    } else if (interface.visionGUI.HSV_calib_event_flag) {
        interface.imageView.set_data(interface.visionGUI.vision->getThreshold(interface.visionGUI.Img_id).data, width, height);
        interface.imageView.refresh();
    }

    // roda os filtros e atualiza a posição dos robos na interface
    updateAllPositions();

    // -------------- CONTROLE -----------------//

    if(interface.imageView.PID_test_flag && !interface.get_start_game_flag()) {
        control.button_PID_Test.set_active(true);
        PID_test();
    } else {
        for(int i = 0; i < interface.robotGUI.robot_list.size(); i++)
            interface.robotGUI.robot_list[i].target=cv::Point(-1,-1);

        Selec_index=-1;
        control.PID_test_flag = false;
    }

    if(!interface.imageView.PID_test_flag && strategyGUI.formation_flag && !interface.get_start_game_flag()) {
        if(strategyGUI.updating_formation_flag)
            updating_formation();

        formation_creation();
    }
    else if(strategyGUI.updating_formation_flag) {
        strategyGUI.updating_formation_flag = false;

        for(int i = 0; i < interface.robotGUI.robot_list.size(); i++) {
            interface.robotGUI.robot_list.at(i).cmdType = POSITION;
            interface.robotGUI.robot_list.at(i).vmax = interface.robotGUI.default_vel[interface.robotGUI.robot_list.at(i).role];
            interface.robotGUI.robot_list.at(i).target = cv::Point(-1, -1);
        }
    }

    // !TODO não chamar esta função todo frame, desativar o pid_test logicamente no controle
    if (interface.imageView.PID_test_flag && (interface.get_start_game_flag() || strategyGUI.formation_flag))
        control.button_PID_Test.set_active(false);

    // ----------- ESTRATEGIA -----------------//

    // só roda a estratégia se o play tiver sido dado
    if(interface.get_start_game_flag()) {
        strategyGUI.strategy.set_Ball(Ball_kf_est); // interface.visionGUI.vision->getBall()
        strategyGUI.strategy.set_default_vel(interface.robotGUI.default_vel);
        Ball_Est = strategyGUI.strategy.get_Ball_Est();

        // roda a estratégia
        strategyGUI.strategy.get_targets(&(interface.robotGUI.robot_list), (interface.visionGUI.vision->getAllAdvRobots()));

        // atualiza a interface
        interface.robotGUI.update_speed_progressBars();
        interface.robotGUI.update_robot_functions();
    } // if start_game_flag

    // ----------------- DESENHA SOBRE A IMAGEM  -----------------//

    if(!interface.draw_info_flag && !interface.visionGUI.getIsSplitView()) {

        // ----------- DESENHOS DA VISÃO ----------- //

        if (!interface.visionGUI.HSV_calib_event_flag) {
            // bola
            circle(cameraFlow, Ball_kf_est, 7, cv::Scalar(255,255,255), 2);

            for (int i = 0; i < interface.robotGUI.robot_list.size(); i++) {
                // orientação do robô
                line(cameraFlow, interface.robotGUI.robot_list.at(i).position, interface.robotGUI.robot_list.at(i).secundary,cv::Scalar(255,255,0), 2);
                // id do robô
                putText(cameraFlow, std::to_string(i+1),cv::Point(interface.robotGUI.robot_list.at(i).position.x-5,interface.robotGUI.robot_list.at(i).position.y-17),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(255,255,0),2);
                // marcação do robô
                circle(cameraFlow, interface.robotGUI.robot_list.at(i).position, 15, cv::Scalar(255,255,0), 2);

                // vetor que todos os robos estão executando
                cv::Point aux_point;
                aux_point.x = round(100*cos(interface.robotGUI.robot_list[i].transAngle));
                aux_point.y = - round(100*sin(interface.robotGUI.robot_list[i].transAngle));
                aux_point += interface.robotGUI.robot_list[i].position;
                arrowedLine(cameraFlow,interface.robotGUI.robot_list[i].position, aux_point,cv::Scalar(255,0,0),2);
            }

            // adversários
            for(int i = 0; i < interface.visionGUI.vision->getAdvListSize(); i++)
                circle(cameraFlow,interface.visionGUI.vision->getAdvRobot(i), 15, cv::Scalar(0,0,255), 2);
        }

        // ----------- DESENHOS DA ESTRATÉGIA ---------- //

        if(interface.get_start_game_flag()) {
            // desenha a estimativa da bola
            circle(cameraFlow, Ball_Est, 7, cv::Scalar(255,140,0), 2);

            // desenha os alvos dos robôs
            for(int i = 0; i < 3; i++) {
                circle(cameraFlow,interface.robotGUI.robot_list[i].target, 7, cv::Scalar(127,255,127), 2);
                putText(cameraFlow,std::to_string(i+1),cv::Point(interface.robotGUI.robot_list[i].target.x-5,interface.robotGUI.robot_list[i].target.y-17),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(127,255,127),2);
            }
        }

        // ----------- DESENHOS DO CONTROLE ---------- //

        if(strategyGUI.formation_flag) {
            // exibe os robos virtuais
            for(int i = 0; i < 3; i++) {
                if(virtual_robot_selected == i)
                    circle(cameraFlow,virtual_robots_positions[i], 20, cv::Scalar(0,255,100), 3);

                // posição
                circle(cameraFlow,virtual_robots_positions[i], 17, cv::Scalar(0,255,0), 2);
                // orientação
                cv::Point aux_point = cv::Point(virtual_robots_positions[i].x + 30*cos(virtual_robots_orientations[i]), virtual_robots_positions[i].y + 30*sin(virtual_robots_orientations[i]));
                arrowedLine(cameraFlow,virtual_robots_positions[i], aux_point,cv::Scalar(0,255,0),2);
                // identificação
                putText(cameraFlow, std::to_string(i+1),virtual_robots_positions[i] + cv::Point(-14,10),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(0,255,0),2);
            }
        }

        if (interface.imageView.PID_test_flag) {
            for(int i = 0; i < interface.robotGUI.robot_list.size(); i++) {
                if(interface.robotGUI.robot_list[i].target.x!=-1&&interface.robotGUI.robot_list[i].target.y != -1) {
                    // linha branca no alvo sendo executado
                    line(cameraFlow, interface.robotGUI.robot_list[i].position,interface.robotGUI.robot_list[i].target, cv::Scalar(255,255,255),2);
                }
                // círculo branco no alvo sendo executado
                circle(cameraFlow,interface.robotGUI.robot_list[i].target, 9, cv::Scalar(255,255,255), 2);
            }

            if(Selec_index != -1)
                circle(cameraFlow,interface.robotGUI.robot_list[Selec_index].position, 17, cv::Scalar(255,255,255), 2);
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
	
	if (interface.get_start_game_flag() || interface.imageView.PID_test_flag) {
		control.update_msg_time();
		notify_data_ready();
	}

    return true;
} // capture_and_show

void CamCap::send_cmd_thread(vector<Robot> &robots) {
	boost::unique_lock<boost::mutex> lock(data_ready_mutex);
	while (true) {
		try {
			data_ready_cond.wait(lock, [this]() { return data_ready_flag; });
		} catch (...) {
			lock.unlock();
			return;
		}
		data_ready_flag = false;
		control.FM.sendCMDs(robots);
	}
}

void CamCap::notify_data_ready() {
	data_ready_flag = true;
	data_ready_cond.notify_all();
}

void CamCap::arrowedLine(cv::Mat img, cv::Point pt1, cv::Point pt2, const cv::Scalar& color,
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

double CamCap::distance(cv::Point a, cv::Point b) {
    return sqrt(pow(double(b.x - a.x), 2) + pow(double(b.y - a.y), 2));
}

double CamCap::angular_distance(double alpha, double beta) {
    double phi = fmod(abs(beta - alpha), (PI));
    double distance = phi > PI/2 ? PI - phi : phi;
    return distance;
}

void CamCap::updating_formation() {
    // se os três robôs estiverem posicionados, desmarca a flag
    int robots_positioned = 0;

    for(int i = 0; i < interface.robotGUI.robot_list.size(); i++) {
        if(distance(interface.robotGUI.robot_list.at(i).position, virtual_robots_positions[i]) > CONST::fixed_pos_distance / 4) {
            //interface.robotGUI.robot_list.at(i).cmdType = VECTOR;
            //interface.robotGUI.robot_list.at(i).transAngle = atan2(double(interface.robotGUI.robot_list.at(i).position.y - virtual_robots_positions[i].y), - double(interface.robotGUI.robot_list.at(i).position.x - virtual_robots_positions[i].x));
            interface.robotGUI.robot_list.at(i).vmax = MAX_POSITIONING_VEL;

            interface.robotGUI.robot_list.at(i).cmdType = POSITION;
            interface.robotGUI.robot_list.at(i).target = virtual_robots_positions[i];

            //std::cout << "robot " << i+1 << " updating position.\n";
        } else if(angular_distance(interface.robotGUI.robot_list.at(i).orientation, virtual_robots_orientations[i]) > MAX_THETA_TOLERATION * (PI/180)) {
            interface.robotGUI.robot_list.at(i).fixedPos = true;
            interface.robotGUI.robot_list.at(i).cmdType = ORIENTATION;
            interface.robotGUI.robot_list.at(i).targetOrientation = virtual_robots_positions[i].x > width/2 ? -virtual_robots_orientations[i] : virtual_robots_orientations[i];
            interface.robotGUI.robot_list.at(i).vmax = MAX_POSITIONING_VEL;
            //std::cout << "robot " << i+1 << " updating rotation. Now: " << interface.robotGUI.robot_list.at(i).orientation << " Desired:" << virtual_robots_orientations[i] << ".\n";
        } else {
            //std::cout << "robot " << i+1 << " done.\n";
            interface.robotGUI.robot_list.at(i).cmdType = ORIENTATION;
            interface.robotGUI.robot_list.at(i).targetOrientation = virtual_robots_orientations[i];
            interface.robotGUI.robot_list.at(i).vmax = 0;
            robots_positioned++;
        }
    }

    if(robots_positioned > 2) {
        strategyGUI.updating_formation_flag = false;
        std::cout << "Done positioning.\n";
    }
}

void CamCap::formation_creation() {
    if (interface.get_start_game_flag()) return;

    if(strategyGUI.update_interface_flag) {
        for(int i = 0; i < 3; i++) {
                virtual_robots_positions[i] = strategyGUI.formation_positions[i];
                virtual_robots_orientations[i] = strategyGUI.formation_orientations[i];
        }

        strategyGUI.update_interface_flag = false;
    }

    // marca o robô mais próximo
    for(int i = 0; i < 3; i++) {
        float dist = sqrt(pow((interface.imageView.robot_pos[0]-virtual_robots_positions[i].x),2)+pow((interface.imageView.robot_pos[1]-virtual_robots_positions[i].y),2));
        if(dist <= 17) {
            virtual_robot_selected = i;
            for(int j = 0; j < 2; j++) {
                interface.imageView.tar_pos[i] = -1;
                interface.imageView.look_pos[i] = -1;
            }
        }
    }

    // segundo clique
    if(virtual_robot_selected > -1) {
        if(interface.imageView.look_pos[0] >= 0) {
            float x1, x2, y1, y2;
            x1 = virtual_robots_positions[virtual_robot_selected].x;
            y1 = virtual_robots_positions[virtual_robot_selected].y;
            x2 = interface.imageView.look_pos[0];
            y2 = interface.imageView.look_pos[1];
            virtual_robots_orientations[virtual_robot_selected] = atan2((y2-y1)*1.3/height,(x2-x1)*1.5/width);
            update_formation_information();
        } else if(interface.imageView.tar_pos[0] >= 0) {
            virtual_robots_positions[virtual_robot_selected] = cv::Point(interface.imageView.tar_pos[0], interface.imageView.tar_pos[1]);
            update_formation_information();
        }
    }
}

void CamCap::update_formation_information() { // atualiza as informações dadas pela interface na estratégia
    // reseta robo selecionado
    virtual_robot_selected = -1;

    // copia os dados pra estratégia
    for(int i = 0; i < 3; i++) {
        strategyGUI.formation_positions[i] = virtual_robots_positions[i];
        strategyGUI.formation_orientations[i] = virtual_robots_orientations[i];
    }
}

void CamCap::PID_test() {
    if (interface.get_start_game_flag()) return;
    
    for(int i = 0; i < interface.robotGUI.robot_list.size() && i < 3; i++) {
        double dist;

        dist = sqrt(pow((interface.imageView.robot_pos[0]-interface.robotGUI.robot_list[i].position.x),2)+pow((interface.imageView.robot_pos[1]-interface.robotGUI.robot_list[i].position.y),2));

        if(dist <= 17) {
            Selec_index = i;
            interface.imageView.tar_pos[0] = -1;
            interface.imageView.tar_pos[1] = -1;
            interface.robotGUI.robot_list[Selec_index].target=cv::Point(-1,-1);
            fixed_ball[Selec_index] = false;
        }
    }

    if(Selec_index > -1) {
        if(sqrt(pow((Ball_kf_est.x-interface.robotGUI.robot_list[Selec_index].target.x),2)+pow((Ball_kf_est.y-interface.robotGUI.robot_list[Selec_index].target.y),2))<=7)
            fixed_ball[Selec_index] = true;

        if(fixed_ball[Selec_index]) {
            interface.robotGUI.robot_list[Selec_index].target = Ball_kf_est;
        } else {
            interface.robotGUI.robot_list[Selec_index].target = cv::Point(interface.imageView.tar_pos[0],interface.imageView.tar_pos[1]);
        }
    }

    for(int i = 0; i < interface.robotGUI.robot_list.size() && i < 3; i++) {
        if(fixed_ball[i]) {
            interface.robotGUI.robot_list[i].target = Ball_kf_est;
        } else {
            if(sqrt(pow((interface.robotGUI.robot_list[i].position.x-interface.robotGUI.robot_list[i].target.x), 2) +
                pow((interface.robotGUI.robot_list[i].position.y-interface.robotGUI.robot_list[i].target.y),2)) < 15) {

                interface.robotGUI.robot_list[i].target = cv::Point(-1,-1);
                interface.imageView.tar_pos[0]=-1;
                interface.imageView.tar_pos[1]=-1;
                interface.robotGUI.robot_list[i].Vr = 0;
                interface.robotGUI.robot_list[i].Vl = 0;
                interface.robotGUI.robot_list[i].vmax = 0;

            }

            if(interface.robotGUI.robot_list[i].target.x != -1 && interface.robotGUI.robot_list[i].target.y != -1) {
                interface.robotGUI.robot_list[Selec_index].target = cv::Point(interface.imageView.tar_pos[0],interface.imageView.tar_pos[1]);
                interface.robotGUI.robot_list[Selec_index].vmax = interface.robotGUI.default_vel[interface.robotGUI.robot_list.at(Selec_index).role];
                interface.robotGUI.robot_list[i].cmdType = VECTOR;
                interface.robotGUI.robot_list[i].transAngle = atan2(double(interface.robotGUI.robot_list[i].position.y - interface.robotGUI.robot_list[i].target.y), - double(interface.robotGUI.robot_list[i].position.x - interface.robotGUI.robot_list[i].target.x));
                //interface.robotGUI.robot_list[i].goTo(interface.robotGUI.robot_list[i].target,interface.visionGUI.vision->getBall());
            } else {
                interface.robotGUI.robot_list[i].Vr = 0;
                interface.robotGUI.robot_list[i].Vl = 0;
            }
        }
    }
} // PID_test

void CamCap::warp_transform(cv::Mat cameraFlow){
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

void CamCap::updateAllPositions() {
    Robot robot;
    cv::Point ballPosition;

    for (int i = 0; i < interface.visionGUI.vision->getRobotListSize(); i++) {
        robot = interface.visionGUI.vision->getRobot(i);
        robot_raw_pos.at(i) = robot.position;
        interface.robotGUI.robot_list[i].orientation = robot.orientation;
        interface.robotGUI.robot_list[i].secundary = robot.secundary;
    }

    // !TODO trocar estas variáveis para um Point e guardar localmente também
    ballPosition = interface.visionGUI.vision->getBall();
    interface.ballX = ballPosition.x;
    interface.ballY = ballPosition.y;

    interface.updateRobotLabels();
    interface.updateFPS(fps_average);

    // KALMAN FILTER
    if(KF_FIRST) {
        //KALMAN FILTER INIT
        for(int i=0; i<3; i++)
            KF_RobotBall[i].KF_init(robot_raw_pos.at(i));

        KF_RobotBall[3].KF_init(ballPosition);
        KF_FIRST = false;
    }

    for (int i = 0; i < interface.visionGUI.vision->getRobotListSize(); i++) {
        interface.robotGUI.robot_list[i].position = KF_RobotBall[i].KF_Prediction(robot_raw_pos.at(i));
    }
    Ball_kf_est = KF_RobotBall[3].KF_Prediction(ballPosition);

} // updateAllPositions