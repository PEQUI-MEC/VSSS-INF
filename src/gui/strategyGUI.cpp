#include "strategyGUI.hpp"

StrategyGUI::StrategyGUI() {
	createFormationFrame();
	createTransitionsFrame();
	pack_start(strategy.testFrame, false, true, 5);
	configureTestFrame();

	for(int i = 0; i < Robots::SIZE; i++) {
		// valores absolutos pois são só pra incializar
		formation_positions[i] = cv::Point(320, 240);
		formation_orientations[i] = cv::Point(200, 480 / 6 + (i + 1) * 480 / 6);
	}
}

StrategyGUI::~StrategyGUI() {}

char * StrategyGUI::unconstant_char(const char * c_str) {
	char * uc;
	int len;
	// tamanho da string
	for(len = 0; c_str[len] != '\0'; len++);
	// aloca nova string
	uc = (char *) malloc(len*sizeof(char));
	// copia dados
	for(int i = 0; i < len; i++)
		uc[i] = c_str[i];

	return uc;
}

void StrategyGUI::_event_loadFormation_bt_clicked() {
    // manda o camcap atualizar a posição real dos robôs
    updating_formation_flag = true;
    debug_log("Positioning robots...");
}

void StrategyGUI::_event_deleteFormation_bt_clicked() {
	updating_formation_flag = false; // para de posicionar caso esteja fazendo isso

	std::ofstream tmpTxtFile;
	std::ifstream txtFile;
	std::string formationName;
	std::string line;

	txtFile.open("config/formation.txt");
	tmpTxtFile.open("config/tmp_formation.txt");

	if(txtFile.is_open() && tmpTxtFile.is_open()) {
		getline(txtFile, line);
		// formações salvas
		int n = atoi(line.c_str());
		// se só tem uma formação salva, apaga o arquivo
		if(n-1 == 0) {
			std::cout << "There was only one formation saved. Formation file deleted.\n";
			txtFile.close();
			tmpTxtFile.close();

			// desabilita os botões
			formation_flag = false;
			bt_createFormation.set_active(false);
			bt_saveFormation.set_sensitive(false);
			formation_name_entry.set_sensitive(false);
			// salva as alterações
			txtFile.close();
			tmpTxtFile.close();
			// remove os arquivos
			system("rm -rf config/formation.txt");
			system("rm -rf config/tmp_formation.txt");
			// atualiza a interface
			loadSavedFormations();
			return;
		}
		// decrementa o número de formações no novo arquivo
		tmpTxtFile << (n-1) << std::endl;
		// para cada prox linha
		for(int i = 0; i < n; i++) {
			// move o cursor enquanto não é a linha que eu quero e copia os dados para o novo arquivo
			getline(txtFile, line);
			// ignora a linha selecionada
			if(i == formation_box.get_active_row_number() - 1)
				continue;

			tmpTxtFile << line << std::endl;
		}

		// desabilita os botões
		formation_flag = false;
		bt_createFormation.set_active(false);
		bt_saveFormation.set_sensitive(false);
		formation_name_entry.set_sensitive(false);
		// salva as alterações
		txtFile.close();
		tmpTxtFile.close();
		// agora as alterações não são mais temporárias
		system("rm -rf config/formation.txt");
		system("mv config/tmp_formation.txt config/formation.txt");
		// atualiza a interface
		loadSavedFormations();
	} else std::cout << "Could not delete formation.\n";
}

void StrategyGUI::_event_saveFormation_bt_clicked() {
	updating_formation_flag = false; // para de posicionar caso esteja fazendo isso

	std::ofstream tmpTxtFile;
	std::ifstream txtFile;
	std::string formationName;
	std::string line;

	if(!formation_flag) {
		std::cout << "Synchronicity error. Flag should be set.\n";
		return;
	}

	txtFile.open("config/formation.txt");
	tmpTxtFile.open("config/tmp_formation.txt");

	// verifica se o nome foi preenchido
	formationName = formation_name_entry.get_text();

	if(formationName == "") {
		std::cout << "Your formation needs to have a name.";
		return;
	}

	// coloca a string no formato certo para ser salva no arquivo
	for(int i = 0; formationName[i] != '\0'; i++) {
		if(!(formationName[i] >= 'a' && formationName[i] <= 'z') && !(formationName[i] >= 'A' && formationName[i] <= 'Z')) {
			formationName[i] = '_';
		}
	}

	if (txtFile.is_open() && tmpTxtFile.is_open()) {
		getline(txtFile, line);
		// formações salvas
		int n = atoi(line.c_str());
		// incrementa o número de formações no novo arquivo
		tmpTxtFile << (n+1) << std::endl;

		// para cada prox linha
		for(int i = 0; i < n; i++) {
			// move o cursor enquanto não é a linha que eu quero e copia os dados para o novo arquivo
			getline(txtFile, line);
			tmpTxtFile << line << std::endl;
		}

		// grava os novos dados no arquivo
		tmpTxtFile << formationName << " ";

		for(int i = 0; i < Robots::SIZE; i++) {
			// não importa deixar o " " no final pois na leitura ele é ignorado
			tmpTxtFile << formation_positions[i].x << " " << formation_positions[i].y << " " << formation_orientations[i].x << " " << formation_orientations[i].y << " ";
		}

		tmpTxtFile << std::endl;

		// desabilita os botões
		formation_flag = false;
		bt_createFormation.set_active(false);
		bt_saveFormation.set_sensitive(false);
		formation_name_entry.set_sensitive(false);
		// salva as alterações
		txtFile.close();
		tmpTxtFile.close();
		// agora as alterações não são mais temporárias
		system("rm -rf config/formation.txt");
		system("mv config/tmp_formation.txt config/formation.txt");
		// atualiza a interface
		loadSavedFormations();

	} else if(tmpTxtFile.is_open()) { // primeira formação salva
		tmpTxtFile << 1 << std::endl;
		// grava os novos dados no arquivo
		tmpTxtFile << formationName << " ";

		for(int i = 0; i < Robots::SIZE; i++) {
			// não importa deixar o " " no final pois na leitura ele é ignorado
			tmpTxtFile << formation_positions[i].x << " " << formation_positions[i].y << " " << formation_orientations[i].x << " " << formation_orientations[i].y << " ";
		}

		tmpTxtFile << std::endl;

		// desabilita os botões
		formation_flag = false;
		bt_createFormation.set_active(false);
		bt_saveFormation.set_sensitive(false);
		formation_name_entry.set_sensitive(false);
		// salva as alterações
		tmpTxtFile.close();
		// agora as alterações não são mais temporárias
		system("mv config/tmp_formation.txt config/formation.txt");
		// atualiza a interface
		loadSavedFormations();
	} else std::cout << "Could not save formation.\n";

}

void StrategyGUI::_event_createFormation_bt_clicked() {
	updating_formation_flag = false; // para de posicionar caso esteja fazendo isso

	formation_flag = !formation_flag;

	bt_createFormation.set_active(!formation_flag);
	bt_saveFormation.set_sensitive(formation_flag);
	formation_name_entry.set_sensitive(formation_flag);
}

void StrategyGUI::_event_formation_box_changed() {
	updating_formation_flag = false; // para de posicionar caso esteja fazendo isso

	if(formation_box.get_active_row_number() == 0) {
		bt_loadFormation.set_sensitive(false);
		bt_deleteFormation.set_sensitive(false);
		return;
	}
	bt_loadFormation.set_sensitive(true);
	bt_deleteFormation.set_sensitive(true);
	// carrega os dados dessa formação
    load_formation();
	// atualiza interface
	update_interface_flag = true;
}

void StrategyGUI::_event_transitions_checkbox_signal_clicked() {
	strategy.transitions_enabled = !strategy.transitions_enabled;
	debug_log("Transitions: " + to_string(strategy.transitions_enabled));
}

void StrategyGUI::load_formation() {
	updating_formation_flag = false; // para de posicionar caso esteja fazendo isso

    std::ifstream txtFile;
    std::string line;

    if(formation_box.get_active_row_number() == 0) {
        bt_loadFormation.set_sensitive(false);
        bt_deleteFormation.set_sensitive(false);
        return;
    }

    txtFile.open("config/formation.txt");

    if(txtFile.is_open()) {
        getline(txtFile, line);
        // formações salvas
        int n = atoi(line.c_str());
        // para cada prox linha
        for(int i = 0; i < n; i++) {
            // move o cursor enquanto não é a linha que eu quero
            getline(txtFile, line);
            if(i == formation_box.get_active_row_number() - 1) {
                std::vector<std::string> strs;
                boost::split(strs, line, boost::is_any_of(" "));

                // as posições estão armazenadas uma na frente da outra. usa 'position' pra navegar sobre elas
                unsigned short position = 1;
                for(int robot = 0; robot < Robots::SIZE; robot++) {
                    formation_positions[robot].x = atoi(strs.at(position++).c_str());
                    formation_positions[robot].y = atoi(strs.at(position++).c_str());
                    formation_orientations[robot].x = atoi(strs.at(position++).c_str());
					formation_orientations[robot].y = atoi(strs.at(position++).c_str());
                }
            }
        }
        // atualiza interface
        update_interface_flag = true;
        debug_log("Formation " + formation_box.get_active_text() + " loaded.");
        // valores disponíveis: formation_positions e formation_orientations
        // isso é atualizado no camcap

        txtFile.close();
    } else {
        // não existem formações salvas
        std::cout << "You removed the file before I could read it.\n";
    }
}

void StrategyGUI::loadSavedFormations() {
	updating_formation_flag = false; // para de posicionar caso esteja fazendo isso

	std::ifstream txtFile;
	std::string line;

	txtFile.open("config/formation.txt");

	formation_box.remove_all();
	formation_box.append("None");

	if(txtFile.is_open()) {
		getline(txtFile, line);
		// formações salvas
		int n = atoi(line.c_str());

		// para cada prox linha
		for(int i = 0; i < n; i++) {
			getline(txtFile, line);
			// pega o nome da formação
			formation_box.append(strtok(unconstant_char(line.c_str()), " "));
		}

		txtFile.close();
	} else {
		// não existem formações salvas
		// std::cout << "There's no saved robot formations.\n";
	}
	// seleciona o None como padrão
	formation_box.set_active(0);
}

void StrategyGUI::createTransitionsFrame() {
	pack_start(transitions_fm, false, true, 5);

	transitions_fm.set_label("Transitions");
	transitions_fm.add(transitions_grid);

	transitions_grid.set_border_width(10);
	transitions_grid.set_column_spacing(5);
	transitions_grid.set_halign(Gtk::ALIGN_CENTER);

	transitions_grid.attach(transitions_check, 0, 0, 1, 1);

	transitions_check.set_label("Enable Transitions");
	transitions_check.set_active();

	transitions_check.signal_clicked().connect(sigc::mem_fun(*this, &StrategyGUI::_event_transitions_checkbox_signal_clicked));
}

void StrategyGUI::configureTestFrame() {
	std::string labels[5] = {"Goalie Line", "Goalie Offset", "Name 3", "Name 4", "Name 5"};
	double min[5] = {0, 0, 0, 0, 0};
	double max[5] = {160, 100, 100, 100, 100};
	double currentValue[5] = {60, 15, 20, 30, 40};
	double digits[5] = {0, 0, 0, 0, 0};
	double steps[5] = {5, 1, 1, 1, 1};

	for (int i = 0; i < 5; i++) {
		strategy.testFrame.setLabel(i, labels[i]);
		strategy.testFrame.configureHScale(i, currentValue[i], min[i], max[i], digits[i], steps[i]);
	}
}

void StrategyGUI::createFormationFrame() {
	pack_start(formation_fm, false, true, 5);
	formation_fm.add(formation_grid);
	formation_fm.set_label("Formation");
	formation_grid.set_halign(Gtk::ALIGN_CENTER);

	bt_createFormation.set_label("Create Formation");
	loadSavedFormations();
	bt_loadFormation.set_label("Load");
	bt_deleteFormation.set_label("Delete");
	formation_name_entry.set_max_length(30);
	formation_name_entry.set_width_chars(30);
	formation_name_entry.set_text(Glib::ustring::format("Formation_name"));
	bt_saveFormation.set_label("Save");

	formation_grid.set_border_width(10);
	formation_grid.set_column_spacing(5);

	formation_grid.attach(bt_createFormation, 0, 0, 1, 1);
	formation_grid.attach(formation_box, 0, 1, 1, 1);
	formation_grid.attach(bt_loadFormation, 1, 1, 1, 1);
	formation_grid.attach(bt_deleteFormation, 2, 1, 1, 1);
	formation_grid.attach(formation_name_entry, 0, 2, 1, 1);
	formation_grid.attach(bt_saveFormation, 1, 2, 1, 1);

	formation_box.set_sensitive(false); // habilita quando der start
	bt_createFormation.set_sensitive(false); // habilita quando der start

	bt_loadFormation.set_sensitive(false); // habilita só quando selecionar uma formação
	bt_deleteFormation.set_sensitive(false); // habilita só quando selecionar uma formação
	bt_saveFormation.set_sensitive(false); // habilita só quando o createFormation está ativado
	formation_name_entry.set_sensitive(false); // habilita só quando o createFormation está ativado

	formation_box.signal_changed().connect(sigc::mem_fun(*this, &StrategyGUI::_event_formation_box_changed));
	bt_createFormation.signal_pressed().connect(sigc::mem_fun(*this, &StrategyGUI::_event_createFormation_bt_clicked));
	bt_saveFormation.signal_clicked().connect(sigc::mem_fun(*this, &StrategyGUI::_event_saveFormation_bt_clicked));
	bt_loadFormation.signal_clicked().connect(sigc::mem_fun(*this, &StrategyGUI::_event_loadFormation_bt_clicked));
	bt_deleteFormation.signal_clicked().connect(sigc::mem_fun(*this, &StrategyGUI::_event_deleteFormation_bt_clicked));
}
