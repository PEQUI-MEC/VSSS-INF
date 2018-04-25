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
#include "cpuTimer.hpp"
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

    vector<cv::Point> robot_raw_pos;
    vector< KalmanFilter > KF_RobotBall;

    cv::Point Ball_Est;
    cv::Point Ball_kf_est;

    cv::Point virtual_robots_positions[3];
    float virtual_robots_orientations[3];
    int virtual_robot_selected = -1;

    StrategyGUI strategyGUI;
    ControlGUI control;
    VSSS_GUI::V4LInterface interface;


    Gtk::Frame fm;
    Gtk::Frame info_fm;
    Gtk::VBox camera_vbox;
    Gtk::Notebook notebook;

    //boost::thread_group threshold_threads;
    sigc::connection con;

    /* PARA TESTE */
    //cv::Point obstacle;
    //cv::Point deviation1;
    //cv::Point deviation2;
    
    boost::thread msg_thread;
	boost::condition_variable data_ready_cond;
	boost::mutex data_ready_mutex;
	bool data_ready_flag = false;

    void updateAllPositions();
    
    bool start_signal(bool b);
    
    bool capture_and_show();
    
    void send_cmd_thread(std::vector<Robot> &robots);
	
	void notify_data_ready();
    
    void arrowedLine(cv::Mat img, cv::Point pt1, cv::Point pt2, const cv::Scalar& color,
        int thickness=1, int line_type=8, int shift=0, double tipLength=0.1);
    
    //void sendCmdToRobots(std::vector<Robot>&robot_list);
    
    double distance(cv::Point a, cv::Point b);
    
    double angular_distance(double alpha, double beta);
    
    // manda os robôs para a posição e orientação alvo
    void updating_formation();
    
    // cria a interface de criação e carregamento de formação
    void formation_creation();

    // atualiza as informações dadas pela interface na estratégia
    void update_formation_information();
    
    /** void transformTargets (std::vector<Robot>&robot_list); */

    void PID_test();
    
    void warp_transform(cv::Mat cameraFlow);
    
    CamCap();
    
    ~CamCap();
};


#endif /* CAMCAP_HPP_ */
