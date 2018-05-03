/**
*  camcap.hpp
*  @file camcap.hpp
*  @since 2014/02/14
*  @author Gustavo
*/

// Must be included before #ifndef directive
// This makes our compiler include our debug namespace directives into this file
#include "aux/debug.hpp"

#ifndef CAMCAP_HPP_
#define CAMCAP_HPP_
#define PI 3.14159265453

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/video/tracking.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <stdlib.h>
#include <tgmath.h>
#include <gtkmm.h>
#include <math.h>
#include <fstream>

#include "aux/kalmanFilter.hpp"
#include "gui/strategyGUI.hpp"
#include "gui/controlGUI.hpp"
#include "gui/v4linterface.hpp"
#include "vision/vision.hpp"
#include "aux/cpuTimer.hpp"
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

    sigc::connection con;
    
    boost::thread msg_thread;
	boost::condition_variable data_ready_cond;
	boost::mutex data_ready_mutex;
	bool data_ready_flag = false;

    /**
	 * @brief Updates the position of all captured elements
	 * @details Sets ball position, robot's orientation and positions (both actual and prediction)
	 */
    void updateAllPositions();
    
    /**
	 * @brief Interface adjustments after 'start' button is clicked
	 * @param b Variable that indicates whether start/stop button was clicked
	 * @return true 
	 */
    bool start_signal(bool b);
    
    /**
	 * @brief Captures camera frames and shows it in the interface
	 * @details It also draw elements (target circles, arrows, virtual robots) and triggers some events (formation change, warp, hsv and pid buttons)
	 * @return bool true 
	 */
    bool capture_and_show();
    
    /**
	 * @brief Draws an arrow between two given points
	 * @param img Cameraflow
	 * @param pt1 Point to be connected
	 * @param pt2 Point to be connected
	 * @param color Arrow's color
	 * @param thickness Arrow's line thickness
	 * @param line_type The way that pixels are connected. Each pixel has 8 neighbors by default
	 * @param shift Number of fractional bits in the point coordinates
	 * @param tipLength Length of the arrow tip  
	 */
    void arrowedLine(cv::Mat img, cv::Point pt1, cv::Point pt2, const cv::Scalar& color,
        int thickness=1, int line_type=8, int shift=0, double tipLength=0.1);
        
    //void sendCmdToRobots(std::vector<Robot>&robot_list, bool &xbeeIsConnected);

    //void sendCmdToRobots(std::vector<Robot>&robot_list);

    /** 
    * @brief Send robots to its estimated positions
    * @param robots Vector cointaining all robots instances
    */
    void send_cmd_thread(std::vector<Robot> &robots);
	
    /**
    * @brief Notifies all waiting threads through a conditional variable
    */
	void notify_data_ready();
            
    /**
	 * @brief Calculates the euclidean distance between two given points
	 * @param a First point
	 * @param b Second point
	 * @return Linear distance between a and b
	 */
    double distance(cv::Point a, cv::Point b);
    
     /**
	 * @brief Calculates the angular distance between two given points
	 * @param alpha First point
	 * @param beta Second point
	 * @return Angle between alpha and beta
	 */
    double angular_distance(double alpha, double beta);
    
    /**
	 * @brief Sends robots to the target position and orientation
	 */
    void updating_formation();
    
     /**
	 * @brief Creates the formation (creation and load) interface
	 */
    void formation_creation();
   
    /**
	 * @brief Updates information given by interface in strategy
	 */
    void update_formation_information();
    
    //void transformTargets (std::vector<Robot>&robot_list);

	 /**
	 * @brief Executes the 'PID Test on click' at the control panel
	 */	
    void PID_test();
    
    /**
	 * @brief Adjusts the image framing
	 */	
    void warp_transform(cv::Mat cameraFlow);
    
    /**
	 * @brief Constructor of a CamCap object
	 * @details Sets variables and append elements to the interface
	 */	
    CamCap();
    
    /**
	 * @brief Destructor of a CamCap object
	 */	
    ~CamCap();
};


#endif /* CAMCAP_HPP_ */
