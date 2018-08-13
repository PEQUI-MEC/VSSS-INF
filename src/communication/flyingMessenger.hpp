/**
 * @file flyingMessenger.hpp
 * @author Pequi Mecânico
 * @date undefined
 * @brief Communication methods with software and xbees.
 * @see https://www.facebook.com/NucleoPMec/
 * @sa https://www.instagram.com/pequimecanico
 */

#ifndef VSSS_FlyingMessenger_H
#define VSSS_FlyingMessenger_H

#include "robots.hpp"
#include "serialCom.hpp"
#include <string>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <condition_variable>

#define DEFAULT_FRAMESKIP 4

template <int size>
struct msg_data {
	std::array<float, size> data;
	bool is_valid;

	inline float& operator[](int i) {
		return data[i];
	}
};

class FlyingMessenger {
private:
	/**
	 * @brief Creates a pointer to SerialCom class.
	 * @details SerialCom class defines interaction methods with xbee radios like initial configuration,
	 *  send and get message. More details can be found on serialCom.hpp.
	 */
	SerialCom* xbee = nullptr;
	
	std::string final_msg;
	float orientation_Kp = 0.8;
	double time_between_msgs;
	int frameskip, send_cmd_count;
	std::chrono::system_clock::time_point previous_msg_time;

	/**
	 * @brief Calls SerialCom's addNew method and add 6 possible connections to the robots based on their
	 *  ID and address.
	 */
	void add_robots();

	/**
	 * @brief Prepare the message to send to the robot a position message.
	 * @details The message will send x position, y position and the velocity needed to make tragetory.
	 *
	 * @param robot Gets a Robot instance to prepare the specific message.
	 * @return std::string Message containing prepared position and velocity message.
	 */
	std::string position_msg(Robots::Command command);

	/**
	 * @brief Prepare the message to send to the robot a speed message.
	 * @details The message will send left and right wheel speed to the robot.
	 *
	 * @param robot Gets a Robot instance to prepare the specific message.
	 * @return std::string Message containing prepared speed message.
	 */
	std::string speed_msg(Robots::Command command);

	/**
	 * @brief 
	 * 
	 */
	std::string velocity_msg(float right_wheel, float left_wheel);

	/**
	 * @brief Prepare the message to send to the robot a orientation message.
	 * @details The message will send orientation value and the max velocity of the tragetory.
	 *
	 * @param robot Gets a Robot instance to prepare the specific message.
	 * @return std::string Message containg prepared orientation message.
	 */
	std::string orientation_msg(Robots::Command command);

	/**
	 * @brief Prepare the message to send to the robot a vector message.
	 * @details The message will send orientation value and the max velocity of the tragetory.
	 *
	 * @param robot Gets a Robot instance to prepare the specific message.
	 * @return std::string Message containg prepared orientation message.
	 */
	std::string vector_msg(Robots::Command command);

	/**
	 * @brief Method to round the double number passed.
	 *
	 * @param num Double number passed to be rounded.
	 * @return std::string Returns a string containing the rounded double number passed.
	 */
	std::string rounded_str(double num);

public:

	/**
	 * @brief Constructor of a new FlyingMessenger object
	 */
	FlyingMessenger();

	/**
	 * @brief Iterates between all robots instances and send specific messages to each one.
	 *
	 * @param robots Vector containing all robots instances.
	 * @return std::vector<message> Returns a vector of all acks received after send commands to each robots.
	 */
	std::vector<message> sendCMDs(std::vector<Robots::Command> commands);

	/**
	 * @brief This method calls sendMessage method from xbee (instance of SerialCom class) and pass id and msg.
	 *
	 * @param id Identification of the robot.
	 * @param msg Message that will be sent to robot.
	 */
	void send_msg(char id, std::string msg);

	/**
	 * @brief Send command using old formatation of the string.
	 *
	 * @param cmd String containing the message that will be sent to the robot.
	 */
	void send_old_format(std::string cmd);

	/**
	 * @brief Get the battery charge value.
	 *
	 * @param id Identification of the robot.
	 * @return double Returns the value of battery energy of the specific robot.
	 */
	double get_battery(char id);

	/**
	 * @brief Get the ack count of identified robot.
	 *
	 * @param id Identification of the robot.
	 * @return ack_count Return the number of acks of the specific robot.
	 */
	ack_count get_ack_count(char id);

	/**
	 * @brief Resets all acks counters from robots.
	 * @details This method calls reset_lost_acks method from xbee (instance of SerialCom class).
	 * 	that will iterate through all robots inside robots vector and sets 0 to all.
	 */
	void reset_lost_acks();

	/**
	 * @brief This method creates the xbee connection passing the port and the baudrate.
	 * @details All connections are instances of SerialCom class.
	 * @param port String path of the serial port that handles xbee radio (Eg. /dev/ttyUSB1).
	 * @param baud Baudrate of the communication between host pc and xbee.
	 */
	void start_xbee(const std::string& port, int baud = 115200);

	/**
	 * @brief Stops xbee connection and purge the instance of xbee (instance of SerialCom class).
	 */
	void stop_xbee();

	/**
	* @brief Calculates the gap between sent messages
	*/	
	void update_msg_time();

	void set_frameskipper(int frames);
	
	int get_frameskipper();

	/**
	* @return time_between_msgs Gap between sent messages
	*/
	double get_time();

	/**
	 * @brief 
	 * 
	 * @tparam size 
	 */
	template<int size>
	msg_data<size> get_values(const std::string &msg, unsigned int first_char_pos);

	/**
	 * @brief 
	 * 
	 */
	void goToOrientation(Robots::Command command, std::string msg);

	/**
	 * @brief 
	 * 
	 * @param msg 
	 */
	void decode_msg(Robots::Command command, std::string msg);

	/**
	 * @brief 
	 * 
	 */
	void orientation_control(Robots::Command command, float new_theta, float velocity);

	/**
	 * @brief 
	 * 
	 */
	float round_angle(float angle);

	/**
	 * @brief 
	 * 
	 */
	float saturate(float value, float limit);
	void stop_and_wait();
};

#endif //VSSS_FlyingMessenger_H
