#ifndef ROBOTGUI_HPP_
#define ROBOTGUI_HPP_

#define N_ROBOTS 3

#include <gtkmm.h>
#include <iostream>
#include "../robot.hpp"

class RobotGUI: public Gtk::VBox {
private:

    // globais pois guardam valores entre chamadas diferentes
    double robots_speed_tmp[N_ROBOTS];
    int robots_function_tmp[N_ROBOTS];

public:

    bool robots_speed_edit_flag = false; // desabilita o frame de edição de velocidade
    bool robots_function_edit_flag = false; // desabilita o frame de edição de funções

    std::vector<Robot> robot_list;

    Gtk::Frame robots_function_fm;
    Gtk::Frame robots_speed_fm;

    Gtk::VBox robots_function_vbox; // vbox principal do frame de funções/papéis
    Gtk::VBox robots_speed_vbox; // vbox principal do frame de velocidades
    Gtk::VBox robots_speed_slider_vbox[N_ROBOTS]; // vboxes dos robôs que guardam o slider e a barra de progresso
    Gtk::HBox robots_function_hbox[N_ROBOTS+1]; // hbox que lista os robôs no frame de funções
    Gtk::HBox robots_speed_hbox[N_ROBOTS+1]; // hbox que lista os robôs no frame de velocidades

    // botões de edit/done
    Gtk::Button robots_function_edit_bt;
    Gtk::Button robots_function_done_bt;
    Gtk::Button robots_speed_edit_bt;
    Gtk::Button robots_speed_done_bt;

    Gtk::ComboBoxText cb_robot_function[N_ROBOTS]; // seletor dropdown das funções

    Gtk::HScale robots_speed_hscale[N_ROBOTS]; // sliders de seleção de velocidade dos robos

    Gtk::ProgressBar robots_speed_progressBar[N_ROBOTS]; // barras que mostram a velocidade atual dos robôs

    RobotGUI();
    ~RobotGUI();

    // cria as telas
    void createFunctionsFrame();
    void createSpeedsFrame();
    // atualiza as telas
    void update_robot_functions();
    void update_speed_progressBars();
    // eventos
    void event_robots_function_done_bt_signal_clicked();
    void event_robots_function_edit_bt_signal_clicked();
    void event_robots_speed_edit_bt_signal_pressed();
    void event_robots_speed_done_bt_signal_clicked();
};

#endif /* ROBOTGUI_HPP_ */
