/**
 * @file strategyGUI.hpp 
 * @since 2016/05/08
 * @author Daniel
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
	 * @brief Converts a constant string to an inconstant one
	 * @param c_str Constant string to be converted
	 * @return char* Not-const converted string 
	 */
	char * unconstant_char(const char * c_str);
	
	/**
	 * @brief Event triggered when a formation is selected and 'load' button is clicked
	 * @details Loads the saved formations file and changes robot's position flags
	 * The formation will be updated on camcap
	 */	
	void _event_loadFormation_bt_clicked();
	
	/**
	 * @brief Event triggered when the delete button is clicked
	 * @details Deletes the selected formation from the formations file (config/formation.txt)
	 */
	void _event_deleteFormation_bt_clicked();
	
	/**
	 * @brief Event triggered when the save button is clicked
	 * @details Adds the formation in the formations file (config/formation.txt)
	 */
	void _event_saveFormation_bt_clicked();
	
	/**
	 * @brief Event triggered when the 'create formation' button is clicked
	 * @details Makes the text field and 'save' button available
	 */
	void _event_createFormation_bt_clicked();
	
	/**
	 * @brief Event triggered when a formation (or none) is selected 
	 * @details Checks if a formation is selected and, case positive, makes the 'load' and 'save' buttons available
	 */
	void _event_formation_box_changed();
	
	/**
	 * @brief Event triggered when the 'penalty' button is clicked
	 * @details Not implemented yet
	 */
	void _event_set_penalty_behavior();
	
 	/**
	 * @brief Switches Strategy's full_transition_enabled flag value 
	 */
	void _event_fulltransition_checkbox_signal_clicked();

	/**
	 * @brief Switches Strategy's half_transition_enabled flag value 
	 */
	void _event_halftransition_signal_clicked();
	
	/**
	 * @brief Switches Strategy's transition_mindcontrol_enabled flag value
	 */
	void _event_transitionmindcontrol_signal_clicked();
	
	/**
	 * @brief Loads formation.txt file and appends the saved formations on the checkbox
	 * @details File structure:
	 * Number of formations
	 * formation_name r1x r1y r1o r2x r2y r2o r3x r3y r3o
	 */
	void loadSavedFormations();
	
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
	cv::Point formation_positions[3]; /**< Updated only if the flag is activated*/
	float formation_orientations[3]; /**< Updated only if the flag is activated*/

	Gtk::Frame behavior_fm;
	Gtk::Grid behavior_grid;
	Gtk::ToggleButton bt_penaltyBehavior;
	Gtk::Frame transitions_fm;
	Gtk::Grid transitions_grid;
	Gtk::CheckButton transitions_fulltransition_check;
	Gtk::CheckButton transitions_halftransition_check;
	Gtk::CheckButton transitions_mindcontrol_check;

	/**
	 * @brief Constructor of a StrategyGUI object
	 */
	StrategyGUI();

	/**
	 * @brief Inicialization of labels and values from Test Parameters frame 
	 */
	void configureTestFrame();
	
	/**
	 * @brief Formation frame creator
	 */
	void createFormationFrame();

	/**
	 * @brief Transitions frame creator
	 */
	void createTransitionsFrame();
	
	/**
	 * @brief Destructor of a StrategyGUI object
	 */	
	~StrategyGUI();
};

#endif /* STRATEGYGUI_HPP_ */
