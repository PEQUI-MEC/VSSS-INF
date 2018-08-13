/**
 * @file strategyGUI.hpp
 * @since 2016/05/08
 * @author Daniel @ Pequi Mecanico | Bryan Lincoln @ Pequi Mecânico
 * @brief Strategy's GUI class
 * @see https://www.facebook.com/NucleoPMec/
 * @sa https://www.instagram.com/pequimecanico
 */

#ifndef STRATEGYGUI_HPP_
#define STRATEGYGUI_HPP_

#include <gtkmm.h>
#include <boost/algorithm/string.hpp>
#include "../strategy.hpp"
#include "filechooser.hpp"

class StrategyGUI: public Gtk::VBox {
private:

	/**
	 * Converts a constant string to an inconstant one
	 * @param c_str Constant string to be converted
	 * @return char* Non-const converted string
	 */
	char * unconstant_char(const char * c_str);

	/**
	 * Event triggered when a formation is selected and 'load' button is clicked
	 * Sets updating_formation_flag so Manager can send the new positions to the robots.
	 */
	void _event_loadFormation_bt_clicked();

	/**
	 * Event triggered when the delete button is clicked
	 * Deletes the selected formation from the formations file (config/formation.txt)
	 */
	void _event_deleteFormation_bt_clicked();

	/**
	 * Event triggered when the save button is clicked
	 * Adds the formation in the formations file (config/formation.txt)
	 */
	void _event_saveFormation_bt_clicked();

	/**
	 * Event triggered when the 'create formation' button is clicked
	 * Makes the text field and 'save' button available
	 */
	void _event_createFormation_bt_clicked();

	/**
	 * Event triggered when a formation (or none) is selected
	 * Checks if a formation is selected and, case positive, makes the 'load' and 'save' buttons available
	 */
	void _event_formation_box_changed();

	/**
	 * Event triggered when the 'penalty' button is clicked
	 * Not implemented yet
	 */
	void _event_set_penalty_behavior();

 	/**
	 * Switches Strategy's full_transition_enabled flag value
	 */
	void _event_transitions_checkbox_signal_clicked();

	/**
	 * Loads formation.txt file and appends the saved formations on the checkbox
	 * File structure:
	 * Number of formations
	 * formation_name r1x r1y r1o r2x r2y r2o r3x r3y r3o
	 */
	void loadSavedFormations();

	/**
	 * Load formation information from file to virtual robot's position arrays.
	 */
	void load_formation();

public:
	/**Tree model columns: */
	class ModelColumns : public Gtk::TreeModel::ColumnRecord
	{
		public:

		ModelColumns() {
			add(strategy_name);
		}

		Gtk::TreeModelColumn<Glib::ustring> strategy_name;
	};

	ModelColumns strategyColumns;

	Gtk::Frame menu_fm;
	Gtk::Frame info_text_fm;
	Gtk::Frame info_img_fm;
	Gtk::TreeView m_TreeView;
	Glib::RefPtr<Gtk::ListStore> m_refTreeModel;

	Gtk::VBox menu_vbox;
	Gtk::HBox menu_hbox[2];

	Gtk::Image strategy_img;

	Gtk::Button select_button;

	Gtk::ScrolledWindow m_ScrolledWindow;
	Gtk::TextView strategy_description_view;
	Glib::RefPtr<Gtk::TextBuffer> strategy_description_text;

	Strategy strategy;

	bool updating_formation_flag = false;

	Gtk::Frame formation_fm;
	bool formation_flag = false;
	bool update_interface_flag = false;
	Gtk::Grid formation_grid;
	Gtk::Button bt_loadFormation;
	Gtk::Button bt_deleteFormation;
	Gtk::ToggleButton bt_createFormation;
	Gtk::Button bt_saveFormation;
	Gtk::ComboBoxText formation_box;
	Gtk::Entry formation_name_entry;
	cv::Point formation_positions[Robots::SIZE]; /**< Updated only if the flag is activated*/
	cv::Point formation_orientations[Robots::SIZE]; /**< Updated only if the flag is activated*/

	Gtk::Frame behavior_fm;
	Gtk::Grid behavior_grid;
	Gtk::ToggleButton bt_penaltyBehavior;
	Gtk::Frame transitions_fm;
	Gtk::Grid transitions_grid;
	Gtk::CheckButton transitions_check;

	/**
	 * @brief Constructor of a StrategyGUI object
	 */
	StrategyGUI();

	/**
	 * Inicialization of labels and values from Test Parameters frame
	 */
	void configureTestFrame();

	/**
	 * Formation frame creator
	 */
	void createFormationFrame();

	/**
	 * Transitions frame creator
	 */
	void createTransitionsFrame();

	/**
	 * Destructor of a StrategyGUI object
	 */
	~StrategyGUI();
};

#endif /* STRATEGYGUI_HPP_ */
