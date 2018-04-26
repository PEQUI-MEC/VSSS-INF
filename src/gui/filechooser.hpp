/** 
* @file filechooser.hpp
*/

#ifndef FILECHOOSER_HPP_
#define FILECHOOSER_HPP_

#include <gtkmm.h>

class FileChooser : public Gtk::Window {

public:
	
	/** 
	* @brief Creates the dialog box so the user can select a local file (after clicking on 'Load')
	*/
	FileChooser();
	/** 
	* @brief Destructor of the class
	*/
	virtual					~FileChooser();
	std::string fileName;
	int result;

protected:

	//Member widgets:
	Gtk::Button             buttonLoad;
	Gtk::Button             buttonQuit;
	Gtk::Grid               mainGrid;
};

#endif /* FILECHOOSER_HPP_ */
