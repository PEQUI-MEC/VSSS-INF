/**
 * @file v4l_device.hpp
 * @author Dr. Gustavo Teodoro Laureano
 * @date Jan 27, 2014
 * @brief Capture library that supports camera capture on linux systems.
 * @see 
 * @se
 */

// Must be included before #ifndef directive
// This makes our compiler include our debug namespace directives into this file
#include "../aux/debug.hpp"

#ifndef V4LCAP_HPP_
#define V4LCAP_HPP_

#include <linux/videodev2.h>
#include <libv4lconvert.h>

class v4lcap {
public:

	enum memory_type {
		MEM_READ, MEM_MMAP, MEM_USER
	};
	
	int fd;
	bool emulate_format;
	bool needs_convert;

	/**
	 * @brief Video format that the cam's video should be returned, if not null.
	 * If format is not supported, this will change to another one.
	 * @format_src Gets the format from de camera.
	 * @format_dest Destination format of the video.
	 */
	struct v4l2_format format_src, format_dest;

	/**
	 * @brief Data obtained from camera.
	 */
	struct v4lconvert_data * v4lconv;

	/**
	 * @brief 
	 * 
	 */
	struct v4l2_requestbuffers reqbuf;

	enum device_status {
		OPENED = 1 << 0, MMAP_INIT = 1 << 1, STREAM_ON = 1 << 2
	};

	unsigned int status;

	struct buffer {
			void * start;
			long int length;
	};

	struct buffer_holder {
			int qtd;
			struct buffer * buffs;
	};

	struct buffer_holder buffers;

	int xioctl(int fd, int request, void * arg);

	v4lcap();

	~v4lcap();

	/**
	 * @brief Search and returns the path name of device that could be opened in the host by.
	 * 
	 * @param device Path name of the device that will be filled inside the method.
	 * @param ndev_max Max number of devices (cameras) on host.
	 * @param init Boolean trigger the method.
	 * @return true Camera was found and the name will be filled inside 'device' param.
	 * @return false Camera was not found.
	 */
	static bool enum_device_name(char device[16], int ndev_max, bool init = false);

	/**
	 * @brief By getting device name and emulated format, this method open the connection to the camera.
	 * 
	 * @param devname Name of requested device.
	 * @param emulate_fmt Boolean that sets up if camera will be emulated using libv4l2 wrapper.
	 * @return true If device was open and ready to use.
	 * @return false If device cannot be opened or device is not compatible with de library.
	 */
	bool open_device(const char * devname, bool emulate_fmt);

	/**
	 * @brief This method starts capturing and streaming from opened device passing memory type and number of buffers used.
	 * 
	 * @param memory_type Defines memory type.
	 * @param nbuffers Number of buffers used to streaming from camera.
	 * @return true Streaming from camera is openned.
	 * @return false Streaming cannot be started.
	 */
	bool start_capturing(enum memory_type mem_type, int nbuffers);

	/**
	 * @brief This method starts capturing and streaming from opened device.
	 * 
	 * @return true Streaming from camera is openned.
	 * @return false Streaming cannot be started.
	 */
	bool start_capturing();

	/**
	 * @brief This method requests buffers from host system to allocate streaming from camera.
	 * 
	 * @param nbuffers Number of buffers requested.
	 * @return true Buffers successfully allocated.
	 * @return false If cannot allocate buffers.
	 */
	bool init_mmap(int nbuffers);

	/* configuration */
	/**
	 * @brief Set the input object
	 * 
	 * @param index ID of selected camera
	 * @return true If querying is capable
	 * @return false If querying is not capable
	 */
	bool set_input(int index);

	/**
	 * @brief Set the standard object
	 * 
	 * @param std_id This type is a set, each bit representing another video standard.
	 * @see https://www.linuxtv.org/downloads/v4l-dvb-apis-old/vidioc-enumstd.html#v4l2-std-id
	 * @return true If querying is capable
	 * @return false If querying is not capable
	 */
	bool set_standard(v4l2_std_id std_id);

	/**
	 * @brief Set the pixel format object
	 * 
	 * @param pixel_format Pixel format param to set on capturing
	 * @return true If format is set
	 * @return false If cannot set the format
	 */
	bool set_pixel_format(unsigned int pixel_format);

	/**
	 * @brief Set the frame size object
	 * 
	 * @param width Frame width
	 * @param height Frame height
	 * @param type Frame type
	 * @return true If frame is set
	 * @return false if cannot set frame
	 */
	bool set_frame_size(int width, int height, unsigned int type);

	/**
	 * @brief Set the frame interval object
	 * 
	 * @param fract Time fraction to be set
	 * @return true If time interval is set
	 * @return false If cannot set time interval
	 */
	bool set_frame_interval(struct v4l2_fract & fract);

	/**
	 * @brief Set the control object
	 * 
	 * @param ctrl_id 
	 * @param value 
	 * @return true 
	 * @return false 
	 */
	bool set_control(int ctrl_id, int value);

	/**
	 * @brief Query stream to initiate
	 * 
	 * @param v4l2_buf_type Buffer type of streaming
	 * @return true If all succeeds
	 * @return false If something goes wrong with stream query
	 */
	bool streamon(enum v4l2_buf_type type);
	bool streamon(int type);

	/**
	 * @brief 
	 * 
	 * @param raw 
	 * @return void* 
	 */
	void * grab_raw(unsigned char * raw);
	void * grab_rgb(unsigned char * rgb);

	/**
	 * @brief Query stream to terminate
	 * 
	 * @param v4l2_buf_type Buffer type off running stream
	 * @return true If all succeds
	 * @return false If something goes wrong with query
	 */
	bool streamoff(enum v4l2_buf_type type);
	bool streamoff(int type);

	/**
	 * @brief Closes capturing query
	 * 
	 * @return true Closed successfully
	 * @return false Something goes wrong
	 */
	bool stop_capturing();

	/**
	 * @brief Free all requested buffers
	 * 
	 * @return true Free all buffers
	 * @return false Nothing freed =,(
	 */
	bool uninit_mmap();

	/**
	 * @brief Close camera communication with pc
	 * 
	 * @return true If close goes right
	 * @return false Can't close webcam communication
	 */
	bool close_device();

	/**
	 * @brief Verify if camera is open
	 * 
	 * @return true Camera is open
	 * @return false Camera is not open
	 */
	bool is_opened();

	/**
	 * @brief Verify query lenght capability
	 * 
	 * @param cap 
	 * @return true 
	 * @return false 
	 */
	bool query_capability(struct v4l2_capability * cap);

	/**
	 * @brief Get the format object
	 * 
	 * @param fmt 
	 * @param type 
	 * @return true 
	 * @return false 
	 */
	bool get_format(struct v4l2_format * fmt, unsigned int type);
	bool get_format(struct v4l2_format * fmt);

	/**
	 * @brief Get the input index object
	 * 
	 * @param input 
	 * @return true 
	 * @return false 
	 */
	bool get_input_index(unsigned int * input);

	/**
	 * @brief Get the input object
	 * 
	 * @param input 
	 * @return true 
	 * @return false 
	 */
	bool get_input(struct v4l2_input * input);

	/**
	 * @brief Get the standard id object
	 * 
	 * @param std_id This type is a set, each bit representing another video standard.
	 * @see https://www.linuxtv.org/downloads/v4l-dvb-apis-old/vidioc-enumstd.html#v4l2-std-id
	 * @return true 
	 * @return false 
	 */
	bool get_standard_id(v4l2_std_id * std_id);

	/**
	 * @brief Get the standard object
	 * 
	 * @param standard 
	 * @return true 
	 * @return false 
	 */
	bool get_standard(struct v4l2_standard * standard);

	/**
	 * @brief Get the stream parameter object
	 * 
	 * @param streamparm 
	 * @param type 
	 * @return true 
	 * @return false 
	 */
	bool get_stream_parameter(struct v4l2_streamparm * streamparm, unsigned int type);

	/**
	 * @brief Get the control object
	 * 
	 * @param ctrl 
	 * @param ctrl_id 
	 * @return true 
	 * @return false 
	 */
	bool get_control(struct v4l2_control * ctrl, unsigned int ctrl_id);

	/**
	 * @brief Query control options
	 * 
	 * @param qctrl 
	 * @return true 
	 * @return false 
	 */
	bool query_control(struct v4l2_queryctrl * qctrl);

	/**
	 * @brief Query control menu
	 * 
	 * @param qmenu 
	 * @return true 
	 * @return false 
	 */
	bool query_control_menu(struct v4l2_querymenu * qmenu);

	/**
	 * @brief Request buffers allocation from SO
	 * 
	 * @param req_buff 
	 * @param nbuffers 
	 * @param v4l2_memory 
	 * @param v4l2_buf_type 
	 * @return true 
	 * @return false 
	 */
	bool request_buffers(struct v4l2_requestbuffers * req_buff, int nbuffers, enum v4l2_memory memory, enum v4l2_buf_type type);
	bool request_buffers(struct v4l2_requestbuffers * req_buff);
	bool request_buffers_mmap(struct v4l2_requestbuffers * req_buff, int nbuffers);

	/**
	 * @brief Query buffers to camera
	 * 
	 * @param buffer 
	 * @return true 
	 * @return false 
	 */
	bool query_buffer(struct v4l2_buffer * buffer);
	bool query_buffer(struct v4l2_buffer * buf, unsigned int type, unsigned int memory, unsigned int index);
	bool query_buffer(struct v4l2_buffer * buf, enum v4l2_buf_type type, enum v4l2_memory memory, unsigned int index);

	/**
	 * @brief Enqueue requested buffers and set capture params
	 * 
	 * @param vbuf 
	 * @return true 
	 * @return false 
	 */
	bool enqueue_buff(struct v4l2_buffer * vbuf);
	bool enqueue_buff(struct v4l2_buffer * vbuf, enum v4l2_buf_type buf_type, enum v4l2_memory memory, int index);
	bool enqueue_buff(struct v4l2_buffer * vbuf, unsigned int buf_type, unsigned int memory, int index);

	/**
	 * @brief Dequeue buffers allocated
	 * 
	 * @param vbuf 
	 * @return true 
	 * @return false 
	 */
	bool dequeue_buff(struct v4l2_buffer * vbuf);
	bool dequeue_buff(struct v4l2_buffer * vbuf, enum v4l2_buf_type buf_type, enum v4l2_memory memory, bool * again);
	bool dequeue_buff(struct v4l2_buffer * vbuf, unsigned int buf_type, unsigned int memory, bool * again);

	/**
	 * @brief Get current format of data, request buffers from mmap, allocate buffers and map memory allocated
	 * 
	 * @param req_buff 
	 * @return true 
	 * @return false 
	 */
	bool init_mmap(struct v4l2_requestbuffers * req_buff);

	/**
	 * @brief Enumerate audio inputs
	 * 
	 * @param audio 
	 * @param index 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_audio_input(struct v4l2_audio * audio, int index = 0, bool init = false);

	/**
	 * @brief Enumerate audio outputs
	 * 
	 * @param audiout 
	 * @param index 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_audio_output(struct v4l2_audioout * audiout, int index = 0, bool init = false);

	/**
	 * @brief Enumerate supported Digital Video timings
	 * 
	 * @param ... 
	 * @return true 
	 * @return false 
	 */
	bool enum_dv_timings(...);
	
	/**
	 * @brief Enumerate image formats
	 * 
	 * @param fd 
	 * @param fmtdesc 
	 * @return true 
	 * @return false 
	 */
	bool enum_image_format(int fd, struct v4l2_fmtdesc * fmtdesc);
	bool enum_image_format(struct v4l2_fmtdesc * fmtdesc, int index = 0, bool init = false);

	/**
	 * @brief Enumerate frame sizes
	 * 
	 * @param frmsize 
	 * @param pixel_format 
	 * @param index 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_frame_size(struct v4l2_frmsizeenum * frmsize, int pixel_format, int index = 0, bool init = false);

	/**
	 * @brief enumerate frame intervals
	 * 
	 * @param frmi 
	 * @param pixel_format 
	 * @param width 
	 * @param height 
	 * @param index 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_frame_interval(struct v4l2_frmivalenum * frmi, int pixel_format, int width, int height, int index = 0, bool init = false);
	
	/**
	 * @brief Enumerate supported frequency bands
	 * 
	 * @param ... 
	 * @return true 
	 * @return false 
	 */
	bool enum_video_freq_bands(...);

	/**
	 * @brief Enumerate video inputs
	 * 
	 * @param input 
	 * @param index 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_video_input(struct v4l2_input * input, int index = 0, bool init = false);

	/**
	 * @brief Enumerate video outputs
	 * 
	 * @param output 
	 * @param index 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_video_output(struct v4l2_output * output, int index = 0, bool init = false);

	/**
	 * @brief Enumerate supported video
	 * 
	 * @param std 
	 * @param index 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_video_standard(struct v4l2_standard * std, int index = 0, bool init = false);

	/**
	 * @brief 
	 * 
	 * @param qctrl 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_control_user_base(struct v4l2_queryctrl * qctrl, bool init = false);

	/**
	 * @brief 
	 * 
	 * @param qctrl 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_control_private_base(struct v4l2_queryctrl * qctrl, bool init = false);

	/**
	 * @brief 
	 * 
	 * @param qctrl 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_control_default(struct v4l2_queryctrl * qctrl, bool init = false);

	/**
	 * @brief 
	 * 
	 * @return true 
	 * @return false 
	 */
	bool show_controls();

	/**
	 * @brief Fills menu controll with values
	 * 
	 * @param qmenu Pointer to menu
	 * @param ctrl 
	 * @param index 
	 * @param init 
	 * @return true 
	 * @return false 
	 */
	bool enum_control_menu(struct v4l2_querymenu * qmenu, v4l2_queryctrl & ctrl, int index = 0, bool init = false);

	bool set_format(struct v4l2_format * fmt);
	//bool set_format(int pixel_format, int width, int height);

};

#endif /* V4LCAP_HPP_ */
