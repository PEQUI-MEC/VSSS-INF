#include "aux/debug.hpp"
#include "aux/equations.hpp"
#include "robots.hpp"

/* Null, because instance will be initialized on demand.
 * Initialized here because 'instance' is a static variable.
 */
Robots* Robots::instance = nullptr;

Robots* Robots::get_instance() {
    if (instance == nullptr) {
        instance = new Robots();
    }
    return instance;
}

Robots::Robots() {
    list.resize(N_ROBOTS);
    for(float & v : default_vel) v = 0.8;

    // Inicialização automática dos valores corretos para os robôs que usamos. Não é crucial pro funcionamento.
    // goleiro
    list.at(0).ID = 'C';
    default_vel[0] = 0.6; // mais precisão
    list.at(0).role = 0;
    // defensor
    list.at(1).ID = 'F';
    default_vel[1] = 0.8; // balanceado
    list.at(1).role = 1;
    // atacante
    list.at(2).ID = 'G';
    default_vel[2] = 1.0; // mais agilidade
    list.at(2).role = 2;
}

bool Robots::_check_index(const std::string& caller, int i) {
    if(i > N_ROBOTS || i < 0) {
        debug_error("[" + caller + "] Index out of range: " + to_string(i));
        return false;
    }
    return true;
}

std::vector<Robots::Command> Robots::get_commands() {
    std::vector<Command> res;
    res.resize(N_ROBOTS);

    for(unsigned short i = 0; i < N_ROBOTS; i++) {
        res.at(i).ID = get_instance()->list.at(i).ID;
        res.at(i).cmdType = get_instance()->list.at(i).cmdType;
        res.at(i).Status.position = get_instance()->list.at(i).position;
        res.at(i).Status.orientation = get_instance()->list.at(i).orientation;
        res.at(i).Status.vmax = get_instance()->list.at(i).vmax;
        switch(res.at(i).cmdType) {
            case CMD::POSITION:
                res.at(i).Msg.target = get_instance()->list.at(i).target;
                break;
            case CMD::SPEED:
                res.at(i).Msg.Velocity.left = get_instance()->list.at(i).Vl;
                res.at(i).Msg.Velocity.right = get_instance()->list.at(i).Vr;
                break;
            case CMD::ORIENTATION:
                res.at(i).Msg.theta = get_instance()->list.at(i).targetOrientation;
                break;
            case CMD::VECTOR:
                res.at(i).Msg.theta = get_instance()->list.at(i).transAngle;
                break;
            default: break;
        }
    }
    return res;
}
char Robots::get_ID(int robot) {
    if(!check_index(robot)) return 'N';
    return get_instance()->list.at(robot).ID;
}
bool Robots::get_usingPotField(int robot) {
    if(!check_index(robot)) return false;
    return get_instance()->list.at(robot).usingPotField;
}
cv::Point Robots::get_position(int robot) {
    if(!check_index(robot)) return cv::Point(-1, -1);
    return get_instance()->list.at(robot).position;
}
cv::Point Robots::get_target(int robot) {
    if(!check_index(robot)) return cv::Point(-1, -1);
    return get_instance()->list.at(robot).target;
}
double Robots::get_orientation(int robot) {
    if(!check_index(robot)) return 0;
    return get_instance()->list.at(robot).orientation;
}
cv::Point Robots::get_secondary_tag(int robot) {
    if(!check_index(robot)) return cv::Point(-1, -1);
    return get_instance()->list.at(robot).secondary;
}
bool Robots::get_fixedPos(int robot) {
    if(!check_index(robot)) return false;
    return get_instance()->list.at(robot).fixedPos;
}
Robots::CMD Robots::get_cmdType(int robot) {
    if(!check_index(robot)) return CMD::POSITION;
    return get_instance()->list.at(robot).cmdType;
}
float Robots::get_velocity(int robot) {
    if(!check_index(robot)) return 0;
    return get_instance()->list.at(robot).vmax;
}
float Robots::get_default_velocity(int robot) {
    if(!check_index(robot)) return 0;
    return get_instance()->default_vel[get_instance()->list.at(robot).role];
}
float Robots::get_default_role_velocity(int role) {
    if(!check_index(role)) return 0;
    return get_instance()->default_vel[role];
}
int Robots::get_role(int robot) {
    if(!check_index(robot)) return 0;
    return get_instance()->list.at(robot).role;
}
int Robots::get_status(int robot) {
    if(!check_index(robot)) return 0;
    return get_instance()->list.at(robot).status;
}
double Robots::get_transAngle(int robot) {
    if(!check_index(robot)) return 0;
    return get_instance()->list.at(robot).transAngle;
}

void Robots::set_ID(int robot, char ID) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).ID = ID;
}
void Robots::set_position(int robot, cv::Point position) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).position = position;
}
void Robots::set_orientation(int robot, double orientation) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).orientation = orientation;
}
void Robots::set_secondary_tag(int robot, cv::Point secondary) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).secondary = secondary;
}
void Robots::set_cmdType(int robot, CMD cmdType) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).cmdType = cmdType;
}
void Robots::set_fixedPos(int robot, bool fixedPos) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).fixedPos = fixedPos;
}
void Robots::set_usingPotField(int robot, bool usingPotField) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).usingPotField = usingPotField;
}
void Robots::set_velocity(int robot, int role) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).vmax = get_instance()->default_vel[role];
}
void Robots::set_velocity(int robot, float velocity) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).vmax = velocity;
}
void Robots::set_velocity(int robot) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).vmax = get_instance()->default_vel[get_instance()->list.at(robot).role];
}
void Robots::set_default_velocity(int robot, float vel) {
    if(!check_index(robot)) return;
    get_instance()->default_vel[get_instance()->list.at(robot).role] = vel;
}
void Robots::set_default_role_velocity(int role, float vel) {
    if(!check_index(role)) return;
    get_instance()->default_vel[role] = vel;
}
void Robots::set_status(int robot, int status) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).status = status;
}
void Robots::set_role(int robot, int role) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).role = role;
}
void Robots::set_transAngle(int robot, double transAngle) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).transAngle = transAngle;
}
void Robots::set_target(int robot, cv::Point target) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).target = target;
}
void Robots::set_command(int robot, cv::Point target, float vel) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).cmdType = CMD::POSITION;
    get_instance()->list.at(robot).target = target;
    if(vel == -1) get_instance()->list.at(robot).vmax = get_instance()->default_vel[get_instance()->list.at(robot).role];
    else get_instance()->list.at(robot).vmax = vel;
}
void Robots::set_command(int robot, float Vl, float Vr) {
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).cmdType = CMD::SPEED;
    get_instance()->list.at(robot).Vl = Vl;
    get_instance()->list.at(robot).Vr = Vr;
}
void Robots::set_command(int robot, double theta, float vel) { // vector - comando de orientação é o look_at
    if(!check_index(robot)) return;
    get_instance()->list.at(robot).cmdType = CMD::VECTOR;
    get_instance()->list.at(robot).transAngle = theta;
    if(vel == -1) get_instance()->list.at(robot).vmax = get_instance()->default_vel[get_instance()->list.at(robot).role];
    else get_instance()->list.at(robot).vmax = vel;
}

void Robots::reset_targets() {
    for(Robot & r : get_instance()->list) {
        r.target = cv::Point(-1, -1);
    }
}
void Robots::reset_commands() {
    for(Robot & r : get_instance()->list) {
        r.cmdType = CMD::VECTOR;
        r.target = cv::Point(-1, -1);
        r.Vl = r.Vr = 0;
        r.vmax = get_instance()->default_vel[r.role];
    }
}

bool Robots::is_posistion_set(int robot) {
    if(!check_index(robot)) return false;
    return get_instance()->list.at(robot).position.x != -1 && get_instance()->list.at(robot).position.y != -1;
}
bool Robots::is_target_set(int robot) {
    if(!check_index(robot)) return false;
    return get_instance()->list.at(robot).target.x != -1 && get_instance()->list.at(robot).target.y != -1;
}
void Robots::stop(int robot) {
    if(!check_index(robot)) return;
    // pode fazer de duas formas: mandando ele ir pra própria posição ou setando 0 nas rodas
    // get_instance()->list.at(robot).target = get_instance()->list.at(robot).position;
    get_instance()->list.at(robot).cmdType = CMD::SPEED;
    get_instance()->list.at(robot).vmax = 0;
    get_instance()->list.at(robot).Vr = 0;
    get_instance()->list.at(robot).Vl = 0;
    get_instance()->list.at(robot).target = cv::Point(-1, -1);
}
void Robots::spin_clockwise(int robot, float vel) {
    if(!check_index(robot)) return;
    if(vel == -1) vel = get_instance()->default_vel[get_instance()->list.at(robot).role];
    get_instance()->list.at(robot).cmdType = CMD::SPEED;
    get_instance()->list.at(robot).vmax = abs(vel);
    get_instance()->list.at(robot).Vr = -vel;
    get_instance()->list.at(robot).Vl = vel;
}
void Robots::spin_counter_clockwise(int robot, float vel) {
    if(!check_index(robot)) return;
    if(vel == -1) vel = get_instance()->default_vel[get_instance()->list.at(robot).role];
    spin_clockwise(robot, -vel);
}
void Robots::look_at(int robot, double orientation, float vel) {
    if(!check_index(robot)) return;
    if(vel == -1) vel = get_instance()->default_vel[get_instance()->list.at(robot).role];
    get_instance()->list.at(robot).cmdType = CMD::ORIENTATION;
    get_instance()->list.at(robot).targetOrientation = orientation;
    get_instance()->list.at(robot).vmax = vel;
}
void Robots::look_at(int robot, cv::Point target, float vel) {
    if(!check_index(robot)) return;
    if(vel == -1) vel = get_instance()->default_vel[get_instance()->list.at(robot).role];
    // !TODO verificar a necessidade do último cast pra double
    double orientation = double(atan2(double(target.y - get_instance()->list.at(robot).position.y), -double(target.x - get_instance()->list.at(robot).position.x)));
    get_instance()->list.at(robot).cmdType = CMD::ORIENTATION;
    get_instance()->list.at(robot).targetOrientation = orientation;
    get_instance()->list.at(robot).vmax = vel;
}
// !TODO usar o Univector Field
void Robots::kick(int robot, cv::Point target) {
    if(!check_index(robot)) return;
    cv::Point pos = get_instance()->list.at(robot).position;
    set_target(robot, target);
    get_instance()->list.at(robot).cmdType = CMD::VECTOR;
    // !TODO verificar a necessidade do último cast pra double
    get_instance()->list.at(robot).transAngle = double(-atan2(double(target.y - pos.y), double(target.x - pos.x)));
    get_instance()->list.at(robot).vmax = MAX_VEL;
}

// !TODO usar velocidade proporcional à distancias sem hardcode
void Robots::manage_velocity(int robot, cv::Point target) {
    if(!check_index(robot)) return;
    cv::Point pos = get_instance()->list.at(robot).position;
    double dist = EQ::distance(pos, target);
    if(dist > 200)
        get_instance()->list.at(robot).vmax = 0.8;
    else if(dist > 80)
        get_instance()->list.at(robot).vmax = 0.4;
    else
        get_instance()->list.at(robot).vmax = 0.1;
}
