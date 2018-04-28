#define debug_set_level(...) Debug::_set_debug_level( __VA_ARGS__ )
#define debug_set_output(...) Debug::_set_output_file( __VA_ARGS__ )
#define debug_log(...) Debug::log(__FILE__, __LINE__, __FUNCTION__, 0, __VA_ARGS__)
#define debug_success(...) Debug::log(__FILE__, __LINE__, __FUNCTION__, 1, __VA_ARGS__)
#define debug_warning(...) Debug::log(__FILE__, __LINE__, __FUNCTION__, 2, __VA_ARGS__)
#define debug_error(...) Debug::log(__FILE__, __LINE__, __FUNCTION__, 3, __VA_ARGS__)
#define debug_fatal(...) Debug::log(__FILE__, __LINE__, __FUNCTION__, 4, __VA_ARGS__)

#ifndef DEBUG_HPP_
#define DEBUG_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>

namespace filesys = boost::filesystem;
using namespace std;

class Formatter {
public:
    template<class Val> Formatter& operator<<(const Val& val) {
        ss_ << val;
        return * this;
    }
    operator string () const { return ss_.str().c_str(); }
private:
    std::stringstream ss_;
};

namespace Debug {
    static int _debug_level;
    static bool _debug_level_set;

    static ofstream _debug_output;
    static bool _write_enabled;

    /**
     * @brief Sets the verbosity of this debug script.
     * @param level 0 = log, 1 = success, 2 = warning, 3 = error, 4 = fatal error, 5 = no messages
     */
    inline void _set_debug_level(int level) {
        if(_debug_level_set) return;

        stringstream color;

        _debug_level = level;

        _debug_level_set = true;

        if(level > 4) return;

        cout << "\033[0;37mDebug will show only ";
        switch(level) {
            case 0: // log
                color << "log, ";
            case 1: // success
                color << "success, ";
            case 2: // warning
                color << "warning, ";
            case 3: // error
                color << "error and ";
            case 4: // fatal
                color << "fatal error ";
        }
        cout << color.str() << "messages.\033[0m" << endl << endl;
    }

    /**
     * @brief Tries to open a given file name as our debug output
     * @param file_name Name of the output file to be opened
     * @return bool True on success, False on error.
     */
    inline bool _set_output_file(string file_name) {
        if(_write_enabled) return false;

        _debug_output.open(file_name);
        if(!_debug_output.is_open()) {
            _write_enabled = false;
            return false;
        }
        _write_enabled = true;
        return true;
    }

    /**
     * @brief Get File Name from a Path with or without extension
     * @param file Full path to the given file
     * @return string The file's name without extension.
     */
    inline string _get_class(string file) {
        // Create a Path object from File Path
        filesys::path pathObj(file);

        if(pathObj.has_stem()) {
            // return the stem (file name without extension) from path object
            return pathObj.stem().string();
        }
        // return the file name with extension from path object
        return pathObj.filename().string();
    }

    /**
     * @brief Logs a string in a fancy way
     * @param file_caller The name of the file where this function was called
     * @param line_caller The line of the file where this function was called
     * @param function_caller The name of the function where this one was called
     * @param level The priority level (used for verbosity) of the received message
     * @param msg The message to be printed
     */
    inline void log(const char * file_caller, int line_caller, const char * function_caller, short level, string msg) {
        if(level < _debug_level) return;

        string class_caller = _get_class(file_caller);
        string color;
        stringstream output_message;

        switch(level) {
            case 0: // log
                color = "\033[0;37m(log) \t\033[1;37m";
            break;
            case 1: // success
                color = "\033[0;32m(success) \t\033[1;32m";
            break;
            case 2: // warning
                color = "\033[0;33m(warning) \t\033[1;33m";
            break;
            case 3: // error
                color = "\033[0;31m(error) \t\033[1;31m";
            break;
            case 4: // fatal
                color = "\033[1;31m(fatal) \t\033[41m\033[1;37m";
            break;
        }

        if(_debug_level == 0)
            output_message << "\033[0m" << "[" << class_caller << " -> " << function_caller << " (" << line_caller << ")]: ";

        if(string(msg).size() > 0)
            output_message << color << msg;

        output_message << "\033[0m" << endl;

        cout << output_message.str();

        if(_write_enabled) {
            _debug_output << output_message.str();
        }
    }

}

#endif