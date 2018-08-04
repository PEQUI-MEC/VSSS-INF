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

	/* UVF constants
	 * double UVFConstants::_de = 0.06;
	 * double UVFConstants::_kr = 0.7;
	 * double UVFConstants::_dmin = 0.04;
	 * double UVFConstants::_delta = 0.15;
	 */
	double radiusSpiral = 0.06;
	double kr = 0.7;

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
	// double lock_angle;

	int frames_blocked;

	bool transitions_enabled = true;

	int t = 0;
	int timeout = 0;
	int transition_timeout = 0;
	bool action1 = false;

	Strategy();
	
	void set_constants(int w, int h);
	
	void get_targets(std::vector<cv::Point> advRobots);
	
	void reset_flags();

	void set_roles();

	bool has_ball(int robotIndex);
	
	void fixed_position_check(int robotIndex);
	
	// bool offensive_adv();
	
	void collision_check(int robotIndex);
	
	void set_role(int robotIndex, int role);
	
	bool is_near(int robotIndex, cv::Point point);
	
	void position_to_vector(int robotIndex);

	double potField(int robotIndex, cv::Point goal, int behavior=BALL_NOT_OBS);

	int pot_rotation_decision(int robotIndex,cv::Point goal, cv::Point obst);
	
	void def_wait(int robotIndex);

	void pot_field_around(int robotIndex);
	
	void crop_targets(int robotIndex);
	
	void atk_routine(int robotIndex);
	
	void def_routine(int robotIndex);
	
	void gk_routine(int robotIndex);
	
	void set_Ball(cv::Point b);

	cv::Point get_Ball_Est();

	void get_past(int robotIndex);

	void get_variables();

	// UVF
	void uvf(int robotIndex);
	double move_to_goal(int robotIndex, cv::Point pos);
	double hyper_spiral_fih(cv::Point p, bool clockwise;
	/*
    std::pair<double,double> hyper_spiral_nh(cv::Point p, bool clockwise, int robotIndex);
    double wrap_to_pi(double theta);
    double angle_with_x (cv::Point p);
	*/
};

#endif /* STRATEGY_HPP_ */
