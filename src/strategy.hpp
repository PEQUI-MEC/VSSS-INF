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

/*
// potField
#define BALL_IS_OBS 0
#define BALL_ONLY_OBS 1
#define BALL_NOT_OBS 2
#define NO_OBS 3
 */


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

	/*
	double pot_thetaX = 0;
	double pot_thetaY = 0;
	double pot_theta = 0;
	double pot_goalTheta = 0;
	double pot_goalMag;
	double pot_magnitude[5];
	double pot_angle[5];
	double ball_angle = 0;
	double ball_mag = 0;
    */

	std::vector<cv::Point> adv;
	int collision_count[N_ROBOTS];
	double past_transangle[N_ROBOTS];
	cv::Point past_position[N_ROBOTS];
	int atk, def, gk, opp;
	// double lock_angle;

	int frames_blocked;

	bool transitions_enabled = true;

	int t = 0;
	// int timeout = 0;
	// int transition_timeout = 0;
	// bool action1 = false;

	Strategy();
	
	void set_constants(int w, int h);
	
	void get_targets(std::vector<cv::Point> advRobots);
	
	void reset_flags();

	void set_roles();

	bool has_ball(int robot_index);
	
	void fixed_position_check(int robot_index);
	
	// bool offensive_adv();
	
	void collision_check(int robot_index);
	
	void set_role(int robot_index, int role);
	
	bool is_near(int robot_index, cv::Point point);
	
	void position_to_vector(int robot_index);

	// double potField(int robot_index, cv::Point goal, int behavior=BALL_NOT_OBS);

	// int pot_rotation_decision(int robot_index,cv::Point goal, cv::Point obst);
	
	void def_wait(int robot_index);

	// void pot_field_around(int robot_index);
	
	void crop_targets(int robot_index);
	
	void atk_routine(int robot_index);
	
	void def_routine(int robot_index);
	
	void gk_routine(int robot_index);
	
	void set_Ball(cv::Point b);

	cv::Point get_Ball_Est();

	void get_past(int robot_index);

	void get_variables();
};

#endif /* STRATEGY_HPP_ */
