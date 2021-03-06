#include "v4ldev.hpp"

int v4ldev::xioctl(int fd, int request, void * arg) {
	int r;
	if (emulate_format) r = v4l2_ioctl(fd, request, arg);
	else r = ioctl(fd, request, arg);

	return r;
}
	
v4ldev::v4ldev() : fd(0), emulate_format(false) {};

v4ldev::~v4ldev() {};

bool v4ldev::open_device(const char * devname, bool emulate_fmt) {
	fd = open(devname, O_RDWR);
	emulate_format = emulate_fmt;

	if(fd < 0) {
		std::cout << "Can not open " << devname << "device!" << std::endl;
		return false;
	}

	if(emulate_fmt) {
		int fd2 = v4l2_fd_open(fd, V4L2_ENABLE_ENUM_FMT_EMULATION);
		if(fd2 < 0) {
			std::cout << "Cannot use libv4l2 wrapper for " << devname << "!" << std::endl;
			emulate_fmt = false;
		}
	}

	struct v4l2_capability cap;
	if(!query_capability(&cap)) {
		std::cout << devname << " is not a V4L2 device!" << std::endl;

		if(emulate_fmt) v4l2_close(fd);
		else close(fd);

		return false;
	}
	
	return true;
}

bool v4ldev::query_capability(struct v4l2_capability * cap) {
	return xioctl(fd, VIDIOC_QUERYCAP, cap) >= 0;
}

bool v4ldev::get_input_index(int * input) {
	return xioctl(fd, VIDIOC_G_INPUT, input) >= 0;
}

bool v4ldev::get_input(struct v4l2_input * input) {
	int index;
	if (!get_input_index(&index)) return false;
	input->index = index;

	return xioctl(fd, VIDIOC_ENUMINPUT, input) >= 0;
}

bool v4ldev::get_format(struct v4l2_format * fmt, enum v4l2_buf_type type) {
	fmt->type = type;
	return xioctl(fd, VIDIOC_G_FMT, fmt) >= 0;
}

bool v4ldev::get_standard_id(v4l2_std_id * std_id) {
	return xioctl(fd, VIDIOC_G_STD, std_id) >= 0;
}

bool v4ldev::get_standard(struct v4l2_standard * standard) {
	v4l2_std_id std_id;

	if (!get_standard_id(&std_id)) return false;

	standard->id = std_id;

	for (__u32 index = 0;; ++index) {
		standard->index = index;
		if (standard->id == std_id) return true;
		if (!xioctl(fd, VIDIOC_ENUMSTD, standard)) break;
	}
	
	return false;
}

bool v4ldev::get_stream_parameter(struct v4l2_streamparm * streamparm) {
	return xioctl(fd, VIDIOC_G_PARM, streamparm) >= 0;
}

bool v4ldev::get_stream_parameter(struct v4l2_streamparm * streamparm, __u32 type) {
	streamparm->type = type;
	return get_stream_parameter(streamparm);
}

bool v4ldev::get_control(struct v4l2_control * ctrl) {
	return xioctl(fd, VIDIOC_G_CTRL, ctrl) >= 0;
}

bool v4ldev::get_control(struct v4l2_control * ctrl, unsigned int ctrl_id) {
	ctrl->id = ctrl_id;
	return get_control(ctrl);
}

bool v4ldev::query_buffer(struct v4l2_buffer * buffer) {
	int r = xioctl(fd, VIDIOC_QUERYBUF, buffer);

	if (r >= 0) return true;

	perror("VIDIOC_QUERYBUF");

	return false;
}

bool v4ldev::query_buffer(struct v4l2_buffer * buf, enum v4l2_buf_type type, enum v4l2_memory memory, unsigned int index) {
	buf->type = type;
	buf->memory = memory;
	buf->index = index;

	return query_buffer(buf);
}

bool v4ldev::enqueue_buff(struct v4l2_buffer * vbuf) {
	return xioctl(fd, VIDIOC_QBUF, vbuf) >= 0;
}

bool v4ldev::enqueue_buff(struct v4l2_buffer * vbuf, enum v4l2_buf_type buf_type, enum v4l2_memory memory, int index) {
	memset(vbuf, 0, sizeof(*vbuf));
	vbuf->type = buf_type;
	vbuf->memory = memory;
	vbuf->index = index;

	return enqueue_buff(vbuf);
}

bool v4ldev::streamon(enum v4l2_buf_type type) {
	return xioctl(fd, VIDIOC_STREAMON, &type) >= 0;
}

bool v4ldev::streamoff(enum v4l2_buf_type type) {
	return xioctl(fd, VIDIOC_STREAMOFF, &type) >= 0;
}

bool v4ldev::dequeue_buff(struct v4l2_buffer * vbuf) {
	return xioctl(fd, VIDIOC_DQBUF, vbuf) >= 0;
}

bool v4ldev::dequeue_buff(struct v4l2_buffer * vbuf, unsigned int buf_type, unsigned int memory, bool * again) {
	memset(vbuf, 0, sizeof(struct v4l2_buffer));
	vbuf->type = buf_type;
	vbuf->memory = memory;
	int r = dequeue_buff(vbuf);

	// if the buffer is not avaliable, try again.
	*again = (r < 0) && (errno == EAGAIN);

	return (r >= 0) || (*again);
}

bool v4ldev::query_control(struct v4l2_queryctrl * qctrl) {
	return xioctl(fd, VIDIOC_QUERYCTRL, qctrl) >= 0;
}

bool v4ldev::query_control_menu(struct v4l2_querymenu * qmenu) {
	return xioctl(fd, VIDIOC_QUERYMENU, qmenu) >= 0;
}

bool v4ldev::request_buffers(struct v4l2_requestbuffers * req_buff) {
	return xioctl(fd, VIDIOC_REQBUFS, req_buff) >= 0;
}

bool v4ldev::request_buffers(struct v4l2_requestbuffers * req_buff, int nbuffers, enum v4l2_memory memory, enum v4l2_buf_type type) {
	req_buff->reserved[0] = 0;
	req_buff->reserved[1] = 0;
	req_buff->count = nbuffers;
	req_buff->memory = memory;
	req_buff->type = type;

	return request_buffers(req_buff);
}
	
