/*
 * ImageView.hpp
 *
 *  Created on: Feb 3, 2014
 *      Author: gustavo
 */

#ifndef IMAGEVIEW_HPP_
#define IMAGEVIEW_HPP_

#include <gtkmm.h>
#include <cairomm/context.h>
#include <iostream>
#include "opencv2/opencv.hpp"

#pragma once

class ImageView: public Gtk::DrawingArea {
	Glib::RefPtr<Gdk::Pixbuf> pb;
	virtual bool on_button_press_event(GdkEventButton *event);

public:

	unsigned char * data;
	int width, height, stride;

	int warp_mat[4][2];
	int adjust_mat[4][2];
	int tar_pos[2];
	int robot_pos[2];
	int look_pos[2];
	cv::Point pointClicked;

	int warp_counter =0;
	int adj_counter =0;
	bool warp_event_flag = false;
	bool PID_test_flag = false;
	bool formation_flag = false;
	bool adjust_event_flag = false;
	bool hold_warp = false;
	bool adjust_rdy = false;
	bool split_flag = false;
	int sector = -1;

	ImageView();

	void set_data(unsigned char * data, int width, int height);

	unsigned char * get_data();

	int get_width();

	int get_height();

	void disable_image_show();

	void refresh();

	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif /* IMAGEVIEW_HPP_ */
