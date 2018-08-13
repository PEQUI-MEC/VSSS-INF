#ifndef DIALOG_HPP_
#define DIALOG_HPP_

#include <string>
#include <gtkmm.h>

class Dialog {
public:
    /**
+	 * Pops up an alert dialog.
+	 * @param title Dialog's title
+	 * @param message Dialog's message
+	 */
    static void pop_alert(std::string title, std::string message);

    /**
+	 * Pops up a critic dialog.
+	 * @param title Dialog's title
+	 * @param message Dialog's message
+	 */
    static void pop_critical(std::string title, std::string message);
};

#endif