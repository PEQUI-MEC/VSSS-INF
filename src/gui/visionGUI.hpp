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

    void __event_bt_HSV_calib_pressed();
    void __event_bt_right_HSV_calib_clicked();
    void __event_bt_left_HSV_calib_clicked();
    void __event_bt_switchMainAdv_clicked();
    void __event_rb_split_mode_clicked();

    void bt_save_picture_clicked();
    void bt_record_video_pressed();

    void __create_frm_hsv();
    void __create_frm_capture();
    void __create_frm_split_view();

    void HScale_Hmin_value_changed();
    void HScale_Smin_value_changed();
    void HScale_Vmin_value_changed();

    void HScale_Hmax_value_changed();
    void HScale_Smax_value_changed();
    void HScale_Vmax_value_changed();
    void HScale_Dilate_value_changed();
    void HScale_Erode_value_changed();
    void HScale_Blur_value_changed();
    void HScale_Amin_value_changed();

    void init_calib_params();

    // void __event_auto_save();
};

#endif /* VISIONGUI_HPP_ */
