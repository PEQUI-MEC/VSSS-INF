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

		decode_msg(msg);
		if(command.cmdType == Robots::CMD::ORIENTATION) {
			msg = final_msg;
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

// verificar sobre alterar o command e passar como argumento
std::string FlyingMessenger::velocity_msg(float right_wheel, float left_wheel) {
	return (rounded_str(right_wheel)+";"+ rounded_str(left_whe));
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

template<int size>
msg_data<size> FlyingMessenger::get_values(const string &msg, unsigned int first_char_pos) {
	std::array<float,size> values{};
	unsigned int pos_atual = first_char_pos;
	for (int i = 0; i < size; ++i) {
		size_t delim_pos = msg.find(';', pos_atual);
		if (delim_pos == string::npos && i != size - 1) return {values,false};
		values[i] = std::stof(msg.substr(pos_atual, delim_pos - pos_atual));
		pos_atual = delim_pos + 1;
	}
	return {values,true};
}

void FlyingMessenger::goToOrientation(string msg) {
	msg_data<2> values = get_values<2>(msg, 1);
	if(values.is_valid)
		orientation_control(values[0], values[1]);
}

void FlyingMessenger::decode_msg(string msg) {

	switch (msg[0]) {
		case 'U':
			uvf_message(msg);
			return;
		case 'K':
			if(msg[1] == 'P') Update_PID_Pos(msg);
			else Update_PID_K(msg);
			return;
		case 'A':
			Update_ACC(msg);
			return;
		case 'O':
			goToOrientation(msg);
			return;
		case 'P':
			GoToPoint(msg);
			return;
		case 'V':
			GoToVector(msg);
			return;
		case 'D':
			debug_mode = !debug_mode;
//			(*r).controllerA->debug_mode = debug_mode;
//			(*r).controllerB->debug_mode = debug_mode;
			return;
		case 'B':
			send_battery();
			return;
		default:
			break;
	}

}

void FlyingMessenger::orientation_control(float new_theta, float velocity) {

	float old_theta = command.Status.orientation;

	if(round_angle(new_theta - old_theta + PI/2) < 0){
		old_theta = round_angle(old_theta + PI);
	}
	float theta_error = round_angle(new_theta - old_theta);
	
	if(std::abs(theta_error) < 2*PI/180) {
		stop_and_wait();
		return;
	}

	float right_wheel_velocity = saturate(orientation_Kp * theta_error, 1);
	float left_wheel_velocity = saturate(-orientation_Kp * theta_error, 1);

	final_msg = velocity_msg(left_wheel_velocity, right_wheel_velocity);
}

float FlyingMessenger::round_angle(float angle) {
	float theta = std::fmod(angle, 2*PI);
	if(theta > PI) theta = theta - 2*PI;
	else if(theta < -PI) theta = theta + 2*PI;
	return theta;
}

float FlyingMessenger::saturate(float value, float limit) {
	if(value > limit) value = limit;
	if(value < -limit) value = -limit;
	return value;
}

FlyingMessenger::FlyingMessenger() {
	setlocale(LC_ALL,"C");
	send_cmd_count = 0;
	frameskip = DEFAULT_FRAMESKIP;
	previous_msg_time = std::chrono::system_clock::now();
}
