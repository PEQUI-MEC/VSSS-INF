/**
 * @file vision.hpp
 * @author Pequi Mecânico
 * @date undefined
 * @brief Vision Class
 * @see https://www.facebook.com/NucleoPMec/
 * @sa https://www.instagram.com/pequimecanico
 */
#ifndef VISION_HPP_
#define VISION_HPP_

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/video/tracking.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include "../robot.hpp"
#include <iostream>
#include "tag.hpp"

class Vision {
private:

    // Constants
    static const int MAIN = 0;
    static const int GREEN = 1;
    // static const int PINK = 2;
    static const int BALL = 2;
    static const int ADV = 3;
    static const int MAX_ADV = 3;
    static const int TOTAL_COLORS = 4;
    static const int MIN = 0;
    static const int MAX = 1;

    // Frames
    cv::Mat in_frame, hsv_frame;
    std::vector<cv::Mat>threshold_frame;
    cv::Mat splitFrame;

    // Robots
    std::vector<Robot> robot_list;
    cv::Point advRobots[MAX_ADV];

    // Ball
    cv::Point ball;

    // TAGS
    std::vector<std::vector<Tag>> tags;

    // HSV Calibration Parameters
    int hue[5][2];
    int saturation[5][2];
    int value[5][2];
    int dilate[5];
    int erode[5];
    int blur[5];
    int areaMin[5];

    // image size
    int width;
    int height;

    // record video flag
    bool bOnAir;

    // video
    cv::VideoWriter video;

    // threads
    boost::thread_group threshold_threads;

    /** Run pre-processing methods in actual frame
     */
    void preProcessing();

    /** Run pos-processing methods in actual frame
     * @brief Executes medianBlur, erode and dilate
     * @param color ball, main or secondary color
     */
    void posProcessing(int color);

    /** Make the threshold with interface values
     * @brief invokes Vision::posProcessing() and Vision::searchTags() methods
     * @param color threshold color, indicates threshold frame in vector
     */
    void segmentAndSearch(int color);

    /** Search tag in threshold frames
     * @param color frame indicator (ball, main or secondary frame)
     * @sa Vision::searchSecondaryTags()
     */
    void searchTags(int color);

    /** Invokes with threads the Vision::segmentAndSearch() method.
     */
    void findTags();

    /** Give tags to the robots
     * @brief uses Vision::inSphere() method
     */
    void pick_a_tag();

    /**
     *
     * @param robot robot's vector
     * @param tempTags tag's vector
     * @param secondary actual secondary position
     * @return 0, if secondary tag isn't part of robot
     *         1, if secondary tag is in right side
     *        -1, if secondary tag is in left side
     */
    int inSphere(Robot * robot, std::vector<Tag> * tempTags, cv::Point secondary);

public:

    /** Vision's constructor
     * @param w width
     * @param h height
     */
    Vision(int w, int h);
    ~Vision();

    /** Run the vision
     * @brief Call Vision::preprocessing method, Vision::searchTags and Vision::pick-a-tag
     * @param raw_frame image from camcap.hh
     */
    void run(cv::Mat raw_frame);

    /** Set HSV calib parameters
     * @note receive values from interface
     * @param H hue value
     * @param S saturation value
     * @param V value
     * @param Amin area min
     * @param E erode value
     * @param D dilate value
     * @param B blur value
     */

    void setCalibParams(int H[5][2], int S[5][2], int V[5][2], int Amin[5], int E[5], int D[5], int B[5]);

    /** Calculates Euclidean distance between two points
     * @return double value with distance value
     */
    double calcDistance(cv::Point p1, cv::Point p2);

    void startNewVideo(std::string videoName);
    bool recordToVideo();
    bool finishVideo();
    bool isRecording();
    void savePicture(std::string in_name);

    /** Switch main color to adversary color
     * @note main and adversary color will be blue or yellow.
     */
    void switchMainWithAdv();

    /** Gets the ball position in image
     * @return cv::Point containing the ball position
     */
    cv::Point getBall();
    /** Obtains robot object
     * @param index robot number
     * @return robot object at index
     */
    Robot getRobot(int index);

    /** Gets robot position in image
     * @param index robot number
     * @return cv::Point containing robot position
     */
    cv::Point getRobotPos(int index);
    cv::Point getAdvRobot(int index);
    cv::Point* getAllAdvRobots();

    /** Concatenate all frames( capture frame and threshold frames (ball, main color and secondary color))
     * @return concatenated image, with capture frame size with all frames
     */
    cv::Mat getSplitFrame();

    int getRobotListSize();
  int getAdvListSize();
  cv::Mat getThreshold(int index);
  void setAllThresholds(cv::Mat input);

  int getHue(int index0, int index1);
  int getSaturation(int index0, int index1);
  int getValue(int index0, int index1);
  int getErode(int index);
  int getDilate(int index);
  int getBlur(int index);
  int getAmin(int index);

  void setFrameSize(int inWidth, int inHeight);
  int getFrameHeight();
  int getFrameWidth();

  void setHue(int index0, int index1, int inValue);
  void setSaturation(int index0, int index1, int inValue);
  void setValue(int index0, int index1, int inValue);
  void setErode(int index, int inValue);
  void setDilate(int index, int inValue);
  void setBlur(int index, int inValue);
  void setAmin(int index, int inValue);
  
  /**
   * @brief Loads default values to vision object, making callibration easier.
   */
  void preloadHSV();
};

#endif /* VISION_HPP_ */
