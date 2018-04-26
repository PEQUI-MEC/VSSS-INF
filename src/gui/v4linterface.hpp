/**
 * @file v4linterface.hpp
 * 
 * */

#ifndef V4LINTERFACE_HPP_
#define V4LINTERFACE_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <gtkmm.h>
#include <gtkmm/messagedialog.h>
#include <linux/videodev2.h>
#include "../capture/v4lcap.hpp"
#include "imageView.hpp"
#include "visionGUI.hpp"
#include "robotGUI.hpp"
#include <ctime>
#include <chrono>

#define PI 3.14159265453

namespace VSSS_GUI {
    typedef struct __ctrl_holder {
        struct v4l2_queryctrl qctrl;
        Gtk::Widget *widget;
        sigc::connection con;
    } ControlHolder;

    class V4LInterface : public Gtk::VBox {
    public:

        ImageView imageView;
        VisionGUI visionGUI;
        RobotGUI robotGUI;

        std::string camera_card;
        std::list <ControlHolder> ctrl_list_default;

        bool warped = false;

        double ballX, ballY;

        Gtk::Image red_button_released;
        Gtk::Image red_button_pressed;

        Gtk::Label *robot1_pos_lb, *robot2_pos_lb, *robot3_pos_lb;
        Gtk::Label *ball_pos_lb;

        Gtk::Frame robots_id_fm;

        Gtk::HBox info_hbox;

        Gtk::VBox robots_pos_vbox;
        Gtk::HBox robots_pos_hbox[7];
        Gtk::HBox start_game_hbox;
        Gtk::VBox buttons_vbox;
        std::vector <std::string> robot_pos;
        Gtk::Button start_game_bt;

        Gtk::Frame robots_pos_fm;
        Gtk::Frame robots_buttons_fm;
        Gtk::Frame robots_checkbox_fm;
        Gtk::VBox robots_pos_buttons_vbox;
        Gtk::HBox fps_hbox;
        Gtk::Label *fps_label;
        Gtk::CheckButton draw_info_checkbox;
        Gtk::HBox draw_info_hbox;
        bool draw_info_flag = false;

        bool reset_warp_flag;

        Gtk::VBox robots_id_vbox;
        Gtk::HBox robots_id_hbox[4];
        Gtk::Button robots_id_edit_bt;
        Gtk::Button robots_id_done_bt;
        Gtk::Entry robots_id_box[3];
        Glib::ustring robots_id_tmp[3];
        bool robots_id_edit_flag = false;

        v4lcap vcap;
        
		 /**
		 * @brief Constructor of a V4LInterface object
		 * @details Initializes variables, sets interface components and attaches events to methods
		 */	
        V4LInterface();

        int offsetL;
        int offsetR;
        Gtk::Scale HScale_offsetL;
        Gtk::Scale HScale_offsetR;
        bool adjust_event_flag;
        bool invert_image_flag = false;

        Gtk::ToggleButton bt_warp;
        Gtk::ToggleButton bt_adjust;
        Gtk::CheckButton bt_invert_image;
        	
        void grab_rgb(unsigned char *rgb);

        // Signals
        sigc::connection cb_input_signal;
        sigc::connection cb_standard_signal;
        sigc::connection cb_format_desc_signal;
        sigc::connection cb_frame_size_signal;
        sigc::connection cb_frame_interval_signal;

        /**
		* @brief Event triggered when 'Quick Save' button is clicked
		*/	
        void __event_bt_quick_save_clicked();
        
        /**
		 * @brief Event triggered when 'Quick Load' button is clicked
		 */	
        void __event_bt_quick_load_clicked();
        
        /**
		 * @brief Event triggered when 'Save' button is clicked
		 */	
        void __event_bt_save_clicked();
		
		/**
		 * @brief Event triggered when 'Load' button is clicked
		 */	
        void __event_bt_load_clicked();

        bool __core_save(const char *);

        bool __core_load(const char *);

        /**
		 * @brief Event triggered when 'start' button is clicked
		 * @details This method triggers the pre-configuration and start of capture device
		 */	
        void __event_bt_start_clicked();

        /**
		 * @brief Event triggered when 'Warp' button is clicked
		 * @details This method enable/disable reset, load and save buttons
		 */			
        void __event_bt_warp_clicked();
		
		/**
		 * @brief Event triggered when 'Adjust' button is pressed
		 * @details Switches adjust_event_flag value
		 */	
        void __event_bt_adjust_pressed();

		/**
		 * @brief Event triggered when 'Reset' button is pressed
		 * @details This method resets image offsets 
		 */			
        void __event_bt_reset_warp_clicked();
        
		/**
		 * @brief Updates the right offset value when the slider position is changed
		 */	
        void HScale_offsetR_value_changed();
		
		/**
		 * @brief Updates the left offset value when the slider position is changed
		 */	
        void HScale_offsetL_value_changed();

		/**
		 * @brief Switches invert_image_flag value when 'invert image' is clicked
		 */	
        void __event_bt_invert_image_signal_clicked();
		
		/**
		 * @brief Event triggered when a new device is selected in Device combobox
		 * @details Updates information about capture device (device, card, driver, bus) and Device Prop frame
		 */	
        void __event_cb_device_changed();
		
		/**
		 * @brief Event triggered when a new input is selected in Input combobox
		 * @details Sets the selected input
		 */	
        void __event_cb_input_changed();

		/**
		 * @brief Event triggered when a new stantard is selected in Standard combobox
		 * @details Sets the selected standard
		 */	
        void __event_cb_standard_changed();

		/**
		 * @brief Event triggered when a new format is selected in Format combobox
		 * @details Sets the selected format
		 */	
        void __event_cb_format_desc_changed();
		
		/**
		 * @brief Event triggered when a new frame size is selected in Frame size combobox
		 * @details Sets the selected image frame
		 */	
        void __event_cb_frame_size_changed();
		
		/**
		 * @brief Event triggered when a new interval is selected in Interval combobox
		 * @details Changes the image frame rate
		 */	
        void __event_cb_frame_interval_changed();

		/**
		 * @brief Creates IDs frame
		 * @details IDs frame associates robots (1,2,3) and hardware (A, B, C,...)
		 */	
        void createIDsFrame();

		/**
		 * @brief Switches draw_info_flag value
		 */	
        void event_draw_info_checkbox_signal_clicked();

		/**
		 * @brief Create Positions frame
		 * @details Frame that shows ball and robots positions 
		 */	
        void createPositionsAndButtonsFrame();

        /**
        * @brief Event triggered when start button is clicked. Starts/finishes video recording.
        */
        void event_start_game_bt_signal_clicked();
        
        /**
        * @brief Event triggered when Robot Functions' done button is clicked
        * @details Assigns each robot (on RobotGUI) to its new ID
        */
        void event_robots_id_done_bt_signal_clicked();
        
        /**
        * @brief Event triggered when Robot Functions' edit button is clicked
        * @details Turns robot's ID combobox (non)editable and keeps the changes
        */
        void event_robots_id_edit_bt_signal_pressed();

        /**
        * @brief Updates robots and ball positions on interface (Positions frame)
        */
        void updateRobotLabels();

        /**
        * @brief Updates the interface's FPS value (Positions frame)
        * @param fps A new fps value
        */
        void updateFPS(int fps);

        /**
        * @return bool start_game_flag
        */
        bool get_start_game_flag();

        /**
        * @brief Updates all robots attributes at RobotGUI
        */
        void update_interface_robots();

        /**
        * @brief Updates Calibration parameters at Vision GUI, Offset values and calls __update_control_widgets
        */        
        void update_interface_camera();

        //Signals
        typedef sigc::signal<bool, bool> SignalStart;

        SignalStart signal_start() {
            return m_signal_start;
        }

    protected:

        SignalStart m_signal_start;
        
        /**
        * @brief Check if the event is a left button click.
        */
        bool on_button_press_event(GdkEventButton *event);

    private:

        bool start_game_flag = false;
        
        /**
        * @brief Initilizes Device Properties combo boxes (Input, Format, Intervals,Standard, Frame Size)
        */
        void __init_combo_boxes();
        
        /**
        * @brief Adds Device Information frame and its inner elements
        */
        void __create_frm_device_info();
        
        /**
        * @brief Adds Device Properties frame and its inner elements
        */
        void __create_frm_device_properties();
        
        /**
        * @brief Creates Save/Load frame
        */
        void createQuickActionsFrame();

        /**
        * @brief Adds Warp frame and its inner elements
        */
        void __create_frm_warp();

        // Combo properties updates
        void __update_cb_device();

        void __update_cb_input();

        void __update_cb_standard();

        void __update_cb_format_desc();

        void __update_cb_frame_size();

        void __update_cb_frame_interval();
        
        /**
        * @brief Update all combo box inputs on CaptureGUI 
        */
        void __update_all();


        Gtk::Frame frm_device_info;
        Gtk::ComboBoxText cb_device;
        Gtk::Button bt_start;
        Gtk::Button bt_reset_warp;

        Gtk::Frame frm_quick_actions;
        Gtk::Button bt_quick_save;
        Gtk::Button bt_quick_load;
        Gtk::Button bt_save;
        Gtk::Button bt_load;

        Gtk::Label lb_device_name;
        Gtk::Label lb_device_card;
        Gtk::Label lb_device_driver;
        Gtk::Label lb_device_bus;
        Gtk::Label right_offset_label;
        Gtk::Label left_offset_label;

        Gtk::Frame frm_device_prop;
        Gtk::Frame frm_warp;
        Gtk::SpinButton sp_width;
        Gtk::SpinButton sp_height;

        Gtk::ComboBox cb_input;
        Gtk::ComboBox cb_standard;
        Gtk::ComboBox cb_format_desc;
        Gtk::ComboBox cb_frame_size;
        Gtk::ComboBox cb_frame_interval;

        Glib::RefPtr <Gtk::ListStore> ls_input;
        Glib::RefPtr <Gtk::ListStore> ls_standard;
        Glib::RefPtr <Gtk::ListStore> ls_format_desc;
        Glib::RefPtr <Gtk::ListStore> ls_frame_size;
        Glib::RefPtr <Gtk::ListStore> ls_frame_interval;

        template<class T>
        class ModelColumn : public Gtk::TreeModel::ColumnRecord {
        public:

            ModelColumn() {
                add(m_col_name);
                add(m_col_data);
            }

            Gtk::TreeModelColumn <Glib::ustring> m_col_name;
            Gtk::TreeModelColumn <T> m_col_data;
        };


        typedef ModelColumn<struct v4l2_input> ModelInput;
        typedef ModelColumn<struct v4l2_standard> ModelStandard;
        typedef ModelColumn<struct v4l2_fmtdesc> ModelFormatDesc;
        typedef ModelColumn<struct v4l2_frmsizeenum> ModelFrameSize;
        typedef ModelColumn<struct v4l2_querymenu> ModelControlMenu;
        typedef ModelColumn<struct v4l2_frmivalenum> ModelFrameInterval;

        ModelInput model_input;
        ModelStandard model_standard;
        ModelFormatDesc model_format_desc;
        ModelFrameSize model_frame_size;
        ModelFrameInterval model_frame_interval;

        ModelControlMenu model_control_menu;

        Gtk::Notebook notebook;
        Gtk::Notebook notebook2;
        
        void __make_controls();
        
        void __make_control_list_default();

        void __make_control_table(std::list <ControlHolder> &list, const char *title);

        void __update_control_widgets(std::list <ControlHolder> &list);

        void __block_control_signals(std::list <ControlHolder> &list, bool block);

        bool __set_control_hscale(int type, double val, std::list <ControlHolder> *list, Gtk::Widget *wctrl);

        void __set_control(std::list <ControlHolder> *list, Gtk::Widget *wctrl);
    };
}

#endif /* V4LINTERFACE_HPP_ */
