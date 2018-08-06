#include "dialog.hpp"

// !TODO setar a transient window do dialog
void Dialog::pop_alert(std::string title, std::string message) {
    Gtk::MessageDialog dialog(title, // titulo
                              false, // use_markup
                              Gtk::MESSAGE_WARNING, // MessageType
                              Gtk::BUTTONS_OK, // ButtonsType
                              false); // modal

    dialog.set_secondary_text(message);

    dialog.run();
}

void Dialog::pop_critical(std::string title, std::string message) {
    Gtk::MessageDialog dialog(title, // titulo
                              false, // use_markup
                              Gtk::MESSAGE_ERROR, // MessageType
                              Gtk::BUTTONS_OK, // ButtonsType
                              false); // modal

    dialog.set_secondary_text(message);

    dialog.run();
}