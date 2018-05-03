#include "robotGUI.hpp"

RobotGUI::RobotGUI() {
    createFunctionsFrame();
    createSpeedsFrame();
}

RobotGUI::~RobotGUI() {}

//////// AUXILIARES

void RobotGUI::update_robot_functions() {
    for (int i = 0; i < N_ROBOTS; i++)
        cb_robot_function[i].set_active(robot_list[i].role);
}

void RobotGUI::update_speed_progressBars() {
    std::ostringstream strs[N_ROBOTS];

    for(int i = 0; i < N_ROBOTS; i++) {
        robots_speed_progressBar[i].set_fraction( robot_list[i].vmax/1.4);
        strs[i] << robot_list[i].vmax;
        robots_speed_progressBar[i].set_text(strs[i].str().substr(0,4));
    }
}

/////// INTERFACES

void RobotGUI::createFunctionsFrame() {
    Gtk::Label *label;
    std::string function[N_ROBOTS];

    pack_start(robots_function_fm, false, true, 5); // coloca o frame no começo da aba

    robots_function_fm.add(robots_function_vbox); // adiciona um vbox

    // coloca os botões na primeira hbox
    robots_function_hbox[0].pack_start(robots_function_edit_bt, false, true, 5); // alinha pro começo
    robots_function_hbox[0].pack_end(robots_function_done_bt, false, true, 5); // alinha pro final
    // coloca essa hbox no começo da vbox
    robots_function_vbox.pack_start(robots_function_hbox[0], false, true, 5);

    // seta os textos das possíveis funções
    for(int i = 0; i < N_ROBOTS; i++)
        function[i].clear();

    function[0].append("Goalkeeper");
    function[1].append("Defense");
    function[2].append("Attack");

    // pra cada robô representado
    for(int i = 0; i < N_ROBOTS; i++) {
        // configura o rótulo
        switch(i) {
            case 1:
                label = new Gtk::Label("Robot 2:");
                break;

            case 2:
                label = new Gtk::Label("Robot 3:");
                break;

            default:
                label = new Gtk::Label("Robot 1:");
        }

        // adiciona as opções de papel pro seletor de cada robo
        for(int j = 0; j < N_ROBOTS; j++)
            cb_robot_function[i].append(function[j]);

        // seleciona uma opção padrão
        cb_robot_function[i].set_active_text(function[i]);

        // coloca o rótulo e o seletor na hbox que tá dentro da vbox
        robots_function_hbox[i+1].pack_start(*label, false, true, 5);
        robots_function_hbox[i+1].pack_start(cb_robot_function[i], false, true, 5);

        // coloca essa hbox dentro da vbox
        robots_function_vbox.pack_start(robots_function_hbox[i+1], false, true, 5);
    }

    // configuração inicial da interface

    // coloca as labels
    robots_function_fm.set_label("Robot Functions"); // nome que fica flutuando no frame
    robots_function_edit_bt.set_label("Edit");
    robots_function_done_bt.set_label("Done");

    // deixa os botões não clicáveis
    robots_function_done_bt.set_state(Gtk::STATE_INSENSITIVE);
    robots_function_edit_bt.set_state(Gtk::STATE_INSENSITIVE);

    for(int i = 0; i < N_ROBOTS; i++)
        cb_robot_function[i].set_state(Gtk::STATE_INSENSITIVE);

    // amarra eventos

    robots_function_edit_bt.signal_clicked().connect(sigc::mem_fun(*this, &RobotGUI::event_robots_function_edit_bt_signal_clicked));
    robots_function_done_bt.signal_clicked().connect(sigc::mem_fun(*this, &RobotGUI::event_robots_function_done_bt_signal_clicked));
}

void RobotGUI::createSpeedsFrame() {
    // coloca o frame das velocidades na aba dos robos
    pack_start(robots_speed_fm, false, true, 5);

    robots_speed_fm.add(robots_speed_vbox); // adiciona uma vbox

    // adiciona os botões de edit numa hbox
    robots_speed_hbox.pack_start(robots_speed_edit_bt, false, true, 5);
    robots_speed_hbox.pack_end(robots_speed_done_bt, false, true, 5);

    // coloca essa hbox na vbox principal
    robots_speed_vbox.pack_start(robots_speed_hbox, false, true, 5);

    // coloca a grid das velocidades nessa vbox também
    robots_speed_vbox.pack_start(speeds_grid, false, true, 0);

    // configura a grade
    speeds_grid.set_border_width(10);
    speeds_grid.set_column_spacing(5);
    speeds_grid.set_halign(Gtk::ALIGN_CENTER);

    // pra cada robô
    for(int i = 0; i < N_ROBOTS; i++) {
        Gtk::Label * label; // ponteiro pq não deve ser destruído quando essa função acabar
        // seleciona a label correspondente
        switch(i) {
            case 1:
                label = new Gtk::Label("DEF");
                break;

            case 2:
                label = new Gtk::Label("ATK");
                break;

            default:
                label = new Gtk::Label("GK");
        }
        // configura os sliders
        robots_speed_hscale[i].set_digits(1);
        robots_speed_hscale[i].set_increments(0.1,1);
        robots_speed_hscale[i].set_range(0,1.4);
        robots_speed_hscale[i].set_size_request(100,-1);
        robots_speed_hscale[i].set_value(0.8);
        robots_speed_hscale[i].set_state(Gtk::STATE_INSENSITIVE);

        // configura as barras de progresso
        robots_speed_progressBar[i].set_halign(Gtk::ALIGN_CENTER);
        robots_speed_progressBar[i].set_valign(Gtk::ALIGN_CENTER);
        robots_speed_progressBar[i].set_text("0.8");
        robots_speed_progressBar[i].set_show_text(true);
        robots_speed_progressBar[i].set_fraction(0.8);

        // coloca o slider e a barra de progresso que representa a velocidade atual em um vbox exclusivo do robô
        robots_speed_slider_vbox[i].pack_start(robots_speed_hscale[i], false, true, 0);
        robots_speed_slider_vbox[i].pack_start(robots_speed_progressBar[i], false, true, 0);

        // adiciona o rótulo e o slider na hbox do robô
        speeds_grid.attach(*label, i, 0, 1, 1);
        speeds_grid.attach(robots_speed_slider_vbox[i], i, 1, 1, 1);
    }

    // configuração inicial

    robots_speed_fm.set_label("Speeds"); // nome do frame

    robots_speed_edit_bt.set_label("Edit"); // botões de edit
    robots_speed_done_bt.set_label("Done"); // e done

    robots_speed_done_bt.set_state(Gtk::STATE_INSENSITIVE); // deixa eles não clicáveis
    robots_speed_edit_bt.set_state(Gtk::STATE_INSENSITIVE);

    // eventos

    robots_speed_edit_bt.signal_pressed().connect(sigc::mem_fun(*this, &RobotGUI::event_robots_speed_edit_bt_signal_pressed));
    robots_speed_done_bt.signal_clicked().connect(sigc::mem_fun(*this, &RobotGUI::event_robots_speed_done_bt_signal_clicked));
}

//////// EVENTOS

void RobotGUI::event_robots_function_edit_bt_signal_clicked() {
	if(!robots_function_edit_flag) {
		robots_function_edit_flag = true;
		robots_function_edit_bt.set_label("Cancel");

        // atualiza interface
        for(int i = 0; i < N_ROBOTS; i++) {
            cb_robot_function[i].set_state(Gtk::STATE_NORMAL);
            robots_function_tmp[i] = cb_robot_function[0].get_active_row_number();
        }

		robots_function_edit_bt.set_state(Gtk::STATE_NORMAL);
        robots_function_done_bt.set_state(Gtk::STATE_NORMAL);
	} else {
		robots_function_edit_flag = false;
		robots_function_edit_bt.set_label("Edit");

        robots_function_done_bt.set_state(Gtk::STATE_INSENSITIVE);

        for(int i = 0; i < N_ROBOTS; i++) {
		    cb_robot_function[i].set_state(Gtk::STATE_INSENSITIVE);
            cb_robot_function[i].set_active(robots_function_tmp[i]);
        }
	}
}

void RobotGUI::event_robots_function_done_bt_signal_clicked() {
    std::string s[N_ROBOTS];

    for(int i = 0; i < N_ROBOTS; i++) {
        s[i] = cb_robot_function[i].get_active_text();

        if (s[i].compare("Goalkeeper") == 0) {
            std::cout << "Robot " << i+1 << ": Goalkeeper." << std::endl;
            robot_list[i].role = 0;
        } else if (s[i].compare("Defense") == 0) {
            std::cout << "Robot " << i+1 << ": Defense." << std::endl;
            robot_list[i].role = 1;
        } else if (s[i].compare("Attack") == 0) {
            std::cout << "Robot " << i+1 << ": Attack." << std::endl;
            robot_list[i].role = 2;
        } else {
            std::cout << "Error: not possible to set robot " << i+1 << " function." << std::endl;
        }

        cb_robot_function[i].set_state(Gtk::STATE_INSENSITIVE);
    }

    robots_function_edit_flag = false;
    robots_function_edit_bt.set_label("Edit");
    robots_function_done_bt.set_state(Gtk::STATE_INSENSITIVE);
}

void RobotGUI::event_robots_speed_edit_bt_signal_pressed() {
	if(!robots_speed_edit_flag) {
		robots_speed_edit_flag = true;
		robots_speed_edit_bt.set_label("Cancel");
		robots_speed_done_bt.set_state(Gtk::STATE_NORMAL);

        for(int i = 0; i < N_ROBOTS; i++) {
            robots_speed_hscale[i].set_state(Gtk::STATE_NORMAL);
            robots_speed_tmp[i] = robots_speed_hscale[i].get_value();
        }
	} else {
		robots_speed_edit_flag = false;
		robots_speed_edit_bt.set_label("Edit");
		robots_speed_done_bt.set_state(Gtk::STATE_INSENSITIVE);

        for(int i = 0; i < N_ROBOTS; i++) {
            robots_speed_hscale[i].set_state(Gtk::STATE_INSENSITIVE);
            robots_speed_hscale[i].set_value(robots_speed_tmp[i]);
        }
	}
}

void RobotGUI::event_robots_speed_done_bt_signal_clicked(){
    robots_speed_edit_flag = false;
	robots_speed_edit_bt.set_label("Edit");
	robots_speed_done_bt.set_state(Gtk::STATE_INSENSITIVE);

    for(int i = 0; i < N_ROBOTS; i++) {
        default_vel[i] = (float) robots_speed_hscale[i].get_value();
        robots_speed_hscale[i].set_state(Gtk::STATE_INSENSITIVE);
    }
}
