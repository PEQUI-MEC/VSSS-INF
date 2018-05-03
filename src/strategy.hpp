#ifndef STRATEGY_HPP_
#define STRATEGY_HPP_
#define PI 3.14159265453
#include "opencv2/opencv.hpp"
#include "aux/ls.hpp"
#include "robot.hpp"
#include <math.h>
#include "gui/testFrame.hpp"
#include "constants.hpp"
#include <vector>

#define N_ROBOTS 3

#define PREDICAO_NUM_SAMPLES 15

//role
#define GOALKEEPER 0
#define DEFENDER 1
#define ATTACKER 2
#define OPPONENT 3

//cmdType
#define POSITION 0
#define SPEED 1
#define ORIENTATION 2
#define VECTOR 3
#define VECTORRAW 4

//state
#define NORMAL_STATE 0
#define CORNER_STATE 1
#define	STEP_BACK 2
#define	ADVANCING_STATE 3
#define	TRANSITION_STATE 4
#define	SIDEWAYS 5
#define DEF_CORNER_STATE 6
#define ATK_PENALTI_STATE 7

//potField
#define BALL_IS_OBS 0
#define BALL_ONLY_OBS 1
#define BALL_NOT_OBS 2
#define NO_OBS 3


class Strategy {
private:
	double distance(cv::Point A, cv::Point B);
	double distance_meters(cv::Point A, cv::Point B);
public:
	TestFrame testFrame;
	
	cv::Point Ball;
    cv::Point Ball_Est;
	int distBall;
	LS LS_ball_x;
	LS LS_ball_y;

	double pot_thetaX = 0;
	double pot_thetaY = 0;
	double pot_theta = 0;
	double pot_goalTheta = 0;
	double pot_goalMag;
	double pot_magnitude[5];
	double pot_angle[5];
	float ball_angle = 0;
	float ball_mag = 0;

	std::vector<Robot> robots;
	cv::Point* adv;
    float default_vel[N_ROBOTS];
	int collision_count[N_ROBOTS];
	double past_transangle[N_ROBOTS];
	cv::Point past_position[N_ROBOTS];
	int atk, def, gk, opp;
	double lock_angle;

	int frames_blocked;

	bool half_transition = false;
	bool full_transition = false;
	bool danger_zone_1 = false;
	bool danger_zone_2 = false;

	bool half_transition_enabled = true;
	bool full_transition_enabled = true;
	bool transition_mindcontrol_enabled = true;

	int t = 0;
	int timeout = 0;
	int transition_timeout = 0;
	int transition_overmind_timeout = 0;
	bool action1 = false;
	bool transition_mindcontrol = false;
	bool atk_mindcontrol = false;
	bool def_mindcontrol = false;

	Strategy();
	
	void set_constants(int w, int h);
	
	void get_variables();
	
	void get_past(int i);
	
	void get_targets(std::vector<Robot> * pRobots, cv::Point * advRobots);
	
	void overmind();
	
	void set_flags();
	
	void fixed_position_check(int i);
	
	bool offensive_adv();
	
	void collision_check(int i);
	
	bool cock_blocked();
	
	void set_role(int i, int role);
	
	bool is_near(int i, cv::Point point);
	
	void position_to_vector(int i);

	double look_at_ball(int i);
	
	double potField(int robot_index, cv::Point goal, int behavior=BALL_NOT_OBS);
	
	int pot_rotation_decision(int robot_index,cv::Point goal, cv::Point obst);
	
	void spin_anti_clockwise(int i, double speed=0.8);
	
	void spin_clockwise(int i, double speed=0.8);
	
	void def_wait(int i);
	
	void pot_field_around(int i);
	
	void crop_targets(int i);
	
	void atk_routine(int i);
	
	void def_routine(int i);
	
	void gk_routine(int i);
	
	void set_Ball(cv::Point b);

    void set_default_vel(float default_vel[]);

	cv::Point get_Ball_Est();
	
};

#endif /* STRATEGY_HPP_ */
