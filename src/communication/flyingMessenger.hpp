#ifndef VSSS_FlyingMessenger_H
#define VSSS_FlyingMessenger_H

#include "../robot.hpp"
#include "serialCom.hpp"
#include <string>
#include <vector>
#include <cmath>

#define POSITION 0
#define SPEED 1
#define ORIENTATION 2
#define VECTOR 3


class FlyingMessenger {

	private:

		SerialCom* xbee = nullptr;
		void add_robots();
		std::string position_msg(Robot robot);
		std::string speed_msg(Robot robot);
		std::string orientation_msg(Robot robot);
		std::string vector_msg(Robot robot);
		std::string rounded_str(double num);

	public:

		FlyingMessenger();
		std::vector<message> sendCMDs(std::vector<Robot> robots);
		void send_msg(char id, std::string msg);
		void send_old_format(std::string cmd);
		double get_battery(char id);
		ack_count get_ack_count(char id);
		void reset_lost_acks();
		void start_xbee(const std::string& port, int baud = 115200);
		void stop_xbee();

};


#endif //VSSS_FlyingMessenger_H