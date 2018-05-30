#include "aux/debug.hpp"

#ifndef STRATEGY_HPP_
#define STRATEGY_HPP_

#include "opencv2/opencv.hpp"
#include "aux/ls.hpp"
#include "robots.hpp"
#include "gui/testFrame.hpp"
#include "constants.hpp"
#include <vector>

#define N_ROBOTS 3
// potField
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
	double ball_angle = 0;
	double ball_mag = 0;

	std::vector<cv::Point> adv;
	int collision_count[N_ROBOTS];
	double past_transangle[N_ROBOTS];
	cv::Point past_position[N_ROBOTS];
	int atk, def, gk, opp;
	double lock_angle;

	int frames_blocked;

	bool danger_zone_1 = false;
	bool danger_zone_2 = false;

	bool transitions_enabled = true;

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
	
	void get_targets(std::vector<cv::Point> advRobots);
	
	void reset_flags();

	void set_roles();

	bool has_ball(int i);

	void overmind();
	
	// void set_flags();
	
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
	
	void def_wait(int i);
	
	void pot_field_around(int i);
	
	void crop_targets(int i);
	
	void atk_routine(int i);
	
	void def_routine(int i);
	
	void gk_routine(int i);
	
	void set_Ball(cv::Point b);

    void set_default_vel(float default_vel[]);

	cv::Point get_Ball_Est();

	void get_past(int i);

	void get_variables();
	
};

#endif /* STRATEGY_HPP_ */
