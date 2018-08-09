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
#include "robots.hpp"
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
    static const int ALL = 10;
    static const int MIN = 0;
    static const int MAX = 1;

    // Frames
    cv::Mat in_frame, hsv_frame;
    std::vector<cv::Mat>threshold_frame;
    cv::Mat splitFrame;

    std::vector<cv::Point> advRobots;

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
    
    //Janelamento
    cv::Rect roi[7];
    bool perdeu[4];
    
    void arrumaRoi(int i);
    void searchRoi(int color);
    void verificaJanelas();
    void resetRoi(int color);

    /** Run pre-processing methods in actual frame
     */
    void preProcessing();

    /** Run pos-processing methods in actual frame
     * @brief Executes medianBlur, erode and dilate
     * @param color ball, main or secondary color
     */
    void posProcessing(int color);

    /** Make the threshold with interface values
     * @brief invokes posProcessing() and searchTags() methods
     * @param color threshold color, indicates threshold frame in vector
     */
    void segmentAndSearch(int color);

    /** Search tag in threshold frames
     * @param color frame indicator (ball, main or secondary frame)
     * @sa Vision::searchSecondaryTags
     */
    void searchTags(int color);

    /** Invokes with threads the Vision::segmentAndSearch() method.
     */
    void findTags();

    /** Give tags to the robots
     * @brief uses inSphere() method
     */
    void pick_a_tag(int color);

    /**
     * Checks if a given secondary tag belongs to our current primary tag
     * @param robot robot's vector
     * @param tempTags tag's vector
     * @param secondary actual secondary position
     * @return 0, if secondary tag isn't part of robot
     *         1, if secondary tag is in right side
     *        -1, if secondary tag is in left side
     */
    int inSphere(Robots::_Status * robot, std::vector<Tag> * tempTags, cv::Point secondary);

public:

    /** Vision's constructor
     * @param w width
     * @param h height
     */
    Vision(int w, int h);
    ~Vision();

    /** Run the vision
     * @brief Call preProcessing() method, searchTags() and pick-a-tag()
     * @param raw_frame image from camcap.hpp
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
     * @param p1 first point
     * @param p2 second point
     * @return double value with distance value
     */
    double calcDistance(cv::Point p1, cv::Point p2);

    /** Start a new video
     * @param videoName video name
     */
    void startNewVideo(std::string videoName);

    /** Writes the input frame in video object
     * @return false if camera is closed,
     *         true if frame was successful written
     */
    bool recordToVideo();

    /** Finishes video recording
     * @return true if was successful
     *         false if not
     */
    bool finishVideo();

    /** Verifies if is recording
     *
     * @return true if is recording
     *         false if isn't recording
     */
    bool isRecording();

    /** Save a picture
     * @param in_name picture's name
     */
    void savePicture(std::string in_name);

    /** Switch main color to adversary color
     * @note main and adversary color will be blue or yellow.
     */
    void switchMainWithAdv();

    /** Gets the ball position in image
     * @return cv::Point containing the ball position
     */
    cv::Point getBall();

    /** Gets a pointer containing all adversary positions
     * @return pointer containing all adversary positions
     */
    std::vector<cv::Point> getAdvs();

    /** Concatenate all frames( capture frame and threshold frames (ball, main color and secondary color))
     * @return concatenated image, with capture frame size with all frames
     */
    cv::Mat getSplitFrame();

    /** Gets threshold image
     * @param index color
     * @return threshold frame in color position
     */
    cv::Mat getThreshold(int index);

    /** Gets hue (Hsv) value
     * @param index0 color position
     * @param index1 min (0) or max (1)
     * @return hue value (min or max) from color position
     */
    int getHue(int index0, int index1);

    /** Gets saturation (hSv) value
     * @param index0 color position
     * @param index1 min (0) or max (1)
     * @return saturation value (min or max) from color position
     */
    int getSaturation(int index0, int index1);

    /** Gets value (hsV) value (hehehe)
     * @param index0 color position
     * @param index1 min (0) or max (1)
     * @return value value (min or max) from color position
     */
    int getValue(int index0, int index1);

    /** Gets erode value
     * @param index color position
     * @return erode value (min or max) from color position
     */
    int getErode(int index);

    /** Gets dilate value
     * @param index color position
     * @return dilate value from color position
     */
    int getDilate(int index);

    /** Gets blur value
     * @param index color position
     * @return blur value from color position
     */
    int getBlur(int index);

    /** Gets min area value
     * @param index color position
     * @return min area value from color position
     */
    int getAmin(int index);

    /** Sets the width and height
     * @param inWidth input width
     * @param inHeight input height
     */
    void setFrameSize(int inWidth, int inHeight);

    /** Gets frame's height
     * @return height value from image
     */
    int getFrameHeight();

    /** Gets frame's width
     * @return width value from image
     */
    int getFrameWidth();

    /** Sets hue (Hsv) value
     * @param index0 color position in vector
     * @param index1 0 to min hue value, otherwise 1 to max hue value
     * @param inValue hue value
     */
    void setHue(int index0, int index1, int inValue);

    /** Sets saturation (hSv) value
     * @param index0 color position in vector
     * @param index1 0 to min saturation value, otherwise 1 to max saturation value
     * @param inValue saturation value
     */
    void setSaturation(int index0, int index1, int inValue);

    /** Sets value (hsV) value
     * @param index0 color position in vector
     * @param index1 0 to min saturation value, otherwise 1 to max saturation value
     * @param inValue value value
     */
    void setValue(int index0, int index1, int inValue);

    /** Sets erosion value
     * @param index0 color position in vector
     * @param inValue erosion value
     */
    void setErode(int index, int inValue);

    /** Sets dilation value
     * @param index0 color position in vector
     * @param inValue dilation value
     */
    void setDilate(int index, int inValue);

    /** Sets blur value
     * @param index0 color position in vector
     * @param inValue blur value
     */
    void setBlur(int index, int inValue);

    /** Sets area min value
     * @param index0 color position in vector
     * @param inValue area min value
     */
    void setAmin(int index, int inValue);

    /**
     * @brief Loads default values to vision object, making callibration easier.
     */
    void preloadHSV();
};

#endif /* VISION_HPP_ */
