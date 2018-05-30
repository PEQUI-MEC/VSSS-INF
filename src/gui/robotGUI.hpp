/**
 * @file robotGUI.hpp
 * @author Bryan Lincoln @ Pequi Mecânico
 * @date 18/05/2018
 * @brief Robot Configuration GUI
 * @see https://www.facebook.com/NucleoPMec/
 * @sa https://www.instagram.com/pequimecanico
 */

#ifndef ROBOTGUI_HPP_
#define ROBOTGUI_HPP_

#include <gtkmm.h>
#include <iostream>
#include "robots.hpp"

class RobotGUI: public Gtk::VBox {
private:
    // Both global because they keep their values at different calls
    double robots_speed_tmp[Robots::SIZE];
    int robots_function_tmp[Robots::SIZE];

    bool robots_speed_edit_flag = false; /**< Disables the speed edition frame*/
    bool robots_function_edit_flag = false; /**< Disables the robot functions edition frame */
    bool robots_id_edit_flag = false; /**< Disables the robot id edition frame */

    Gtk::Grid speeds_grid;
    Gtk::Frame robots_function_fm;
    Gtk::Frame robots_speed_fm;

    Gtk::VBox robots_function_vbox; /**< Robot Function frame main vbox */
    Gtk::VBox robots_speed_vbox; /**< Speed frame main vbox*/

    Gtk::VBox robots_speed_slider_vbox[Robots::SIZE]; /**< Robot's vboxes that keep the slider and progress bar */
    Gtk::HBox robots_function_hbox[Robots::SIZE+1]; /**< Hbox that lists robots in Robo Functions frame */
    Gtk::HBox robots_speed_hbox; /**< Hbox that holds edit/done buttons */

    Gtk::Frame robots_id_fm;
    Gtk::VBox robots_id_vbox;
    Gtk::HBox robots_id_hbox[Robots::SIZE+1]; /**< + the hbox that holds Edit and Done buttons */
    Gtk::Button robots_id_edit_bt;
    Gtk::Button robots_id_done_bt;
    Gtk::Entry robots_id_box[Robots::SIZE];
    Glib::ustring robots_id_tmp[Robots::SIZE];

    // Edit/Done buttons
    Gtk::Button robots_speed_edit_bt;
    Gtk::Button robots_speed_done_bt;
    Gtk::Button robots_function_edit_bt;
    Gtk::Button robots_function_done_bt;
    Gtk::ComboBoxText cb_robot_function[Robots::SIZE]; /**< Functions dropdown selector */
    Gtk::HScale robots_speed_hscale[Robots::SIZE]; /**< Speed selection sliders */
    Gtk::ProgressBar robots_speed_progressBar[Robots::SIZE]; /**< Bars that show robot's actual speed */

    /**
	 * Generates a Robot Functions frame
	 */
    void create_functions_frame();

    /**
	 * Generates a Speeds control frame
	 */
    void create_speeds_frame();

    /**
     * Generates IDs frame
     */
    void create_ids_frame();

    /**
	 * Updates robot's role after Done button is clicked
	 */
    void event_robots_function_done_bt_signal_clicked();

    /**
     * Event triggered when robot function's Edit button is clicked
     * @details This method may update buttons' label/state and the active element in the combobox
     */
    void event_robots_function_edit_bt_signal_clicked();

    /**
	 * Event triggered when Speeds' Edit button is clicked
	 * @details This method may update buttons' label/state and the value at the speed slider
	 */
    void event_robots_speed_edit_bt_signal_pressed();

    /**
	 * Updates robot's speed after Done button is clicked
	 */
    void event_robots_speed_done_bt_signal_clicked();

    /**
     * Event triggered when Robot Functions' done button is clicked
     * @details Assigns each robot (on RobotGUI) to its new ID
     */
    void event_robots_id_done_bt_signal_clicked();

    /**
     * Event triggered when Robot Functions' edit button is clicked
     * @details Turns robot's ID combobox (non)editable and keeps the changes
     */
    void event_robots_id_edit_bt_signal_pressed();

public:
	/**
	 * Constructor of a RobotGUI object
	 * @details Creates all robot's configuration frames and initialize data.
	 */	
    RobotGUI();

    /**
     * Enables all dynamic buttons and inputs
     */
    void enable_buttons();

    /**
     * Disables all dynamic buttons and inputs
     */
    void disable_buttons();

    /**
	 * Updates robot functions at the screen
	 */
    void update_robot_functions();

    /**
     * Updates the speed sliders after successful load
     */
     void update_speed_sliders();

    /**
	 * Updates the speed progress bar at the screen
	 */
    void update_speed_progressBars();

    /**
     * Updates the IDs frame at screen
     */
     void update_ids();
};

#endif /* ROBOTGUI_HPP_ */
