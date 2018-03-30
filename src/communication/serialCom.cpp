#include <iostream>
#include "serialCom.hpp"


using namespace std;


SerialCom::SerialCom(const string &port, int baud) {

	xbee_err ret;

	if((ret = xbee_setup(&xbee, "xbee1", port.c_str(), baud)) != XBEE_ENONE) {
		printf("Xbee setup error: %d (%s)\n", ret, xbee_errorToStr(ret));
	} else {
		cout << "Xbee connected on " << port << endl;
	}

}


SerialCom::~SerialCom(){

	for(pair<const char,robot_xbee>& r : robots)
		xbee_conEnd(r.second.con);

	robots.clear();
	xbee_shutdown(xbee);

}


//void SerialCom::add_robot(char id, uint16_t addr) {
void SerialCom::newRobot(char id, uint16_t addr) {

	struct xbee_con *con;
	struct xbee_conAddress address{};

	memset(&address, 0, sizeof(address));
	address.addr16_enabled = 1;
	address.addr16[0] = (uint8_t) (addr >> 8);
	address.addr16[1] = (uint8_t) addr;

	xbee_err ret;

	if((ret = xbee_conNew(xbee, &con, "16-bit Data", &address)) != XBEE_ENONE) {
		printf("Xbee connection error: %d (%s)\n", ret, xbee_errorToStr(ret));
		return;
	}

	robots[id] = {id,addr,con,{0,0,0}};

}


//int SerialCom::send(char id, const string &message) {
int SerialCom::sendMessage(char id, const string &message) {

	if(robots.count(id) == 0)
		return -1;

	uint8_t ack;
	xbee_conTx(robots[id].con, &ack, message.c_str());
	update_ack(id, ack);

	return ack;

}


//vector<message> SerialCom::get_messages() {
vector<message> SerialCom::getMessages() {

	struct xbee_pkt *pkt;
	vector<message> msgs;

	for(auto& r : robots){
		if(xbee_conRx(r.second.con, &pkt, nullptr) == XBEE_ENONE && pkt->dataLen > 0)
			msgs.push_back({r.second.id,getString(pkt)});

	}

	return msgs;

}


//vector<message> SerialCom::send_get_answer_all(const std::string &message) {
vector<message> SerialCom::getResponseAll(const std::string &message) {

	vector<struct message> msgs;

	for(auto& r : robots){
		string ret = getResponse(r.second.id,message);
		if(!ret.empty())
			msgs.push_back({r.second.id,ret});
	}

	return msgs;

}


//string SerialCom::send_get_answer(char id, const string &message) {
string SerialCom::getResponse(char id, const string &message) {

	if(robots.count(id) == 0)
		return "erro";

	struct xbee_pkt *pkt;
	uint8_t ack;
	xbee_conTx(robots[id].con, &ack, message.c_str());
	update_ack(id, ack);

	if(ack == 0 && xbee_conRxWait(robots[id].con, &pkt, nullptr) == XBEE_ENONE && pkt->dataLen > 0) {
		return getString(pkt);
	} else {
		return "erro";
	}

}


void SerialCom::update_ack(char id, int ack) {

	ack_count& acks = robots[id].acks;

	if(ack != 0)
		acks.lost++;

	acks.total++;
	acks.lost_rate = double(acks.lost)/double(acks.total)*100;

}

//string SerialCom::get_string(struct xbee_pkt *pkt){
string SerialCom::getString(struct xbee_pkt *pkt) {

	string str(pkt->data, pkt->data+pkt->dataLen);
	return str;

}


ack_count SerialCom::get_ack_count(char id) {

	return robots[id].acks;

}


void SerialCom::reset_lost_acks() {

	for(auto& r : robots)
		r.second.acks = {0,0,0};

}
