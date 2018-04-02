#ifndef VSSS_JSONSAVEMANAGER_H
#define VSSS_JSONSAVEMANAGER_H

#include "../lib/json.hpp"
#include "../gui/v4linterface.hpp"

using json = nlohmann::json;
using namespace vsss_gui;

class jsonSaveManager {

private:
    /**
     * @brief Global JSON object that keeps data between save/load calls.
     */
    json configs;

    /**
     * @brief Pointer to main interface object. Used to get/set it's values at runtime.
     */
    V4LInterface* interface;

    /**
     * @brief Auxiliar function that checks if a given configuration is already set in our JSON object.
     *
     * @param config Our JSON configuration object.
     * @param name Name of the given configuration.
     * @return bool True if the configuration exists; false otherwise.
     */
    bool exists(json &config, std::string name);

public:

    /**
     * @brief Constructor for JSON Save Manager. "explicit" means that all params types must be satisfied.
     * @param interf Main interface object.
     */
    explicit jsonSaveManager(V4LInterface* interf):interface(interf){};

    /**
     * @brief Loads all avaliable configurations in the given .json file.
     *
     * @param file_path Path to the .json file.
     */
    void load(std::string file_path = "config/quicksave.json");

    /**
     * @brief Saves all volatile data set in the interface to a given .json file.
     *
     * @param file_path Path to the .json file.
     */
    void save(std::string file_path = "config/quicksave.json");

    /**
     * @brief Save robot configurations from interface to our JSON object.
     */
    void save_robots();

    /**
     * @brief Load robot configurations from our JSON object to the interface.
     */
    void load_robots();

    /**
     * @brief Save camera configurations from interface to our JSON object.
     */
    void save_camera();

    /**
     * @brief Load camera configurations from our JSON object to the interface.
     */
    void load_camera();

    /**
     * @brief Configures vision's warp matrix to match it's defaults.
     * @param mat_config Ready-to-save warp matrix.
     * @param mat Ready-to-load warp matrix.
     * @param save The direction of the conversion. "true" converts data from 'mat' to 'mat_config';
     *  false converts data from 'mat_config' to 'mat'.
     */
    void config_matrix(json& mat_config, int (&mat)[4][2], bool save);

    /**
     * @brief Read a given file and place it's data in our JSON object.
     *
     * @param file_path Path to the .json file.
     * @return int 0 case load is successful; 1 case file couldn't be opened; 2 case parse error; 3 case unknown error.
     */
    int read_configs_from_file(std::string file_path = "config/quicksave.json");

    /**
     * @brief Actually writes our JSON object in the given file.
     *
     * @param file_path Path to the .json file.
     */
    void write_configs_to_file(std::string file_path = "config/quicksave.json");
};

#endif //VSSS_JSONSAVEMANAGER_H
