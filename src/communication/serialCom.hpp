#ifndef VSSS_XBEE_H
#define VSSS_XBEE_H

#include <xbee.h>
#include <vector>
#include <string>
#include <stack>
#include <cstring>
#include <unordered_map>


struct message {
	char id;
	std::string data;
};


struct ack_count {
	int lost;
	int total;
	double lost_rate;
};


struct robot_xbee {
	char id;
	uint16_t addr;
	xbee_con *con;
	ack_count acks;
};


class SerialCom {

	private:

		struct xbee *xbee;
		std::unordered_map<char, robot_xbee> robots;

	public:

		SerialCom(const std::string &port, int baud);
		~SerialCom();
		//void add_robot(char id, uint16_t addr);
    	void newRobot(char id, uint16_t addr);
		//int send(char id, const std::string &message);
    	int sendMessage(char id, const std::string &message);
    	//std::vector<message> get_messages();
    	std::vector<message> getMessages();
		//std::string send_get_answer(char id, const std::string &message);
    	std::string getResponse(char id, const std::string &message);
		//std::vector<message> send_get_answer_all(const std::string &message);
    	std::vector<message> getResponseAll(const std::string &message);
    	//std::string get_string(xbee_pkt *pkt);
    	std::string getString(xbee_pkt *pkt);

		void update_ack(char id, int ack);
		ack_count get_ack_count(char id);
		void reset_lost_acks();
		
};


#endif //VSSS_XBEE_H
