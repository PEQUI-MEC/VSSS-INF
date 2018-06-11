/** 
 * @file controlGUI.hpp
 * @since 2016/05/08
 * @author Daniel 
 * 
 * Este código cria a interface da aba 'Control' do código do VS.
 * Para liberar os widgets, o botão PID deve estar pressionado.
 * Nas caixas de texto, pode-se usar '.' ou ',' para tratar números
 * do tipo double.
 *
 * Conta com uma Hscale e uma Entry (o TextBox do Gtkmm) para que possa
 * colocar o valor do PID de cada jogador.
 */

#ifndef CONTROLGUI_HPP_
#define CONTROLGUI_HPP_

#include <gtkmm.h>
#include <string>
//#include "../serialW.hpp"
#include "../communication/flyingMessenger.hpp"
#include "testFrame.hpp"
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
// system_clock::now
#include <iostream>
#include <ctime>
#include <chrono>

class ControlGUI: public Gtk::VBox {
public:
	//SerialW s;
	FlyingMessenger FM;

	const static int TOTAL_ROBOTS = 7;

	TestFrame testFrame;
	
	double acc_time;
	int time_count;
	bool Serial_Enabled;
	int frameskipper;
	/**Flag that indicates whether PID button is pressed or not*/
	bool PID_test_flag = false;
	//Graphics interface content containers
	Gtk::Frame Serial_fm;
	Gtk::Frame Test_fm;
	Gtk::HBox Top_hbox;
	Gtk::VBox Serial_vbox;
	Gtk::VBox Test_vbox;
	Gtk::HBox Serial_hbox[4];
	Gtk::Label *label;
	Gtk::Button bt_send_cmd;
	Gtk::Entry send_cmd_box;
	Gtk::Entry entry_set_frameskip;

	// Buttons and combobox radio
	Gtk::Button bt_Serial_Start;
	Gtk::Button bt_Robot_Status;
	Gtk::Button bt_Serial_Refresh;
	Gtk::Button bt_set_frameskip;
	Gtk::ComboBoxText cb_serial;
	Gtk::ToggleButton button_PID_Test;
	Gtk::Button bt_Serial_test;
	Gtk::ComboBoxText cb_test;
	Gtk::Entry Tbox_V1;
	Gtk::Entry Tbox_V2;

	Gtk::Grid status_grid;
	Gtk::Frame status_fm;
	Gtk::Image status_img[TOTAL_ROBOTS];
	Gtk::Label robots_lb[TOTAL_ROBOTS];
	Gtk::Label status_lb[TOTAL_ROBOTS];
	Gtk::Label lastUpdate_lb;
	Gtk::Label time_msgs;
	Gtk::ProgressBar battery_bar[TOTAL_ROBOTS];

	Gtk::Frame pid_fm;
	Gtk::VBox pid_vbox;
	Gtk::HBox pid_hbox[2];
	Gtk::Button pid_edit_bt;
	Gtk::Button pid_send_bt;
	Gtk::Entry pid_box[3];
	Glib::ustring pid_tmp[4];
	Gtk::ComboBoxText cb_pid_robot;
	Gtk::ComboBoxText cb_pid_type;
	bool pid_edit_flag = false;
	
	/** 
	* @brief Constructor of a ControlGUI object
	* @details Append elements to the interface. Attaches interface signals (pressing, clicking) to events (methods)
	*/
	ControlGUI();

	/** 
	* @brief Creates the Text frame on Control Gui
	*/
	void configureTestFrame();

	/** 
	* @brief Sends the command of Serial's text label using Flying Messenger's send_old_format method
	*/
	void _send_command();

	/** 
	* @brief Switches PID_test_flag value
	*/
	void _PID_Test();
	
	/** 
	* @brief Translates battery message and calls updateInterfaceStatus sending it as a parameter
	* @param buf String with the message to be sent to the battery
	* @param id A robot's id
	*/
	void handleBatteryMsg(char buf[12], int id);
	
	/** 
	* @brief Updates a single robot's battery status
	* @param battery Battery percentage of a robot
	* @param id A robot's id
	*/
	void updateInterfaceStatus(double battery, int id);

	/** 
	* @brief Update the battery status of all robots
	*/
	void _robot_status();

	/** 
	* @brief Initializes the selected communication device using flyingMessenger's start_xbee method
	*/
	void _start_serial();

	/** 
	* @brief Checks if a string is a float number
	* @return bool If the given string is a float, returns true
	*/
	bool isFloat(std::string value);

	/** 
	* @brief Gets the speed text labels from 'Serial' frame and sends them as a command to the selected robot.
	* Call flyingMessenger's send_msg method.
	*/
	void _send_test();

	/** 
	* @brief Updates the list of available communication devices on 'Serial' frame combobox
	*/
	void _update_cb_serial();

	/** 
	* @brief Creates the 'Robot Status' frame on Control Gui
	*/
	void _create_status_frame();

	/** 
	* @param pos Position of the selected robot at Robot Status frame
	* @return char Selected robot's ID (A, B,...,G)
	*/
	char get_robot_id(int pos);

	/** 
	* @param id A Robot's ID (A,B,...,G)
	* @return char Position of the robot at Robot Status frame
	*/
	int get_robot_pos(char id);

	/**
	* @brief Shows the frameskipper value on interface
	*/	
	void set_frameskipper();

	/**
	* @brief Gets the gap between sent messages (through flyingMessenger's get time function) and shows it on interface
	*/	
	void update_msg_time();
};

#endif /* CONTROLGUI_HPP_ */
