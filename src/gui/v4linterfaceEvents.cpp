/*
* v4lgui.cpp
*
*  Created on: Feb 1, 2014
*      Author: gustavo
*/

#include "v4linterface.hpp"
#include "filechooser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include "../aux/jsonSaveManager.h"

#define DEFAULT_STR " - "

namespace VSSS_GUI {
    bool V4LInterface::on_button_press_event(GdkEventButton *event) {
        if((event->type == GDK_BUTTON_PRESS)) {
            //  std::cout<<"click"<<std::endl;
            return true;
        } else return false;
    }

    void V4LInterface::HScale_offsetR_value_changed() {
        offsetR = HScale_offsetR.get_value();
    }

    void V4LInterface::HScale_offsetL_value_changed() {
        offsetL = HScale_offsetL.get_value();
    }

    // signals
    void V4LInterface::__event_bt_quick_save_clicked() {
        jsonSaveManager config(this);
        config.save();
        debug_log("Quick save");
    }

    void V4LInterface::__event_bt_save_clicked() {
        FileChooser loadWindow;

        if (loadWindow.result == Gtk::RESPONSE_OK){
            jsonSaveManager config(this);
            config.save(loadWindow.fileName);
        }

        debug_log("Save");
    }

    void V4LInterface::__event_bt_quick_load_clicked() {
        jsonSaveManager config(this);
        config.load();
        update_interface_camera();
        debug_log("Quick load");
    }

    void V4LInterface::__event_bt_load_clicked() {
        FileChooser loadWindow;

        if(loadWindow.result == Gtk::RESPONSE_OK) {
            jsonSaveManager config(this);
            config.load(loadWindow.fileName);
            update_interface_camera();
        }

        debug_log("Load");
    }

    void V4LInterface::__event_bt_start_clicked() {
        if(!vcap.is_opened()) return;

        Glib::ustring label = bt_start.get_label();

        if(0 == label.compare("start")) {
            bool r;
            // = Pre-configure device ======================================

            // set frame size
            r = vcap.set_frame_size(sp_width.get_value_as_int(), sp_height.get_value_as_int(), V4L2_BUF_TYPE_VIDEO_CAPTURE);

            if (!r) std::cout << "Can't set frame size!" << std::endl;

            // = Init memory map buffers ===================================
            //			if (vcap.start_capturing(v4lcap::MEM_READ, 2)) {
            //				std::cout << "Can't map device memory!" << std::endl;
            //				return;
            //			}

            if(!vcap.init_mmap(2)) {
                std::cout << "Can't map device memory!" << std::endl;
                return;
            }

            if(!vcap.start_capturing()) {
                std::cout << "Can't start device!" << std::endl;
                return;
            }

            // = Actualize the displayed frame size ========================
            struct v4l2_format format;
            vcap.get_format(&format, V4L2_BUF_TYPE_VIDEO_CAPTURE);

            sp_width.set_value(format.fmt.pix.width);
            sp_height.set_value(format.fmt.pix.height);

            bt_start.set_label("stop");
            // Botão Stop desabilitado até que arrume o bug do malloc do threshold
            bt_start.set_state(Gtk::STATE_INSENSITIVE);
            cb_device.set_state(Gtk::STATE_INSENSITIVE);
            cb_input.set_state(Gtk::STATE_INSENSITIVE);
            cb_standard.set_state(Gtk::STATE_INSENSITIVE);
            cb_frame_size.set_state(Gtk::STATE_INSENSITIVE);
            cb_format_desc.set_state(Gtk::STATE_INSENSITIVE);
            sp_width.set_state(Gtk::STATE_INSENSITIVE);
            sp_height.set_state(Gtk::STATE_INSENSITIVE);
            cb_frame_interval.set_state(Gtk::STATE_INSENSITIVE);
            bt_warp.set_state(Gtk::STATE_NORMAL);
            bt_quick_save.set_state(Gtk::STATE_NORMAL);
            bt_quick_load.set_state(Gtk::STATE_NORMAL);
            bt_save.set_state(Gtk::STATE_NORMAL);
            bt_load.set_state(Gtk::STATE_NORMAL);
            HScale_offsetR.set_state(Gtk::STATE_NORMAL);
            HScale_offsetL.set_state(Gtk::STATE_NORMAL);
            visionGUI.rb_split_view.set_state(Gtk::STATE_NORMAL);
            visionGUI.rb_original_view.set_state(Gtk::STATE_NORMAL);
            visionGUI.bt_HSV_calib.set_state(Gtk::STATE_NORMAL);
            visionGUI.rb_mode_GMM.set_state(Gtk::STATE_NORMAL);
            visionGUI.rb_mode_HSV.set_state(Gtk::STATE_NORMAL);
            visionGUI.bt_record_video.set_state(Gtk::STATE_NORMAL);
            visionGUI.bt_save_picture.set_state(Gtk::STATE_NORMAL);
            visionGUI.en_video_name.set_state(Gtk::STATE_NORMAL);
            visionGUI.en_picture_name.set_state(Gtk::STATE_NORMAL);
            m_signal_start.emit(true);
        } else {
            if(!vcap.stop_capturing()) {
                std::cout << "Can't stop device!" << std::endl;
                return;
            }

            if(!vcap.uninit_mmap()) {
                std::cout << "Can't unmmap device memory!" << std::endl;
                return;
            }

            bt_start.set_label("start");
            cb_device.set_state(Gtk::STATE_NORMAL);
            cb_input.set_state(Gtk::STATE_NORMAL);
            cb_standard.set_state(Gtk::STATE_NORMAL);
            cb_frame_size.set_state(Gtk::STATE_NORMAL);
            cb_format_desc.set_state(Gtk::STATE_NORMAL);
            sp_width.set_state(Gtk::STATE_NORMAL);
            sp_height.set_state(Gtk::STATE_NORMAL);
            cb_frame_interval.set_state(Gtk::STATE_NORMAL);
            visionGUI.bt_HSV_calib.set_state(Gtk::STATE_INSENSITIVE);
            bt_warp.set_state(Gtk::STATE_INSENSITIVE);
            bt_quick_save.set_state(Gtk::STATE_INSENSITIVE);
            bt_quick_load.set_state(Gtk::STATE_INSENSITIVE);
            bt_save.set_state(Gtk::STATE_INSENSITIVE);
            bt_load.set_state(Gtk::STATE_INSENSITIVE);
            m_signal_start.emit(false);
        }
        return;
    }

    void V4LInterface::__event_bt_warp_clicked() {
        std::cout << "Warp drive engaged" << std::endl;

        if(!imageView.warp_event_flag) {
            imageView.warp_event_flag = true;
            bt_reset_warp.set_state(Gtk::STATE_NORMAL);
            bt_quick_load.set_state(Gtk::STATE_NORMAL);
            bt_quick_save.set_state(Gtk::STATE_NORMAL);
            bt_load.set_state(Gtk::STATE_NORMAL);
            bt_save.set_state(Gtk::STATE_NORMAL);
            //bt_invert_image.set_state(Gtk::STATE_NORMAL);
        } else {
            imageView.warp_event_flag=false;
            bt_reset_warp.set_state(Gtk::STATE_INSENSITIVE);
            bt_quick_load.set_state(Gtk::STATE_INSENSITIVE);
            bt_quick_save.set_state(Gtk::STATE_INSENSITIVE);
            bt_load.set_state(Gtk::STATE_INSENSITIVE);
            bt_save.set_state(Gtk::STATE_INSENSITIVE);
                reset_warp_flag = true;
            //bt_invert_image.set_state(Gtk::STATE_INSENSITIVE);
        }
    }

    void V4LInterface::__event_bt_adjust_pressed() {
        if(!adjust_event_flag) {
            adjust_event_flag=true;
            std::cout << "ADJUST = TRUE" << std::endl;
        } else {
            adjust_event_flag=false;
            std::cout << "ADJUST = FALSE" << std::endl;
        }
    }

    void V4LInterface::__event_bt_reset_warp_clicked() {
        std::cout << "Resetting warp matrix." << std::endl;
        warped = false;
        bt_warp.set_state(Gtk::STATE_NORMAL);
        bt_adjust.set_active(false);
        bt_adjust.set_state(Gtk::STATE_INSENSITIVE);
        adjust_event_flag = false;
        imageView.adjust_rdy=false;
        offsetL = 0;
        offsetR = 0;
        HScale_offsetL.set_value(0);
        HScale_offsetR.set_value(0);
        reset_warp_flag = true;
    }

    void V4LInterface::__event_bt_invert_image_signal_clicked() {
        if(!invert_image_flag) {
            invert_image_flag = true;
            std::cout << "imageView >>>>>>>INVERTED<<<<<<<" << std::endl;
        } else {
            invert_image_flag = false;
            std::cout << "imageView >>>>>>>NORMAL<<<<<<<" << std::endl;
        }
    }

    void V4LInterface::__event_cb_device_changed() {
        if(vcap.is_opened()) vcap.close_device();

        Glib::ustring dev = cb_device.get_active_text();

        if(dev.size() < 1) return;
        if(vcap.open_device(dev.data(), true)) {
            struct v4l2_capability cap;
            vcap.query_capability(&cap);
            camera_card = std::string((const char *) cap.card);

            lb_device_name.set_text(dev.data());
            lb_device_card.set_text((const char *) cap.card);
            lb_device_driver.set_text((const char *) cap.driver);
            lb_device_bus.set_text((const char *) cap.bus_info);
        } else std::cout << "Ooops!" << std::endl;

        __update_all();

        // free memory =============================================================
        Gtk::Widget * page;

        page = notebook.get_nth_page(1);

        while(page) {
            notebook.remove_page(1);
            delete page;
            page = notebook.get_nth_page(1);
        }

        __make_control_list_default();

        __event_bt_quick_load_clicked();

        __make_control_table(ctrl_list_default, "Cam Configs");

        __update_control_widgets(ctrl_list_default);
    }

    void V4LInterface::__event_cb_input_changed() {
        if(cb_input.get_active_row_number() == -1) return;

        Gtk::TreeModel::iterator it = cb_input.get_active();
        Gtk::TreeModel::Row row = *it;
        struct v4l2_input input = row[model_input.m_col_data];

        int r = vcap.set_input(input.index);

        if(!r) std::cout << "Can't set input!" << std::endl;
        __update_all();
    }

    void V4LInterface::__event_cb_standard_changed() {
        if(cb_standard.get_active_row_number() == -1) return;

        Gtk::TreeModel::iterator it = cb_standard.get_active();
        Gtk::TreeModel::Row row = *it;

        struct v4l2_standard standard = row[model_standard.m_col_data];

        int r = vcap.set_standard(standard.id);

        if(!r) std::cout << "Can't set standard!" << std::endl;

        __update_all();
    }

    void V4LInterface::__event_cb_format_desc_changed() {
        if (cb_format_desc.get_active_row_number() == -1) return;

        Gtk::TreeModel::iterator it = cb_format_desc.get_active();
        Gtk::TreeModel::Row row = *it;

        struct v4l2_fmtdesc fmtdesc = row[model_format_desc.m_col_data];

        //int r = vcap.set_pixel_format(fmtdesc.pixelformat, V4L2_BUF_TYPE_VIDEO_CAPTURE);
        int r = vcap.set_pixel_format(fmtdesc.pixelformat);

        if (!r) std::cout << "Can't set format!" << std::endl;

        __update_all();
    }

    void V4LInterface::__event_cb_frame_size_changed() {
        if (cb_frame_size.get_active_row_number() == -1) return;

        Gtk::TreeModel::iterator it = cb_frame_size.get_active();
        Gtk::TreeModel::Row row = *it;

        struct v4l2_frmsizeenum frmsize = row[model_frame_size.m_col_data];

        int r = vcap.set_frame_size(frmsize.discrete.width, frmsize.discrete.height, V4L2_BUF_TYPE_VIDEO_CAPTURE);
        if(!r) std::cout << "Can't set frame size!" << std::endl;

        __update_all();
    }

    void V4LInterface::__event_cb_frame_interval_changed() {
        if (cb_frame_interval.get_active_row_number() == -1) return;

        Gtk::TreeModel::iterator it = cb_frame_interval.get_active();
        Gtk::TreeModel::Row row = *it;

        struct v4l2_frmivalenum frame_interval = row[model_frame_interval.m_col_data];

        int r = vcap.set_frame_interval(frame_interval.discrete);
        if(!r) std::cout << "Can't set frame interval!" << std::endl;

        __update_all();
    }

    bool V4LInterface::__set_control_hscale(int type, double val, std::list<ControlHolder> * list, Gtk::Widget * wctrl) {
        std::list<ControlHolder>::iterator iter;

        for (iter = list->begin(); iter != list->end(); ++iter) {
            if ((*iter).widget == wctrl) break;
        }

        int value = static_cast<Gtk::HScale *>(wctrl)->get_value();
        struct v4l2_queryctrl qctrl = (*iter).qctrl;

        if(!vcap.set_control(qctrl.id, value)) {
            std::cout << "Can not update control [" << qctrl.name << "] with value " << value << std::endl;
            return false;
        }

        struct v4l2_control ctrl;

        if(!vcap.get_control(&ctrl, qctrl.id))
            return false;

        __update_control_widgets(ctrl_list_default);

        return true;
    }

    void V4LInterface::__set_control(std::list<ControlHolder> * list, Gtk::Widget * wctrl) {
        std::list<ControlHolder>::iterator iter;

        for (iter = list->begin(); iter != list->end(); ++iter) {
            if ((*iter).widget == wctrl) break;
        }

        int value;
        struct v4l2_queryctrl qctrl = (*iter).qctrl;

        switch (qctrl.type) {
            case V4L2_CTRL_TYPE_INTEGER:
            case V4L2_CTRL_TYPE_INTEGER64:
            case V4L2_CTRL_TYPE_CTRL_CLASS:
            case V4L2_CTRL_TYPE_BITMASK:
            default:
                break;

            case V4L2_CTRL_TYPE_BOOLEAN:

                value = static_cast<Gtk::CheckButton *>(wctrl)->get_active();
                if (!vcap.set_control(qctrl.id, value))
                    std::cout << "Can not update control [" << qctrl.name << "] with value " << value << std::endl;

                break;

            case V4L2_CTRL_TYPE_BUTTON:

                if (!vcap.set_control(qctrl.id, 1))
                    std::cout << "Can not update control [" << qctrl.name << "] with value " << 1 << std::endl;

                break;

            case V4L2_CTRL_TYPE_STRING:
                break;

            case V4L2_CTRL_TYPE_MENU:
            case V4L2_CTRL_TYPE_INTEGER_MENU:

                Gtk::TreeModel::Children::iterator iter = static_cast<Gtk::ComboBox *>(wctrl)->get_active();
                Gtk::TreeModel::Row row = *iter;
                struct v4l2_querymenu qmenu;
                qmenu = row[model_control_menu.m_col_data];

                if (!vcap.set_control(qctrl.id, qmenu.index))
                    std::cout << "Can not update control [" << qctrl.name << "] with value " << qmenu.name << std::endl;

                break;
        }
        __update_control_widgets(ctrl_list_default);
    }

    void V4LInterface::event_draw_info_checkbox_signal_clicked() {
        draw_info_flag = !draw_info_flag;
    }

    void V4LInterface::event_start_game_bt_signal_clicked() {
        if(!start_game_flag) {
            start_game_flag = true;
            start_game_bt.set_image(red_button_pressed);

            std::string dateString;
            time_t tt;
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            tt = std::chrono::system_clock::to_time_t ( now );
            dateString.append(std::string(ctime(&tt)).substr(0,24));

            if(visionGUI.vision->isRecording()) {
                visionGUI.vision->finishVideo();
                visionGUI.bt_record_video.set_label("REC");
            }

            visionGUI.bt_record_video.set_state(Gtk::STATE_INSENSITIVE);
            visionGUI.en_video_name.set_state(Gtk::STATE_INSENSITIVE);
            visionGUI.en_video_name.set_text(dateString);
            visionGUI.vision->startNewVideo(dateString);

            // Quick save
            jsonSaveManager config(this);
            config.save();
            debug_log("Quick save");

            _evt_play(true); // avisa aos interessados que o jogo foi iniciado
        } else {
            visionGUI.vision->finishVideo();
            visionGUI.bt_record_video.set_state(Gtk::STATE_NORMAL);
            visionGUI.en_video_name.set_state(Gtk::STATE_NORMAL);
            visionGUI.en_video_name.set_text("");
            start_game_flag = false;
            start_game_bt.set_image(red_button_released);

            //Quick Load
            jsonSaveManager config(this);
            config.load();
            update_interface_camera();
            debug_log("Quick load");

            _evt_play(false); // avisa aos interessados que o jogo foi pausado
        }
    }

    void V4LInterface::update_interface_camera() {
        visionGUI.HScale_Hmin.set_value(visionGUI.vision->getHue(visionGUI.Img_id, 0));
        visionGUI.HScale_Hmax.set_value(visionGUI.vision->getHue(visionGUI.Img_id, 1));
        visionGUI.HScale_Smin.set_value(visionGUI.vision->getSaturation(visionGUI.Img_id, 0));
        visionGUI.HScale_Smax.set_value(visionGUI.vision->getSaturation(visionGUI.Img_id, 1));
        visionGUI.HScale_Vmin.set_value(visionGUI.vision->getValue(visionGUI.Img_id, 0));
        visionGUI.HScale_Vmax.set_value(visionGUI.vision->getValue(visionGUI.Img_id, 1));

        visionGUI.HScale_Dilate.set_value(visionGUI.vision->getDilate(visionGUI.Img_id));
        visionGUI.HScale_Erode.set_value(visionGUI.vision->getErode(visionGUI.Img_id));
        visionGUI.HScale_Blur.set_value(visionGUI.vision->getBlur(visionGUI.Img_id));
        visionGUI.HScale_Amin.set_value(visionGUI.vision->getAmin(visionGUI.Img_id));

        if (warped) {
            bt_warp.set_state(Gtk::STATE_INSENSITIVE);
            if (imageView.adjust_rdy)
                bt_adjust.set_state(Gtk::STATE_INSENSITIVE);
            else
                bt_adjust.set_state(Gtk::STATE_NORMAL);
        } else {
            bt_warp.set_state(Gtk::STATE_NORMAL);
            bt_adjust.set_state(Gtk::STATE_INSENSITIVE);
        }

        if (warped || imageView.adjust_rdy)
            bt_reset_warp.set_state(Gtk::STATE_NORMAL);
        else
            bt_reset_warp.set_state(Gtk::STATE_INSENSITIVE);

        HScale_offsetL.set_value(offsetL);
        HScale_offsetR.set_value(offsetR);

        __update_control_widgets(ctrl_list_default);
    }
}
