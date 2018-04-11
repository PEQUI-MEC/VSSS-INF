/*
 * v4ldev.hpp
 *
 *  Created on: Feb 20, 2014
 *      Author: gustavo
 */

#ifndef V4LDEV_HPP_
#define V4LDEV_HPP_

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <libv4l2.h>
#include <linux/videodev2.h>

#include <string.h> /* menset */
#include <iostream> /* cout, endl, ... */

class v4ldev {
	int fd;
	bool emulate_format;

private:

	int xioctl(int fd, int request, void * arg);
	
public:
	v4ldev();

	~v4ldev();

	bool open_device(const char * devname, bool emulate_fmt);
	
	/* IOCTLs */
	bool query_capability(struct v4l2_capability * cap);
	
	bool get_input_index(int * input);
	
	bool get_input(struct v4l2_input * input);
	
	bool get_format(struct v4l2_format * fmt, enum v4l2_buf_type type);
	
	bool get_standard_id(v4l2_std_id * std_id);
	
	bool get_standard(struct v4l2_standard * standard);
	
	bool get_stream_parameter(struct v4l2_streamparm * streamparm);
	
	bool get_stream_parameter(struct v4l2_streamparm * streamparm, __u32 type);
	
	bool get_control(struct v4l2_control * ctrl);
	
	bool get_control(struct v4l2_control * ctrl, unsigned int ctrl_id);
	
	bool query_buffer(struct v4l2_buffer * buffer);
	
	bool query_buffer(struct v4l2_buffer * buf, enum v4l2_buf_type type, enum v4l2_memory memory, unsigned int index);
	
	bool enqueue_buff(struct v4l2_buffer * vbuf);
	
	bool enqueue_buff(struct v4l2_buffer * vbuf, enum v4l2_buf_type buf_type, enum v4l2_memory memory, int index);
	
	bool streamon(enum v4l2_buf_type type);
	
	bool streamoff(enum v4l2_buf_type type);
	
	bool dequeue_buff(struct v4l2_buffer * vbuf);
	
	bool dequeue_buff(struct v4l2_buffer * vbuf, unsigned int buf_type, unsigned int memory, bool * again);
	
	bool query_control(struct v4l2_queryctrl * qctrl);
	
	bool query_control_menu(struct v4l2_querymenu * qmenu);
	
	bool request_buffers(struct v4l2_requestbuffers * req_buff);
	
	bool request_buffers(struct v4l2_requestbuffers * req_buff, int nbuffers, enum v4l2_memory memory, enum v4l2_buf_type type);
};

#endif /* V4LDEV_HPP_ */
