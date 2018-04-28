/*
 * main.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: gustavo
 */

// Must be included before #ifndef directive
// This makes our compiler include our debug namespace directives into this file
#include "aux/debug.hpp"

#include <gtkmm.h>
#include "camcap.hpp"
#include <iostream>

void usage() {
	cout << "\nValid [optional] options: " << endl;
	cout << "\t\033[1;37m-v \033[0m[verbosity level: \033[1;37m0\033[0m = log, \033[1;37m1\033[0m = success, \033[1;37m2\033[0m = warning, \033[1;37m3\033[0m = error, \033[1;37m4\033[0m = fatal error, \033[1;37m5\033[0m = no messages]" << endl;
	cout << "\t\033[1;37m--verbosity \033[0m[same as -v]" << endl;
	cout << "\t\033[1;37m-o \033[0m[debug output file - default is \"debug.log\"]" << endl;
	cout << "\t\033[1;37m--output \033[0m[same as -o]" << endl << endl;
}

int main(int argc, char ** argv) {
	Gtk::Main kit(argc, argv);
	Gtk::Window window;

	for(int i = 1; i < argc; i++) {
		if(string(argv[i]) == "-v" || string(argv[i]) == "--verbosity") {
			if(i + 1 < argc) {
				try {
					int verb = stoi(argv[i+1]);
					debug_set_level(verb);
					i++;
				}
				catch(...) {
					debug_error("This is not a number.");
					usage();
					break;
				}
			} else {
				debug_error("Missing verbosity level.");
				usage();
				break;
			}
		} else if(string(argv[i]) == "-o" || string(argv[i]) == "--output") {
			if(i + 1 < argc) {
				if(debug_set_output(argv[i+1])) {
					debug_success("Debug output file set.");
					i++;
				} else {
					debug_error("Debug output file could not be opened.");
					i++;
				}
			} else {
				debug_error("Debug output file not informed.");
				usage();
				break;
			}
		} else {
			debug_error("Invalid options.");
			usage();
			break;
		}
	}

	debug_set_level(0);
	if(debug_set_output("debug.log")) {
		debug_log("Saving debug information to \"./debug.log\"");
	}

	CamCap camcap;

	window.set_position(Gtk::WIN_POS_CENTER);
	//window.set_border_width(10);
	window.maximize();
	window.set_title("VSSS INF - P137");

	window.add(camcap);

	window.show_all();

	Gtk::Main::run(window);

	return EXIT_SUCCESS;
}