#include "visionGUI.hpp"

void VisionGUI::__create_frm_split_view() {
    Gtk::VBox * vbox;
    Gtk::Grid * grid;
    Gtk::Label * label;
    Gtk::Frame * frame;

    vbox = new Gtk::VBox();
    grid = new Gtk::Grid();
    frame = new Gtk::Frame();

    pack_start(*frame, false, false, 5);

    frame->add(*vbox);
    vbox->pack_start(*grid, false, true, 5);
    vbox->set_halign(Gtk::ALIGN_CENTER);
    vbox->set_valign(Gtk::ALIGN_CENTER);

    frame->set_label("Split View");

    grid->set_border_width(5);
    grid->set_column_spacing(10);
    grid->set_row_spacing(5);

    label = new Gtk::Label("Select Mode: ");
    grid->attach(*label, 0, 0, 1, 1);
    rb_original_view.set_label("Original");
    grid->attach(rb_original_view, 1, 0, 1, 1);
    rb_split_view.set_label("Split");
    rb_split_view.join_group(rb_original_view);
    grid->attach(rb_split_view, 2, 0, 1, 1);

    rb_original_view.set_state(Gtk::STATE_INSENSITIVE);
    rb_split_view.set_state(Gtk::STATE_INSENSITIVE);

    rb_original_view.signal_clicked().connect(sigc::mem_fun(*this, &VisionGUI::__event_rb_split_mode_clicked));
    rb_split_view.signal_clicked().connect(sigc::mem_fun(*this, &VisionGUI::__event_rb_split_mode_clicked));
}

void VisionGUI::__event_rb_split_mode_clicked() {
    isSplitView = rb_split_view.get_active();
}

bool VisionGUI::getIsSplitView() {
  return isSplitView;
}

void VisionGUI::__create_frm_capture() {
    Gtk::VBox * vbox;
    Gtk::Grid * grid;
    Gtk::Label * label;
    Gtk::Frame * frame;

    vbox = new Gtk::VBox();
    grid = new Gtk::Grid();
    frame = new Gtk::Frame();

    pack_start(*frame, false, false, 5);

    frame->add(*vbox);
    vbox->pack_start(*grid, false, true, 5);
    vbox->set_halign(Gtk::ALIGN_CENTER);
    vbox->set_valign(Gtk::ALIGN_CENTER);

    frame->set_label("Video/Image Capture");

    grid->set_border_width(5);
    grid->set_column_spacing(10);
    grid->set_row_spacing(5);

    label = new Gtk::Label("Picture Name:");
    label->set_alignment(1.0);
    grid->attach(*label, 0, 0, 1, 1);
    label = new Gtk::Label("Video Name:");
    label->set_alignment(1.0);
    grid->attach(*label, 0, 1, 1, 1);

    grid->attach(en_picture_name, 1, 0, 1, 1);
    grid->attach(en_video_name, 1, 1, 1, 1);

    bt_save_picture.set_label("Save");
    bt_record_video.set_label("REC");
    grid->attach(bt_save_picture, 2, 0, 1, 1);
    grid->attach(bt_record_video, 2, 1, 1, 1);

    en_video_name.set_state(Gtk::STATE_INSENSITIVE);
    en_picture_name.set_state(Gtk::STATE_INSENSITIVE);
    bt_record_video.set_state(Gtk::STATE_INSENSITIVE);
    bt_save_picture.set_state(Gtk::STATE_INSENSITIVE);

    bt_record_video.signal_pressed().connect(sigc::mem_fun(*this, &VisionGUI::bt_record_video_pressed));
    bt_save_picture.signal_clicked().connect(sigc::mem_fun(*this, &VisionGUI::bt_save_picture_clicked));
}

void VisionGUI::bt_record_video_pressed() {
    if(vision->isRecording()) {
        vision->finishVideo();
        bt_record_video.set_label("REC");
        en_video_name.set_text("");
        en_video_name.set_state(Gtk::STATE_NORMAL);
    } else {
        std::string name = en_video_name.get_text();

        bt_record_video.set_label("Stop");
        en_video_name.set_state(Gtk::STATE_INSENSITIVE);
        if(name.empty()) {
          vision->startNewVideo(std::to_string(vidIndex++));
          en_video_name.set_text(std::to_string(vidIndex));
      } else {
          vision->startNewVideo(name);
      }
  }
}

void VisionGUI::bt_save_picture_clicked() {
    std::string name = en_picture_name.get_text();

    if(name.empty()) vision->savePicture(std::to_string(picIndex++));
    else vision->savePicture(name);

    en_picture_name.set_text("");
}

void VisionGUI::__create_frm_hsv() {
    Gtk::VBox * vbox;
    Gtk::Grid * grid;
    Gtk::Label * label;
    Gtk::Frame * frame;

    vbox = new Gtk::VBox();
    grid = new Gtk::Grid();
    frame = new Gtk::Frame();

    pack_start(*frame, false, false, 5);

    frame->add(*vbox);
    vbox->pack_start(*grid, false, true, 5);
    vbox->set_halign(Gtk::ALIGN_CENTER);
    vbox->set_valign(Gtk::ALIGN_CENTER);

    frame->set_label("HSV Calibration");

    grid->set_border_width(5);
    grid->set_column_spacing(15);
    grid->set_row_spacing(0);
    // grid->set_column_homogeneous(true);

    bt_HSV_calib.set_label("HSV Calib.");
    grid->attach(bt_HSV_calib, 0, 0, 1, 1);

    bt_HSV_left.set_label(" < ");
    grid->attach(bt_HSV_left, 2, 0, 1, 1);
    HSV_label.set_text("Main");
    grid->attach(HSV_label, 3, 0, 1, 1);
    bt_HSV_right.set_label(" > ");
    grid->attach(bt_HSV_right, 4, 0, 1, 1);

    bt_switchMainAdv.set_label("Main <-> Adv");
    grid->attach(bt_switchMainAdv, 6, 0, 1, 1);

    grid = new Gtk::Grid();
    grid->set_border_width(5);
    grid->set_column_spacing(15);
    grid->set_row_spacing(0);
    grid->set_column_homogeneous(true);
    vbox->pack_start(*grid, false, true, 5);

    label = new Gtk::Label("Hmin:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 0, 0, 1, 1);

    HScale_Hmin.set_digits(0);
    HScale_Hmin.set_increments(1,1);
    HScale_Hmin.set_range(0,255);
    HScale_Hmin.set_value_pos(Gtk::POS_TOP);
    HScale_Hmin.set_draw_value();
    grid->attach(HScale_Hmin, 1, 0, 2, 1);

    label = new Gtk::Label("Hmax:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 3, 0, 1, 1);

    HScale_Hmax.set_digits(0);
    HScale_Hmax.set_increments(1,1);
    HScale_Hmax.set_range(0,180);
    HScale_Hmax.set_value_pos(Gtk::POS_TOP);
    HScale_Hmax.set_draw_value();
    grid->attach(HScale_Hmax, 4, 0, 2, 1);

    label = new Gtk::Label("Smin:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 0, 1, 1, 1);

    HScale_Smin.set_digits(0);
    HScale_Smin.set_increments(1,1);
    HScale_Smin.set_range(0,255);
    HScale_Smin.set_value_pos(Gtk::POS_TOP);
    HScale_Smin.set_draw_value();
    grid->attach(HScale_Smin, 1, 1, 2, 1);

    label = new Gtk::Label("Smax:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 3, 1, 1, 1);

    HScale_Smax.set_digits(0);
    HScale_Smax.set_increments(1,1);
    HScale_Smax.set_range(0,255);
    HScale_Smax.set_value_pos(Gtk::POS_TOP);
    HScale_Smax.set_draw_value();

    grid->attach(HScale_Smax,4, 1, 2, 1);

    label = new Gtk::Label("Vmin:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 0, 2, 1, 1);

    HScale_Vmin.set_digits(0);
    HScale_Vmin.set_increments(1,1);
    HScale_Vmin.set_range(0,255);
    HScale_Vmin.set_value_pos(Gtk::POS_TOP);
    HScale_Vmin.set_draw_value();
    grid->attach(HScale_Vmin, 1, 2, 2, 1);

    label = new Gtk::Label("Vmax:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 3, 2, 1, 1);

    HScale_Vmax.set_digits(0);
    HScale_Vmax.set_increments(1,1);
    HScale_Vmax.set_range(0,255);
    HScale_Vmax.set_value_pos(Gtk::POS_TOP);
    HScale_Vmax.set_draw_value();

    grid->attach(HScale_Vmax, 4, 2, 2, 1);

    label = new Gtk::Label("Erode:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 0, 3, 1, 1);

    HScale_Erode.set_digits(0);
    HScale_Erode.set_increments(1,1);
    HScale_Erode.set_range(0,50);
    HScale_Erode.set_value_pos(Gtk::POS_TOP);
    HScale_Erode.set_draw_value();
    grid->attach(HScale_Erode, 1, 3, 2, 1);

    label = new Gtk::Label("Dilate:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 3, 3, 1, 1);

    HScale_Dilate.set_digits(0);
    HScale_Dilate.set_increments(1,1);
    HScale_Dilate.set_range(0,50);
    HScale_Dilate.set_value_pos(Gtk::POS_TOP);
    HScale_Dilate.set_draw_value();
    grid->attach(HScale_Dilate, 4, 3, 2, 1);

    label = new Gtk::Label("Blur:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 0, 4, 1, 1);

    HScale_Blur.set_digits(0);
    HScale_Blur.set_increments(2,2);
    HScale_Blur.set_range(3,9);
    HScale_Blur.set_value_pos(Gtk::POS_TOP);
    HScale_Blur.set_draw_value();
    grid->attach(HScale_Blur, 1, 4, 2, 1);

    label = new Gtk::Label("Amin:");
    label->set_alignment(1.0, 1.0);
    grid->attach(*label, 3, 4, 1, 1);

    HScale_Amin.set_digits(0);
    HScale_Amin.set_increments(1,1);
    HScale_Amin.set_range(0,200);
    HScale_Amin.set_value_pos(Gtk::POS_TOP);
    HScale_Amin.set_draw_value();

    grid->attach(HScale_Amin, 4, 4, 2, 1);

    bt_HSV_calib.set_state(Gtk::STATE_INSENSITIVE);
    bt_switchMainAdv.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Hmin.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Smin.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Vmin.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Hmax.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Smax.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Vmax.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Dilate.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Erode.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Blur.set_state(Gtk::STATE_INSENSITIVE);
    HScale_Amin.set_state(Gtk::STATE_INSENSITIVE);
    bt_HSV_left.set_state(Gtk::STATE_INSENSITIVE);
    bt_HSV_right.set_state(Gtk::STATE_INSENSITIVE);

    bt_HSV_calib.signal_pressed().connect(sigc::mem_fun(*this, &VisionGUI::__event_bt_HSV_calib_pressed));
    bt_HSV_right.signal_clicked().connect(sigc::mem_fun(*this, &VisionGUI::__event_bt_right_HSV_calib_clicked));
    bt_HSV_left.signal_clicked().connect(sigc::mem_fun(*this, &VisionGUI::__event_bt_left_HSV_calib_clicked));
    bt_switchMainAdv.signal_clicked().connect(sigc::mem_fun(*this, &VisionGUI::__event_bt_switchMainAdv_clicked));
    HScale_Hmin.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Hmin_value_changed));
    HScale_Hmax.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Hmax_value_changed));
    HScale_Smin.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Smin_value_changed));
    HScale_Smax.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Smax_value_changed));
    HScale_Vmin.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Vmin_value_changed));
    HScale_Vmax.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Vmax_value_changed));
    HScale_Erode.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Erode_value_changed));
    HScale_Dilate.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Dilate_value_changed));
    HScale_Blur.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Blur_value_changed));
    HScale_Amin.signal_value_changed().connect(sigc::mem_fun(*this, &VisionGUI::HScale_Amin_value_changed));
}

void VisionGUI::__event_bt_switchMainAdv_clicked() {
    vision->switchMainWithAdv();
    HScale_Hmin.set_value(vision->getHue(Img_id, 0));
    HScale_Hmax.set_value(vision->getHue(Img_id, 1));
    HScale_Smin.set_value(vision->getSaturation(Img_id, 0));
    HScale_Smax.set_value(vision->getSaturation(Img_id, 1));
    HScale_Vmin.set_value(vision->getValue(Img_id, 0));
    HScale_Vmax.set_value(vision->getValue(Img_id, 1));
    HScale_Amin.set_value(vision->getAmin(Img_id));
    HScale_Blur.set_value(vision->getBlur(Img_id));
    HScale_Erode.set_value(vision->getErode(Img_id));
    HScale_Dilate.set_value(vision->getDilate(Img_id));
}

void VisionGUI::HScale_Hmin_value_changed() {
    vision->setHue(Img_id, 0, (int) HScale_Hmin.get_value());
}

void VisionGUI::HScale_Smin_value_changed() {
    vision->setSaturation(Img_id, 0, (int) HScale_Smin.get_value());
}

void VisionGUI::HScale_Vmin_value_changed() {
    vision->setValue(Img_id, 0, (int) HScale_Vmin.get_value());
}

void VisionGUI::HScale_Hmax_value_changed() {
    vision->setHue(Img_id, 1, (int) HScale_Hmax.get_value());
}

void VisionGUI::HScale_Smax_value_changed() {
    vision->setSaturation(Img_id, 1, (int) HScale_Smax.get_value());
}

void VisionGUI::HScale_Vmax_value_changed() {
    vision->setValue(Img_id, 1, (int) HScale_Vmax.get_value());
}

void VisionGUI::HScale_Amin_value_changed() {
    vision->setAmin(Img_id, (int) HScale_Amin.get_value());
}

void VisionGUI::HScale_Dilate_value_changed() {
    if(HScale_Dilate.get_value() < 0) vision->setDilate(Img_id, 0);
    else vision->setDilate(Img_id, (int) HScale_Dilate.get_value());
}

void VisionGUI::HScale_Erode_value_changed() {
    if(HScale_Erode.get_value() < 0) vision->setErode(Img_id, 0);
    else vision->setErode(Img_id, (int) HScale_Erode.get_value());
}

void VisionGUI::HScale_Blur_value_changed() {
    if(HScale_Blur.get_value() < 3) {
        vision->setBlur(Img_id, 3);
    } else if((int) HScale_Blur.get_value() % 2 == 0){
        vision->setBlur(Img_id, (int) HScale_Blur.get_value()+1);
    } else{
        vision->setBlur(Img_id, (int)HScale_Blur.get_value());
    }
}

void VisionGUI::__event_bt_HSV_calib_pressed() {
    if(HSV_calib_event_flag) {
        HSV_calib_event_flag=false;
        // VisionGUI::__event_auto_save();
        HScale_Hmin.set_state(Gtk::STATE_INSENSITIVE);
        HScale_Smin.set_state(Gtk::STATE_INSENSITIVE);
        HScale_Vmin.set_state(Gtk::STATE_INSENSITIVE);
        HScale_Hmax.set_state(Gtk::STATE_INSENSITIVE);
        HScale_Smax.set_state(Gtk::STATE_INSENSITIVE);
        HScale_Vmax.set_state(Gtk::STATE_INSENSITIVE);
        HScale_Dilate.set_state(Gtk::STATE_INSENSITIVE);
        HScale_Erode.set_state(Gtk::STATE_INSENSITIVE);
        HScale_Blur.set_state(Gtk::STATE_INSENSITIVE);
        HScale_Amin.set_state(Gtk::STATE_INSENSITIVE);
        bt_HSV_right.set_state(Gtk::STATE_INSENSITIVE);
        bt_HSV_left.set_state(Gtk::STATE_INSENSITIVE);
        bt_switchMainAdv.set_state(Gtk::STATE_INSENSITIVE);
    } else {
        HSV_calib_event_flag=true;
        HScale_Hmin.set_state(Gtk::STATE_NORMAL);
        HScale_Smin.set_state(Gtk::STATE_NORMAL);
        HScale_Vmin.set_state(Gtk::STATE_NORMAL);
        HScale_Hmax.set_state(Gtk::STATE_NORMAL);
        HScale_Smax.set_state(Gtk::STATE_NORMAL);
        HScale_Vmax.set_state(Gtk::STATE_NORMAL);
        HScale_Dilate.set_state(Gtk::STATE_NORMAL);
        HScale_Erode.set_state(Gtk::STATE_NORMAL);
        HScale_Blur.set_state(Gtk::STATE_NORMAL);
        HScale_Amin.set_state(Gtk::STATE_NORMAL);
        bt_HSV_right.set_state(Gtk::STATE_NORMAL);
        bt_HSV_left.set_state(Gtk::STATE_NORMAL);
        bt_switchMainAdv.set_state(Gtk::STATE_NORMAL);
    }
}

void VisionGUI::selectFrame(int sector) {
    switch (sector) {
        case 0:

            rb_original_view.set_active(true);
            __event_rb_split_mode_clicked();
            break;

        case 1:

            Img_id = 3;
            rb_original_view.set_active(true);
            __event_rb_split_mode_clicked();
            __event_bt_right_HSV_calib_clicked();

            if(!bt_HSV_calib.get_active()) {
                bt_HSV_calib.set_active(true);
                __event_bt_HSV_calib_pressed();
            }

            break;

        case 2:

            Img_id = 0;
            rb_original_view.set_active(true);
            __event_rb_split_mode_clicked();
            __event_bt_right_HSV_calib_clicked();

            if(!bt_HSV_calib.get_active()) {
                bt_HSV_calib.set_active(true);
                __event_bt_HSV_calib_pressed();
            }

            break;

        case 3:

            Img_id = 1;
            rb_original_view.set_active(true);
            __event_rb_split_mode_clicked();
            __event_bt_right_HSV_calib_clicked();

            if(!bt_HSV_calib.get_active()) {
                bt_HSV_calib.set_active(true);
                __event_bt_HSV_calib_pressed();
            }

            break;

        default:
            break;
    }
}

void VisionGUI::__event_bt_right_HSV_calib_clicked() {
    Img_id = Img_id + 1;

    if(Img_id > 3) Img_id = 0;

    HScale_Hmin.set_value(vision->getHue(Img_id, 0));
    HScale_Hmax.set_value(vision->getHue(Img_id, 1));

    HScale_Smin.set_value(vision->getSaturation(Img_id, 0));
    HScale_Smax.set_value(vision->getSaturation(Img_id, 1));

    HScale_Vmin.set_value(vision->getValue(Img_id, 0));
    HScale_Vmax.set_value(vision->getValue(Img_id, 1));

    HScale_Dilate.set_value(vision->getDilate(Img_id));
    HScale_Erode.set_value(vision->getErode(Img_id));

    HScale_Blur.set_value(vision->getBlur(Img_id));
    HScale_Amin.set_value(vision->getAmin(Img_id));

    switch(Img_id) {
        case 0:

            HSV_label.set_text("Main");
            break;

        case 1:

            HSV_label.set_text("Green");
            break;

        case 2:

            HSV_label.set_text("Ball");
            break;

        case 3:

            HSV_label.set_text("Opp.");
            break;

        default:
            break;
    }
}

void VisionGUI::__event_bt_left_HSV_calib_clicked() {
    Img_id = Img_id-1;

    if(Img_id < 0) Img_id = 3;

    HScale_Hmin.set_value(vision->getHue(Img_id, 0));
    HScale_Hmax.set_value(vision->getHue(Img_id, 1));

    HScale_Smin.set_value(vision->getSaturation(Img_id, 0));
    HScale_Smax.set_value(vision->getSaturation(Img_id, 1));

    HScale_Vmin.set_value(vision->getValue(Img_id, 0));
    HScale_Vmax.set_value(vision->getValue(Img_id, 1));

    HScale_Dilate.set_value(vision->getDilate(Img_id));
    HScale_Erode.set_value(vision->getErode(Img_id));

    HScale_Blur.set_value(vision->getBlur(Img_id));
    HScale_Amin.set_value(vision->getAmin(Img_id));

    switch(Img_id) {
        case 0:

            HSV_label.set_text("Main");
            break;

        case 1:

            HSV_label.set_text("Green");
            break;

        case 2:

            HSV_label.set_text("Ball");
            break;

        case 3:

            HSV_label.set_text("Opp.");
            break;

        default:
            break;
    }
}

void VisionGUI::init_calib_params() {
    // Inicializar variáveis de calibração
    int H[5][2] = { {0,180}, {0,180}, {0,180}, {0,180}, {0,180} };
    int S[5][2] = { {0, 255}, {0, 255}, {0, 255}, {0, 255}, {0, 255} };
    int V[5][2] = { {0, 255}, {0, 255}, {0, 255}, {0, 255}, {0, 255} };
    int B[5] {3, 3, 3, 3, 3};
    int D[5] = {0, 0, 0, 0, 0};
    int E[5] = {0, 0, 0, 0, 0};
    int Amin[5] = {150, 90, 90, 115, 50};

    // Configurar os valores iniciais de calibração
    vision->setCalibParams(H, S, V, Amin, E, D, B);

    // Corrigir os valores mostrados na interface
    HScale_Hmax.set_value(H[0][1]);
    HScale_Smax.set_value(S[0][1]);
    HScale_Vmax.set_value(V[0][1]);
    HScale_Amin.set_value(Amin[0]);
}

void VisionGUI::setFrameSize(int inWidth, int inHeight) {
    vision->setFrameSize(inWidth, inHeight);
}

VisionGUI::VisionGUI() :
  HSV_calib_event_flag(false), Img_id(0), vidIndex(0), picIndex(0), isSplitView(false)
{

    vision = new Vision(640, 480);

    __create_frm_capture();
    __create_frm_split_view();
    __create_frm_hsv();

    init_calib_params();
}

VisionGUI::~VisionGUI() {
    if(vision->isRecording()) vision->finishVideo();
}
