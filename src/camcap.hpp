/*
*  camcap.hpp
*
*  Created on: Feb 18, 2014
*      Author: gustavo
*/

#ifndef CAMCAP_HPP_
#define CAMCAP_HPP_
#define PI 3.14159265453

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/video/tracking.hpp>

#include "gui/strategyGUI.hpp"
#include "gui/controlGUI.hpp"
#include "vision/vision.hpp"
#include "kalmanFilter.hpp"
#include "gui/v4linterface.hpp"
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <stdlib.h>
#include <tgmath.h>
#include <gtkmm.h>
#include <math.h>
#include <fstream>
#include "cpuTimer.cpp"
#include "constants.hpp"

#define MAX_THETA_TOLERATION 3
#define MAX_POSITIONING_VEL 0.1


class CamCap: public Gtk::HBox {
public:
    int width, height;
    int Selec_index=-1;
    int fps_average = 0;
    int timerCounter = 0;
    CPUTimer timer;

    bool fixed_ball[3];
    bool KF_FIRST = true;

    unsigned char * data;

    int frameCounter;
    double ticks = 0;
    vector<cv::Point> robot_kf_est;
    vector< KalmanFilter > KF_RobotBall;

    cv::Point Ball_Est;
    cv::Point Ball_kf_est;

    cv::Point virtual_robots_positions[3];
    float virtual_robots_orientations[3];
    int virtual_robot_selected = -1;

    StrategyGUI strategyGUI;
    ControlGUI control;
    vsss_gui::V4LInterface interface;

    Gtk::Frame fm;
    Gtk::Frame info_fm;
    Gtk::VBox camera_vbox;
    Gtk::Notebook notebook;

    boost::thread_group threshold_threads;
    sigc::connection con;

    /* PARA TESTE */
    cv::Point obstacle;
    cv::Point deviation1;
    cv::Point deviation2;


    void updateAllPositions() {

        Robot robot;
        cv::Point ballPosition;

        for (int i = 0; i < interface.visionGUI.vision->getRobotListSize(); i++) {
            robot = interface.visionGUI.vision->getRobot(i);
            interface.robotGUI.robot_list[i].position = robot.position;
            interface.robotGUI.robot_list[i].orientation = robot.orientation;
            interface.robotGUI.robot_list[i].secundary = robot.secundary;
        }

        ballPosition = interface.visionGUI.vision->getBall();
        interface.ballX = ballPosition.x;
        interface.ballY = ballPosition.y;

        interface.updateRobotLabels();
        interface.updateFPS(fps_average);


        // KALMAN FILTER
        if(KF_FIRST) {
            //KALMAN FILTER INIT
            for(int i=0; i<3; i++)
                KF_RobotBall[i].KF_init(interface.visionGUI.vision->getRobotPos(i));

            KF_RobotBall[3].KF_init(interface.visionGUI.vision->getBall());
            KF_FIRST = false;
        }

        // !TODO Otimização do código::Usar laço de repetição
        robot_kf_est[0] = KF_RobotBall[0].KF_Prediction(interface.visionGUI.vision->getRobotPos(0));
        robot_kf_est[1] = KF_RobotBall[1].KF_Prediction(interface.visionGUI.vision->getRobotPos(1));
        robot_kf_est[2] = KF_RobotBall[2].KF_Prediction(interface.visionGUI.vision->getRobotPos(2));
        Ball_kf_est = KF_RobotBall[3].KF_Prediction(ballPosition);

    } // updateAllPositions


    bool start_signal(bool b) {

        if (b) {
            cout << "Start Clicked!" << endl;

            if (data) {
                interface.imageView.disable_image_show();
                free(data);
                data = 0;
            }
            /*GdkScreen* screen = gdk_screen_get_default();
            if (interface.vcap.format_dest.fmt.pix.width > gdk_screen_get_width(screen)/2 || interface.vcap.format_dest.fmt.pix.height > gdk_screen_get_height(screen)/2)
            {
            width = gdk_screen_get_width(screen)/2;
            height = gdk_screen_get_height(screen)/2;
            strategyGUI.strategy.set_constants(width,height);
            }
            else
            {*/
            width = interface.vcap.format_dest.fmt.pix.width;
            height = interface.vcap.format_dest.fmt.pix.height;
            strategyGUI.strategy.set_constants(width,height);
            //}

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

            cout << "Start Clicked! 1" << endl;

        } else {
            cout << "Stop Button Clicked!" << endl;
            con.disconnect();

            // Travar os botões de edit
            interface.robots_id_edit_bt.set_state(Gtk::STATE_INSENSITIVE);
            interface.robotGUI.robots_speed_edit_bt.set_state(Gtk::STATE_INSENSITIVE);
            interface.robotGUI.robots_function_edit_bt.set_state(Gtk::STATE_INSENSITIVE);
            strategyGUI.formation_box.set_sensitive(false);
            strategyGUI.bt_createFormation.set_sensitive(false);
        }

        interface.__event_bt_quick_load_clicked();
        interface.visionGUI.quickLoadGMM();

        return true;

    } // start_signal


    bool capture_and_show() {

        if (!data) return false;

        if (frameCounter == 0) timer.start();

        frameCounter++;

        interface.vcap.grab_rgb(data);
        interface.imageView.set_data(data, width, height);
        interface.imageView.refresh();

        cv::Mat cameraFlow(height, width, CV_8UC3, data);

        if(interface.imageView.hold_warp) {
            interface.warped = true;
            interface.bt_adjust.set_state(Gtk::STATE_NORMAL);
            // !TODO Otimização do codigo:: repetição desnecessária de variável
            interface.imageView.warp_event_flag = false;
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
        interface.imageView.gmm_sample_flag = interface.visionGUI.getSamplesEventFlag();

        if (interface.imageView.sector != -1) {
            interface.visionGUI.selectFrame(interface.imageView.sector);
            interface.imageView.sector = -1;
        }

        if(interface.warped) {
            interface.bt_warp.set_active(false);
            interface.bt_warp.set_state(Gtk::STATE_INSENSITIVE);
            warp_transform(cameraFlow);
            interface.imageView.warp_event_flag=false;

            if(interface.invert_image_flag)
                cv::flip(cameraFlow,cameraFlow, -1);
        }

        if (interface.imageView.gmm_ready_flag) {
            interface.visionGUI.gmm.setFrame(cameraFlow);
            interface.visionGUI.gmm.pushSample(interface.imageView.gmm_clicks);
            interface.visionGUI.incrementSamples();
            interface.imageView.gmm_ready_flag = false;
        }

        if (interface.visionGUI.gmm.getIsTrained() && !interface.visionGUI.getIsHSV()) {
            interface.visionGUI.gmm.run(cameraFlow);

            if (interface.visionGUI.gmm.getDoneFlag())
                interface.visionGUI.vision->runGMM(interface.visionGUI.gmm.getAllThresholds());


            if (interface.visionGUI.getGaussiansFrameFlag()) {
                interface.imageView.set_data(interface.visionGUI.gmm.getGaussiansFrame().data, width, height);
                interface.imageView.refresh();
            } else if (interface.visionGUI.getFinalFrameFlag()) {
                interface.imageView.set_data(interface.visionGUI.gmm.getFinalFrame().data, width, height);
                interface.imageView.refresh();
            } else if (interface.visionGUI.getThresholdFrameFlag()) {
                interface.imageView.set_data(interface.visionGUI.gmm.getThresholdFrame(interface.visionGUI.getGMMColorIndex()).data, width, height);
                interface.imageView.refresh();
            }
        } else {
            interface.visionGUI.vision->run(cameraFlow);
            if (interface.visionGUI.getIsSplitView()) {
                interface.imageView.set_data(interface.visionGUI.vision->getSplitFrame().clone().data, width, height);
                interface.imageView.refresh();
            } else if (interface.visionGUI.HSV_calib_event_flag) {
                interface.imageView.set_data(interface.visionGUI.vision->getThreshold(interface.visionGUI.Img_id).data, width, height);
                interface.imageView.refresh();
            }
        }

        if(!interface.visionGUI.HSV_calib_event_flag) {
            if (!interface.draw_info_flag && !interface.visionGUI.getIsSplitView()) {
                cv::Point aux_point;

                if (interface.imageView.PID_test_flag) {
                    for(int i = 0; i < interface.robotGUI.robot_list.size(); i++) {
                        if(interface.robotGUI.robot_list[i].target.x!=-1&&interface.robotGUI.robot_list[i].target.y != -1) {
                            // linha branca no alvo sendo executado
                            line(cameraFlow, interface.robotGUI.robot_list[i].position,interface.robotGUI.robot_list[i].target, cv::Scalar(255,255,255),2);
                            // linha roxa no alvo final
                            line(cameraFlow, interface.robotGUI.robot_list[i].position, cv::Point(interface.imageView.tar_pos[0], interface.imageView.tar_pos[1]), cv::Scalar(255,0,255),2);
                        }
                        // círculo branco no alvo sendo executado
                        circle(cameraFlow,interface.robotGUI.robot_list[i].target, 9, cv::Scalar(255,255,255), 2);
                        // círculo roxo no alvo final
                        circle(cameraFlow,cv::Point(interface.imageView.tar_pos[0], interface.imageView.tar_pos[1]), 7, cv::Scalar(255,0,255), 2);
                        // círculo vermelho no obstáculo
                        circle(cameraFlow,obstacle, 17, cv::Scalar(255,0,0), 2);
                        // círculo verde nos desvios
                        circle(cameraFlow,deviation1, 7, cv::Scalar(0,255,0), 2);
                        circle(cameraFlow,deviation2, 7, cv::Scalar(0,255,0), 2);
                    }

                    if(Selec_index != -1)
                      circle(cameraFlow,interface.robotGUI.robot_list[Selec_index].position, 17, cv::Scalar(255,255,255), 2);
                }

                if (interface.visionGUI.getDrawSamples()) {
                    std::vector<cv::Point> points = interface.visionGUI.gmm.getSamplePoints();

                    for (int i = 0; i < points.size(); i = i+2)
                        rectangle(cameraFlow, points.at(i), points.at(i+1), cv::Scalar(0,255,255));
                }

                circle(cameraFlow,interface.visionGUI.vision->getBall(), 7, cv::Scalar(255,255,255), 2);

                for (int i = 0; i < interface.visionGUI.vision->getRobotListSize(); i++) {
                    // robo 1
                    line(cameraFlow, interface.visionGUI.vision->getRobot(i).position, interface.visionGUI.vision->getRobot(i).secundary,cv::Scalar(255,255,0), 2);
                    //line(cameraFlow,interface.robotGUI.robot_list[0].position,interface.robotGUI.robot_list[0].ternary,cv::Scalar(100,255,0), 2);
                    putText(cameraFlow, std::to_string(i+1),cv::Point(interface.visionGUI.vision->getRobot(i).position.x-5,interface.visionGUI.vision->getRobot(i).position.y-17),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(255,255,0),2);
                    circle(cameraFlow, interface.visionGUI.vision->getRobot(i).position, 15, cv::Scalar(255,255,0), 2);
                    // linha da pick-a
                    if(interface.visionGUI.vision->getRobot(i).rearPoint != cv::Point(-1,-1))
                        line(cameraFlow, interface.visionGUI.vision->getRobot(i).secundary,interface.visionGUI.vision->getRobot(i).rearPoint,cv::Scalar(255,0,0), 2);

                    // vetor que todos os robos estão executando
                    aux_point.x = round(100*cos(interface.robotGUI.robot_list[i].transAngle));
                    aux_point.y = - round(100*sin(interface.robotGUI.robot_list[i].transAngle));
                    aux_point += interface.robotGUI.robot_list[i].position;
                    arrowedLine(cameraFlow,interface.robotGUI.robot_list[i].position, aux_point,cv::Scalar(255,0,0),2);
                }

                for(int i = 0; i < 5; i++) {
                    aux_point.x = round(100*cos(strategyGUI.strategy.pot_angle[i]));
                    aux_point.y = - round(100*sin(strategyGUI.strategy.pot_angle[i]));
                    aux_point += interface.robotGUI.robot_list[2].position;

                    if(strategyGUI.strategy.pot_magnitude[i]!=0)
                        arrowedLine(cameraFlow,interface.robotGUI.robot_list[2].position, aux_point, cv::Scalar(0,255,0));
                }

                aux_point.x = round(100*cos(strategyGUI.strategy.pot_goalTheta));
                aux_point.y = - round(100*sin(strategyGUI.strategy.pot_goalTheta));
                aux_point += interface.robotGUI.robot_list[2].position;
                arrowedLine(cameraFlow,interface.robotGUI.robot_list[2].position, aux_point, cv::Scalar(255,255,0));

                for(int i = 0; i < interface.visionGUI.vision->getAdvListSize(); i++)
                    circle(cameraFlow,interface.visionGUI.vision->getAdvRobot(i), 15, cv::Scalar(0,0,255), 2);
            } // if !interface.draw_info_flag
        } // if !draw_info_flag

        updateAllPositions();

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
        } else if(strategyGUI.updating_formation_flag) {
            strategyGUI.updating_formation_flag = false;

            for(int i = 0; i < interface.robotGUI.robot_list.size(); i++) {
                interface.robotGUI.robot_list.at(i).cmdType = POSITION;
                interface.robotGUI.robot_list.at(i).vmax = interface.robotGUI.robot_list.at(i).vdefault;
                interface.robotGUI.robot_list.at(i).target = cv::Point(-1, -1);
            }
        }

        if (interface.imageView.PID_test_flag && (interface.get_start_game_flag() || strategyGUI.formation_flag))
            control.button_PID_Test.set_active(false);

        // ----------- ESTRATEGIA -----------------//
        if(interface.get_start_game_flag()) {
            strategyGUI.strategy.set_Ball(interface.visionGUI.vision->getBall());
            Ball_Est = strategyGUI.strategy.get_Ball_Est();
            // line(cameraFlow,interface.visionGUI.vision->getBall(),Ball_Est,cv::Scalar(255,140,0), 2);
            circle(cameraFlow,Ball_Est, 7, cv::Scalar(255,140,0), 2);
            //char buffer[3]; -> não é utilizado
            // line(cameraFlow,cv::Point(strategyGUI.strategy.COORD_BOX_DEF_X,strategyGUI.strategy.COORD_BOX_UP_Y - strategyGUI.strategy.ABS_ROBOT_SIZE/2),cv::Point(strategyGUI.strategy.COORD_GOAL_DEF_FRONT_X,strategyGUI.strategy.COORD_BOX_UP_Y- strategyGUI.strategy.ABS_ROBOT_SIZE/2),cv::Scalar(255,140,0), 2);
            strategyGUI.strategy.get_targets(&(interface.robotGUI.robot_list), (interface.visionGUI.vision->getAllAdvRobots()));

            for(int i = 0; i < 3; i++) {
                circle(cameraFlow,interface.robotGUI.robot_list[i].target, 7, cv::Scalar(127,255,127), 2);
                putText(cameraFlow,std::to_string(i+1),cv::Point(interface.robotGUI.robot_list[i].target.x-5,interface.robotGUI.robot_list[i].target.y-17),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(127,255,127),2);
            }

            interface.robotGUI.update_speed_progressBars();
            interface.robotGUI.update_robot_functions();

        } // if start_game_flag
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

    } // capture_and_show


    void arrowedLine(cv::Mat img, cv::Point pt1, cv::Point pt2, const cv::Scalar& color,
        int thickness=1, int line_type=8, int shift=0, double tipLength=0.1) {

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


    //void sendCmdToRobots(std::vector<Robot>&robot_list, bool &xbeeIsConnected){
    void sendCmdToRobots(std::vector<Robot>&robot_list){

        while (1) {
            if (interface.get_start_game_flag() || interface.imageView.PID_test_flag || strategyGUI.updating_formation_flag) {
                // !TODO Otimização do código::Usar laço de repetição
                //transformTargets(robotGUI.robot_list);
                robot_list[0].position = robot_kf_est[0];
                robot_list[1].position = robot_kf_est[1];
                robot_list[2].position = robot_kf_est[2];
                //control.s.sendCmdToRobots(robot_list);
                control.FM.sendCMDs(robot_list);
            }

            boost::this_thread::sleep(boost::posix_time::milliseconds(200));
        }
    }


    double distance(cv::Point a, cv::Point b) {

        return sqrt(pow(double(b.x - a.x), 2) + pow(double(b.y - a.y), 2));

    }


    double angular_distance(double alpha, double beta) {

        double phi = fmod(abs(beta - alpha), (PI));
        double distance = phi > PI/2 ? PI - phi : phi;
        return distance;

    }


    // manda os robôs para a posição e orientação alvo
    void updating_formation() {

        // se os três robôs estiverem posicionados, desmarca a flag
        int robots_positioned = 0;
        //std::cout << "\niteration id#" << rand() << "\n";
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

    // cria a interface de criação e carregamento de formação
    void formation_creation() {

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
                // !TODO Otimização do código::Usar laço de repetição
                interface.imageView.tar_pos[0] = -1;
                interface.imageView.tar_pos[1] = -1;
                interface.imageView.look_pos[0] = -1;
                interface.imageView.look_pos[1] = -1;
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


    // atualiza as informações dadas pela interface na estratégia
    void update_formation_information() {

        // reseta robo selecionado
        virtual_robot_selected = -1;

        // copia os dados pra estratégia
        for(int i = 0; i < 3; i++) {
            strategyGUI.formation_positions[i] = virtual_robots_positions[i];
            strategyGUI.formation_orientations[i] = virtual_robots_orientations[i];
        }

    }

    /*void transformTargets (std::vector<Robot>&robot_list){
        double tmp[2];
        for (int i = 0; i < 3; i++) {
            if(robot_list[i].target.x!=-1&&robot_list[i].target.y!=-1) {
                // tmp[0] = double(robot_list[i].target.x - robot_kf_est[i].x);
                // tmp[1] = double(robot_list[i].target.y - robot_kf_est[i].y);
                tmp[0] = double(robot_list[i].target.x - robot_list[i].position.x);
                tmp[1] = double(robot_list[i].target.y - robot_list[i].position.y);
                // cout << "tmp[0] " << tmp[0] << " tmp[1] " << tmp[1] << endl;
                robot_list[i].transTarget.x = round(cos(robot_list[i].orientation)*tmp[0] + sin(robot_list[i].orientation)*tmp[1]);
                robot_list[i].transTarget.y = round(-(-sin(robot_list[i].orientation)*tmp[0] + cos(robot_list[i].orientation)*tmp[1]));
                // cout << "robot_list[i].transTarget.x " << robot_list[i].transTarget.x << " robot_list[i].transTarget.y " << robot_list[i].transTarget.y << endl;
                // robot_list[i].transAngle = atan2(robot_list[i].transTarget.y, robot_list[i].transTarget.x);
                // cout << "transAngle " << robot_list[i].transAngle << endl;

            } else {
                robot_list[i].transTarget.x = NULL;
                robot_list[i].transTarget.y = NULL;
            }
        }
    } */


    void PID_test() {

        if (interface.get_start_game_flag()) return;

        double dist;
        int old_Selec_index;

        old_Selec_index = Selec_index;

        for(int i = 0; i < interface.robotGUI.robot_list.size() && i < 3; i++) {
            // interface.robotGUI.robot_list[i].cmdType = 0; // position cmd
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
            if(sqrt(pow((interface.visionGUI.vision->getBall().x-interface.robotGUI.robot_list[Selec_index].target.x),2)+pow((interface.visionGUI.vision->getBall().y-interface.robotGUI.robot_list[Selec_index].target.y),2))<=7)
                fixed_ball[Selec_index] = true;

            if(fixed_ball[Selec_index]) {
                interface.robotGUI.robot_list[Selec_index].target=interface.visionGUI.vision->getBall();
            } else {
                interface.robotGUI.robot_list[Selec_index].target = cv::Point(interface.imageView.tar_pos[0],interface.imageView.tar_pos[1]);
            }
        }


        for(int i = 0; i < interface.robotGUI.robot_list.size() && i < 3; i++) {
            if(fixed_ball[i]) {
                interface.robotGUI.robot_list[i].target=interface.visionGUI.vision->getBall();
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
                    interface.robotGUI.robot_list[Selec_index].vmax = interface.robotGUI.robot_list[Selec_index].vdefault;
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


    void warp_transform(cv::Mat cameraFlow){

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


    CamCap() : data(0), width(0), height(0), frameCounter(0) {

        // !TODO Otimização do código::Usar laço de repetição
        fixed_ball[0]=false;
        fixed_ball[1]=false;
        fixed_ball[2]=false;
        fm.set_label("ImageView");
        fm.add(interface.imageView);

        notebook.append_page(interface, "Capture");
        notebook.append_page(interface.robotGUI, "Robot");
        notebook.append_page(interface.visionGUI, "Vision");
        notebook.append_page(control, "Control");
        notebook.append_page(strategyGUI, "Strategy");

        // !TODO Otimização do código::Usar laço de repetição
        robot_kf_est.push_back(Ball_Est);
        robot_kf_est.push_back(Ball_Est);
        robot_kf_est.push_back(Ball_Est);
        robot_kf_est.push_back(Ball_Est);

        KalmanFilter kf;
        // !TODO Otimização do código::Usar laço de repetição
        KF_RobotBall.push_back(kf);
        KF_RobotBall.push_back(kf);
        KF_RobotBall.push_back(kf);
        KF_RobotBall.push_back(kf);

        for(int i = 0; i < 3; i++) {
            virtual_robots_orientations[i] = 0;

            // !TODO hardcoded, usar variáveis quando possível
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

        // Thread que envia comandos para o robo
        // threshold_threads.add_thread(new boost::thread(&CamCap::sendCmdToRobots,this,
        // boost::ref(interface.robotGUI.robot_list), boost::ref(control.s.Serial_Enabled)));
        threshold_threads.add_thread(new boost::thread(&CamCap::sendCmdToRobots,this, boost::ref(interface.robotGUI.robot_list)));

        interface.signal_start().connect(sigc::mem_fun(*this, &CamCap::start_signal));

    }


    ~CamCap(){
        con.disconnect();
        interface.imageView.disable_image_show();
        free(data);

        data = 0;
    }

};


#endif /* CAMCAP_HPP_ */
