#include "flyingMessenger.hpp"

using namespace std;

void FlyingMessenger::start_xbee(const std::string &port, int baud) {
	xbee = new SerialCom(port, baud);
	add_robots();
}

void FlyingMessenger::stop_xbee() {
	if(!xbee) return;
	delete xbee;
	xbee = nullptr;
}

void FlyingMessenger::add_robots() {
	if(!xbee) return;

	xbee->newRobot('A', 0x88a0);
	xbee->newRobot('B', 0xb24a);
	xbee->newRobot('C', 0x215c);
	xbee->newRobot('D', 0x35f6);
	xbee->newRobot('E', 0x97e7);
	xbee->newRobot('F', 0x6b0d);
	xbee->newRobot('G', 0x56bc);
}

void FlyingMessenger::send_msg(char id, std::string msg) {
	if(!xbee) return;
	xbee->sendMessage(id,msg);
}

void FlyingMessenger::send_old_format(std::string cmd) {
	if(!xbee) return;

	char id = cmd[0];
	std::string msg = cmd.substr(2,cmd.find('#')-2);
	xbee->sendMessage(id,msg);
}

std::vector<message> FlyingMessenger::sendCMDs(std::vector<Robot> robots) {
	vector<message> acks;
	if(!xbee) return acks;

	for(Robot robot : robots){
		std::string msg;
		switch (robot.cmdType){
			case POSITION:
				if(robot.target.x != -1 && robot.target.y != -1)
					msg = position_msg(robot);
				break;

			case SPEED:
				msg = speed_msg(robot);
				break;

			case ORIENTATION:
				msg = orientation_msg(robot);
				break;

			case VECTOR:
				msg = vector_msg(robot);
				break;

			default:
				if(robot.target.x != -1 && robot.target.y != -1)
					msg = position_msg(robot);
		}

		if(!msg.empty()){
			int ack = xbee->sendMessage(robot.ID,msg);
			acks.push_back({robot.ID, std::to_string(ack)});
		}
	}

	return acks;
}

std::string FlyingMessenger::position_msg(Robot robot) {
	double diff_x = robot.target.x - robot.position.x;
	double diff_y = robot.target.y - robot.position.y;

	double transTarget_x = cos(robot.orientation)*diff_x + sin(robot.orientation)*diff_y;
	double transTarget_y = -(-sin(robot.orientation)*diff_x + cos(robot.orientation)*diff_y);

	double pos_x = transTarget_x*(150.0/640.0);
	double pos_y = transTarget_y*(130.0/480.0);

	return ("P"+ rounded_str(pos_x)+";"+ rounded_str(pos_y)+";"+ rounded_str(robot.vmax));
}

std::string FlyingMessenger::speed_msg(Robot robot) {
	return (rounded_str(robot.Vr)+";"+ rounded_str(robot.Vl));
}

std::string FlyingMessenger::orientation_msg(Robot robot) {
	double orientation = (robot.orientation + robot.targetOrientation)*180/M_PI;
	return ("O"+ rounded_str(orientation)+";"+ rounded_str(robot.vmax));
}

std::string FlyingMessenger::vector_msg(Robot robot) {
	double orientation = atan2(sin(robot.orientation + robot.transAngle), cos(robot.orientation + robot.transAngle));
	orientation = orientation*(180.0/M_PI);
	return ("V"+ rounded_str(orientation)+";"+ rounded_str(robot.vmax));
}

double FlyingMessenger::get_battery(char id) {
	if(!xbee) return -1;
	std::string msg = xbee->getResponse(id,"B");
	if(msg.empty() || msg[0] != 'B') return -1;
	return ((stod(msg.substr(1)) - 6.4)/2.0)*100;
}

std::string FlyingMessenger::rounded_str(double num) {
	double rounded_num = round(num*100)/100;
	std::ostringstream ss;
	ss << rounded_num;

	return ss.str();
}

ack_count FlyingMessenger::get_ack_count(char id) {
	if(!xbee) return {-1,-1,-1};
	else return xbee->get_ack_count(id);
}

void FlyingMessenger::reset_lost_acks() {
	if(!xbee) return;
	xbee->reset_lost_acks();
}

FlyingMessenger::FlyingMessenger() {
	setlocale(LC_ALL,"C");
}
