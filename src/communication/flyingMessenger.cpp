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

std::vector<message> FlyingMessenger::sendCMDs(std::vector<Robots::Command> commands) {
	vector<message> acks;
	if(!xbee || ++send_cmd_count <= frameskip) return acks;

	for(Robots::Command command : commands){
		std::string msg;
		switch (command.cmdType){
			case Robots::CMD::SPEED:
				msg = speed_msg(command);
				break;

			case Robots::CMD::ORIENTATION:
				msg = orientation_msg(command);
				break;

			case Robots::CMD::VECTOR:
				msg = vector_msg(command);
				break;

			default: // POSITION
                if(command.Msg.target.x != -1 && command.Msg.target.y != -1)
                    msg = position_msg(command);
		}

		if(!msg.empty()) {
			int ack = xbee->sendMessage(command.ID,msg);
			acks.push_back({command.ID, std::to_string(ack)});
		}
	}
	
	update_msg_time();
	send_cmd_count=0;

	return acks;
}

std::string FlyingMessenger::position_msg(Robots::Command command) {
	double diff_x = command.Msg.target.x - command.Status.position.x;
	double diff_y = command.Msg.target.y - command.Status.position.y;

	double transTarget_x = cos(command.Status.orientation)*diff_x + sin(command.Status.orientation)*diff_y;
	double transTarget_y = -(-sin(command.Status.orientation)*diff_x + cos(command.Status.orientation)*diff_y);

	double pos_x = transTarget_x*(150.0/640.0);
	double pos_y = transTarget_y*(130.0/480.0);

	return ("P"+ rounded_str(pos_x)+";"+ rounded_str(pos_y)+";"+ rounded_str(command.Status.vmax));
}

std::string FlyingMessenger::speed_msg(Robots::Command command) {
	return (rounded_str(command.Msg.Velocity.right)+";"+ rounded_str(command.Msg.Velocity.left));
}

std::string FlyingMessenger::orientation_msg(Robots::Command command) {
	double orientation = (command.Status.orientation + command.Msg.theta)*180/M_PI;
	return ("O"+ rounded_str(orientation)+";"+ rounded_str(command.Status.vmax));
}

std::string FlyingMessenger::vector_msg(Robots::Command command) {
	double orientation = atan2(sin(command.Status.orientation + command.Msg.theta), cos(command.Status.orientation + command.Msg.theta));
	orientation = orientation*(180.0/M_PI);
	return ("V"+ rounded_str(orientation)+";"+ rounded_str(command.Status.vmax));
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

void FlyingMessenger::set_frameskipper(int frames) { 
	frameskip = frames;
}

int FlyingMessenger::get_frameskipper() {
	return frameskip;
}

double FlyingMessenger::get_time() {
	return time_between_msgs;
}

void FlyingMessenger::update_msg_time() {
	auto now = std::chrono::system_clock::now();
	std::chrono::duration<double, std::milli> time_diff = now - previous_msg_time;
	time_between_msgs = time_diff.count();
	previous_msg_time = now;
}

FlyingMessenger::FlyingMessenger() {
	setlocale(LC_ALL,"C");
	send_cmd_count = 0;
	frameskip = DEFAULT_FRAMESKIP;
	previous_msg_time = std::chrono::system_clock::now();
}
