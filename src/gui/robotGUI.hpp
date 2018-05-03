/**
 * @file robotGUI.hpp
 *
 * */

#ifndef ROBOTGUI_HPP_
#define ROBOTGUI_HPP_

#define N_ROBOTS 3

#include <gtkmm.h>
#include <iostream>
#include "../robot.hpp"

class RobotGUI: public Gtk::VBox {
private:
	
    // Both global because they keep their values at different calls
    double robots_speed_tmp[N_ROBOTS];
    int robots_function_tmp[N_ROBOTS];

    bool robots_speed_edit_flag = false; /**< Disable the speed edition frame*/
    bool robots_function_edit_flag = false; /**< Disable the robot functions edition frame */

    Gtk::Grid speeds_grid;
    Gtk::Frame robots_function_fm;
    Gtk::Frame robots_speed_fm;

    Gtk::VBox robots_function_vbox; /**< Robot Function frame main vbox */
    Gtk::VBox robots_speed_vbox; /**< Speed frame main vbox*/

    Gtk::VBox robots_speed_slider_vbox[N_ROBOTS]; /**< Robot's vboxes that keep the slider and progress bar*/
    Gtk::HBox robots_function_hbox[N_ROBOTS+1]; /**< Hbox that lists robots in Robo Functions frame */
    Gtk::HBox robots_speed_hbox; /**< Hbox that holds edit/done buttons */

public:

    float default_vel[N_ROBOTS];

    // Edit/Done buttons
    Gtk::Button robots_speed_edit_bt;
    Gtk::Button robots_speed_done_bt;
    Gtk::Button robots_function_edit_bt;
    Gtk::Button robots_function_done_bt;
    Gtk::ComboBoxText cb_robot_function[N_ROBOTS]; /**< Functions dropdown selector*/
    Gtk::HScale robots_speed_hscale[N_ROBOTS]; /**< Speed selection sliders*/
    Gtk::ProgressBar robots_speed_progressBar[N_ROBOTS]; /**< Bars that show robot's actual speed*/
    
    std::vector<Robot> robot_list;
	
	/**
	 * @brief Constructor of a RobotGUI object
	 * @details Calls createFunctionsFrame and createSpeedsFrame
	 */	
    RobotGUI();
    
    /**
	 * @brief Destructor of a RobotGUI object
	 */	
    ~RobotGUI();

	/**
	 * @brief Creator of Robot Functions frame
	 */	
    void createFunctionsFrame();
    
    /**
	 * @brief Creator of Speeds control frame
	 */	
    void createSpeedsFrame();
 
    /**
	 * @brief Updates robot functions at the screen
	 */	
    void update_robot_functions();
    
    /**
	 * @brief Updates the speed progress bar at the screen
	 */	
    void update_speed_progressBars();
  
    /**
	 * @brief Updates robot's role after Done button is clicked
	 */	
    void event_robots_function_done_bt_signal_clicked();
    
	/**
	 * @brief Event triggered when robot function's Edit button is clicked
	 * @details This method may update buttons' label/state and the active element in the combobox
	 */	
    void event_robots_function_edit_bt_signal_clicked();
    
    /**
	 * @brief Event triggered when Speeds' Edit button is clicked
	 * @details This method may update buttons' label/state and the value at the speed slider
	 */	
    void event_robots_speed_edit_bt_signal_pressed();
    
    /**
	 * @brief Updates robot's speed after Done button is clicked
	 */	
    void event_robots_speed_done_bt_signal_clicked();
};

#endif /* ROBOTGUI_HPP_ */
