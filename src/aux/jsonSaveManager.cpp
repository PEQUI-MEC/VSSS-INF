#include "jsonSaveManager.h"

using std::string;

void jsonSaveManager::load(const string file_path) {
    int error = read_configs_from_file(file_path);

    if(error == 1) debug_warning("File " + file_path + " not found");
    else if(error == 2) debug_warning(file_path + " is not a valid JSON file");

    if(!error){
        load_camera();
        load_robots();
    }
}

void jsonSaveManager::save(const string file_path) {
    int error = read_configs_from_file(file_path);
    if(error == 1) debug_log("Creating " + file_path);
    else if(error == 2) debug_warning(file_path + " is not a valid JSON file, will be overwritten");

    save_camera();
    save_robots();
    write_configs_to_file(file_path);
}

void jsonSaveManager::save_robots() {
    for (int i = 0; i < Robots::SIZE; ++i) {
        json& robot_config = configs["Robots"]["Robot "+std::to_string(i+1)];
        // debug_warning(string(1, Robots::get_ID(i)));
        robot_config["ID"] = string(1, Robots::get_ID(i));
        robot_config["role"] = Robots::get_role(i);
        robot_config["speed"] = to_string(round(Robots::get_default_velocity(i)*1000)/1000); // salvar como string pois double zoa as casas decimais
    }
}

void jsonSaveManager::load_robots() {
    if(!exists(configs, "Robots")) return;
    for (int i = 0; i < Robots::SIZE; ++i) {
        json& robot_config = configs["Robots"]["Robot "+std::to_string(i+1)];

        if(exists(robot_config, "ID")){
            string id = robot_config["ID"];
            // debug_log("setting id: " + id);
            Robots::set_ID(i, id[0]);
        }
        if(exists(robot_config, "role")) {
            Robots::set_role(i, robot_config["role"]);
        }
        if(exists(robot_config, "speed")){
            string tmp_speed = robot_config["speed"];
            Robots::set_default_velocity(i, stof(tmp_speed));
        }
    }
    interface->robotGUI.update_ids();
    interface->robotGUI.update_robot_functions();
    interface->robotGUI.update_speed_sliders();
}

void jsonSaveManager::save_camera() {
    json& camera_config = configs["Cameras"][interface->camera_card];
    string hsv_calibs[4] = {"Main", "Green", "Ball", "Opp."};

    for (int i = 0; i < 4; ++i) {
        json& hsv = camera_config["HSV Calibration"][hsv_calibs[i]];
        Vision& vision = *(interface->visionGUI.vision);

        hsv["hue_min"] = vision.getHue(i, 0);
        hsv["hue_max"] = vision.getHue(i, 1);
        hsv["saturation_min"] = vision.getSaturation(i, 0);
        hsv["saturation_max"] = vision.getSaturation(i, 1);
        hsv["value_min"] = vision.getValue(i, 0);
        hsv["value_max"] = vision.getValue(i, 1);
        hsv["dilate"] = vision.getDilate(i);
        hsv["erode"] = vision.getErode(i);
        hsv["blur"] = vision.getBlur(i);
        hsv["amin"] = vision.getAmin(i);
    }

    if(interface->warped) {
        json &warp_mat = camera_config["warp_mat"];
        config_matrix(warp_mat, interface->imageView.warp_mat, true);
    }

    if(interface->imageView.adjust_rdy) {
        json &adjust_mat = camera_config["adjust_mat"];
        config_matrix(adjust_mat, interface->imageView.adjust_mat, true);
    }

    camera_config["offsetL"] = interface->offsetL;
    camera_config["offsetR"] = interface->offsetR;

    json& properties_config = camera_config["Camera Properties"];
    for(ControlHolder ctrl : interface->ctrl_list_default) {
        v4l2_control control{};
        interface->vcap.get_control(&control, ctrl.qctrl.id);
        string name = string((char *) ctrl.qctrl.name);
        properties_config[name] = control.value;
    }
}

void jsonSaveManager::load_camera() {
    if(!exists(configs["Cameras"], interface->camera_card)) return;
    json& camera_config = configs["Cameras"][interface->camera_card];

    string hsv_calibs[4] = {"Main", "Green", "Ball", "Opp."};
    for (int i = 0; i < 4; ++i) {
        json& hsv = camera_config["HSV Calibration"][hsv_calibs[i]];
        Vision& vision = *(interface->visionGUI.vision);

        if(exists(hsv, "hue_min")) vision.setHue(i, 0, hsv["hue_min"]);
        if(exists(hsv, "hue_max")) vision.setHue(i, 1, hsv["hue_max"]);
        if(exists(hsv, "saturation_min")) vision.setSaturation(i, 0, hsv["saturation_min"]);
        if(exists(hsv, "saturation_max")) vision.setSaturation(i, 1, hsv["saturation_max"]);
        if(exists(hsv, "value_min")) vision.setValue(i, 0, hsv["value_min"]);
        if(exists(hsv, "value_max")) vision.setValue(i, 1, hsv["value_max"]);
        if(exists(hsv, "dilate")) vision.setDilate(i, hsv["dilate"]);
        if(exists(hsv, "erode")) vision.setErode(i, hsv["erode"]);
        if(exists(hsv, "blur")) vision.setBlur(i, hsv["blur"]);
        if(exists(hsv, "amin")) vision.setAmin(i, hsv["amin"]);
    }

    if(exists(camera_config, "warp_mat")){
        json& warp_mat = camera_config["warp_mat"];
        config_matrix(warp_mat, interface->imageView.warp_mat, false);
        interface->warped = true;
    } else {
        interface->warped = false;
    }

    if(exists(camera_config, "adjust_mat")) {
        json &adjust_mat = camera_config["adjust_mat"];
        config_matrix(adjust_mat, interface->imageView.adjust_mat, false);
        interface->imageView.adjust_rdy = true;
    } else {
        interface->imageView.adjust_rdy= false;
    }

    if(exists(camera_config, "offsetL")) interface->offsetL = camera_config["offsetL"];
    if(exists(camera_config, "offsetR")) interface->offsetR = camera_config["offsetR"];

    if(exists(camera_config, "Camera Properties")){
        json& properties_config = camera_config["Camera Properties"];
        for(ControlHolder& ctrl : interface->ctrl_list_default) {
            string name = string((char *) ctrl.qctrl.name);
            if(exists(properties_config, name)){
                int value = properties_config[name];
                interface->vcap.set_control(ctrl.qctrl.id, value);
            }
        }
    }
}

void jsonSaveManager::config_matrix(json& mat_config, int (&mat)[4][2], bool save){
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 2; ++j) {
            string pos_string = "["+std::to_string(i)+"]["+std::to_string(j)+"]";
            if(save) mat_config[pos_string] = mat[i][j];
            else mat[i][j] = mat_config[pos_string];
        }
    }
}

int jsonSaveManager::read_configs_from_file(string file_path) {
    try {
        std::ifstream file(file_path);
        if(file.is_open()) {
            file >> configs;
            file.close();
        }
        else return 1;
    } catch(std::exception& e) {
        string error = e.what();
        if(error.find("parse_error") != string::npos)
            return 2;
        return 3;
    }
    return 0;
}

void jsonSaveManager::write_configs_to_file(string file_path) {
    std::ofstream file(file_path);
    file << std::setw(4) << configs << endl;
    file.close();
    debug_success("Config file written.");
}

bool jsonSaveManager::exists(json &config, string name) {
    return (config.find(name) != config.end());
}