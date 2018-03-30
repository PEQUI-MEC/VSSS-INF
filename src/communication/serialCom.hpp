#ifndef VSSS_XBEE_H
#define VSSS_XBEE_H

#include <xbee.h>
#include <vector>
#include <string>
#include <stack>
#include <cstring>
#include <unordered_map>

/**
 * 
 */
struct message {
	char id;
	std::string data;
};

/**
 * 
 */
struct ack_count {
	int lost;
	int total;
	double lost_rate;
};

/**
 * 
 */
struct robot_xbee {
	char id;
	uint16_t addr;
	xbee_con *con;
	ack_count acks;
};


class SerialCom {

	private:

		/**
		 * @brief Pointer that points to xbee struct inside xbee.h.
		 */
		struct xbee *xbee;
		
		/**
		 * @brief unordered map that stores all robots.
		 */
		std::unordered_map<char, robot_xbee> robots;

	public:

		/**
		 * @brief Constructor of a new Serial Com object.
		 * 
		 * @param port String path of the serial port that handles xbee radio (Eg. /dev/ttyUSB1).
		 * @param baud Baudrate of the communication between host pc and xbee.
		 */
		SerialCom(const std::string &port, int baud);

		/**
		 * @brief Destroy the SerialCom object
		 */
		~SerialCom();

		/**
		 * @brief Receives ID and address of the robot and creates a connection to the robot.
		 * @details Robots connection are added using xbee_conNew method provided from xbee library. 
		 *  xbee_conNew creates a new connection via the given libxbee instance. The new connection 
		 *  will be returned via 'con', and the connection's type is provided by type. From xbee attie 
		 *  library the types are available on http://doc.libxbee.attie.co.uk/man3/xbee_conNew.3.html.
		 * 
		 * @param id Robot ID provided by vision.
		 * @param addr Address provided statically from FlyingMessenger class.
		 */
		//void add_robot(char id, uint16_t addr);
    void newRobot(char id, uint16_t addr);

		/**
		 * @brief Method that sends message to a specific robot identified by robot ID.
		 * @details This method calls xbee_conTx method from xbee library. This method receives a ACK return from the sent message and update ACK count using update_ack method.
		 * 
		 * @param id Robot ID that identifies specific robot.
		 * @param message Message that will be sent to robot.
		 * @return Returns ACK received from xbee_conTx method.
		 */
		//int send(char id, const std::string &message);
    int sendMessage(char id, const std::string &message);

		/**
		 * @brief Method that receives messages from remote xbees and returns a vector with all messages received.
		 * 
		 * @return Returns vector of struct message that handles char id and string data.
		 */
    //std::vector<message> get_messages();
    std::vector<message> getMessages();

		/**
		 * This method sends a message to a specific robot taged by your identification number and waits for xbee sends back the string.
		 * 
		 * @param id The robot identification to get message from.
		 * @param message Message that will be sent to the robot.
		 * @return Returns a string that contains the robot[id] message.
		 */
		//std::string send_get_answer(char id, const std::string &message);
    std::string getResponse(char id, const std::string &message);


		/**
		 * @brief The same functionality of getResponse method but sends message and waits response from all robots.
		 * 
		 * @param message Message that will be sent to the robots.
		 * @return std::vector<message> Returns a vector containing all robots return messages.
		 */
		//std::vector<message> send_get_answer_all(const std::string &message);
		std::vector<message> getResponseAll(const std::string &message);

		/**
		 * @brief 
		 * 
		 * @param pkt Package received from the robot of type xbee_pkt (struct defined on xbee.h library).
		 * @return std::string Message converted.
		 */
        //std::string get_string(xbee_pkt *pkt);
		std::string getString(xbee_pkt *pkt);

		/**
		 * @brief This method updates the ack of type struct ack_count that contains the ack return validation.
		 * 
		 * @param id Identification of the robot owner of the ack counter.
		 * @param ack Ack returned from robot.
		 */
		void update_ack(char id, int ack);

		/**
		 * @brief Get the ack from a identified robot.
		 * 
		 * @param id Identification of the robot member.
		 * @return ack_count Number of acks of the robot.
		 */
		ack_count get_ack_count(char id);

		/**
		 * @brief This method resets all acks received from all robots.
		 */
		void reset_lost_acks();
		
};


#endif //VSSS_XBEE_H
