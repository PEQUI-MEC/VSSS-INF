/** 
* @file visionGUI.hpp
*/

#ifndef VISIONGUI_HPP_
#define VISIONGUI_HPP_

#include <gtkmm.h>
#include "../vision/vision.hpp"
#include "filechooser.hpp"

class VisionGUI: public Gtk::VBox {
public:

    Vision *vision;
    Gtk::ToggleButton bt_HSV_calib;

    int Img_id;

    bool HSV_calib_event_flag;

    Gtk::RadioButton rb_original_view, rb_split_view;

    Gtk::Scale HScale_Hmin;
    Gtk::Scale HScale_Smin;
    Gtk::Scale HScale_Vmin;
    Gtk::Scale HScale_Hmax;
    Gtk::Scale HScale_Smax;
    Gtk::Scale HScale_Vmax;

    Gtk::Scale HScale_Dilate;
    Gtk::Scale HScale_Erode;

    Gtk::Scale HScale_Blur;

    Gtk::Scale HScale_Amin;

    Gtk::ToggleButton bt_record_video;
    Gtk::Button bt_save_picture;
    Gtk::Entry en_video_name, en_picture_name;

    // Frame Calibration Mode
    Gtk::RadioButton rb_mode_GMM, rb_mode_HSV;

    void selectFrame(int sector);

    void setFrameSize(int inWidth, int inHeight);
    bool getIsSplitView();

    VisionGUI();
    ~VisionGUI();

private:

    // Frame Split View
    bool isSplitView;

    // Frame Capture
    int picIndex, vidIndex;

    // Frame HSV Calibration
    Gtk::Label HSV_label;
    Gtk::Button bt_HSV_left;
    Gtk::Button bt_HSV_right;
    Gtk::Button bt_switchMainAdv;
	
	/**
	* @brief Event triggered when HSV Calib button is pressed
	* @details Switches HSV_calib_event_flag value and turns calibration parameters (non)editable
	*/
    void __event_bt_HSV_calib_pressed();

    /**
	* @brief Event triggered when HSV Calib right button is pressed
	* @details Jump to the next calibration category and loads its parameters values
	*/
    void __event_bt_right_HSV_calib_clicked();

    /**
	* @brief Event triggered when HSV Calib left button is pressed
	* @details Jump to the previous calibration category and loads its parameters values
	*/
    void __event_bt_left_HSV_calib_clicked();
    
    /**
	* @brief Event triggered when 'Main <-> Adv' button is pressed
	* @details Switches host team and adversary calibration parameters 
	*/
    void __event_bt_switchMainAdv_clicked();
    
    /**
	* @brief Turns 'isSplitView' flag active when triggered when Split mode is selected
	*/ 
    void __event_rb_split_mode_clicked();

    /**
	* @brief Saves the actual frame image in /media/pictures when 'save' button is clicked
	* If 'Picture name' textfield is empty, a numeric ID value will be assigned 
	*/
    void bt_save_picture_clicked();

    /**
	* @brief Starts video recording when 'REC' button is pressed, changing its label to 'Stop', and vice-versa.
	* If 'Video name' textfield is empty, a numeric ID value will be assigned. 
	*/
    void bt_record_video_pressed();

    /**
	* @brief Creates HSV calibration frame and its inner elements. Ataches interface signals (button pressing/clicking, values change) to events
	*/
    void __create_frm_hsv();

    /**
	* @brief Creates HSV calibration frame and its inner elements. Ataches interface signals (button pressing/clicking) to events
	*/
    void __create_frm_capture();
    
    /**
	* @brief Creates Split View frame and its inner elements. Ataches interface signals (signals pressing/clicking) to events
	*/
    void __create_frm_split_view();

    /**
	* @brief Call Vision's setHue method passsing Hmin value as a parameter
	*/
    void HScale_Hmin_value_changed();

    /**
	* @brief Call Vision's setSaturation method passsing Smin value as a parameter
	*/
    void HScale_Smin_value_changed();
    
    /**
	* @brief Call Vision's setValue method passsing Vmin value as a parameter
	*/    
    void HScale_Vmin_value_changed();

    /**
	* @brief Call Vision's setHue method passsing Hmax value as a parameter
	*/
    void HScale_Hmax_value_changed();

    /**
	* @brief Call Vision's setSaturation method passsing Smax value as a parameter
	*/
    void HScale_Smax_value_changed();

    /**
	* @brief Call Vision's setValue method passsing Vmax value as a parameter
	*/
    void HScale_Vmax_value_changed();
    
    /**
	* @brief Call Vision's setDilate method passsing Dilate value as a parameter
	*/
    void HScale_Dilate_value_changed();
    
    /**
	* @brief Call Vision's Erode method passsing Erode value as a parameter
	*/    
    void HScale_Erode_value_changed();
    
    /**
	* @brief Call Vision's setBlur method passsing Blur value as a parameter
	*/
    void HScale_Blur_value_changed();

    /**
	* @brief Call Vision's setAmin method passsing Amin value as a parameter
	*/    
    void HScale_Amin_value_changed();

    /**
	* @brief Sets initial calibration parameter values shown at interface
	*/
    void init_calib_params();

    // void __event_auto_save();
};

#endif /* VISIONGUI_HPP_ */
