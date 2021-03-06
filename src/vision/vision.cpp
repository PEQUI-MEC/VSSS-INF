#include "vision.hpp"
#include "../aux/debug.hpp"

void Vision::run(cv::Mat raw_frame) {
    in_frame = raw_frame.clone();
    if (bOnAir) recordToVideo();
    preProcessing();
    findTags();
    pick_a_tag();
}

void Vision::preProcessing() {
    cv::cvtColor(in_frame,hsv_frame,cv::COLOR_RGB2HSV);
}

void Vision::findTags() {
    for (int i = 0; i < TOTAL_COLORS; i++)
        threshold_threads.add_thread(new boost::thread(&Vision::segmentAndSearch, this, i));

    threshold_threads.join_all();
}

void Vision::segmentAndSearch(int color) {
    cv::Mat frame = hsv_frame.clone();

    inRange(frame,cv::Scalar(hue[color][MIN],saturation[color][MIN],value[color][MIN]),
    cv::Scalar(hue[color][MAX],saturation[color][MAX],value[color][MAX]),threshold_frame.at(color));

    posProcessing(color);
    searchTags(color);
}

void Vision::posProcessing(int color) {
    cv::Mat erodeElement = cv::getStructuringElement( cv::MORPH_RECT,cv::Size(3,3));
    cv::Mat dilateElement = cv::getStructuringElement( cv::MORPH_RECT,cv::Size(3,3));

    cv::medianBlur(threshold_frame.at(color), threshold_frame.at(color), blur[color]);
    cv::erode(threshold_frame.at(color),threshold_frame.at(color),erodeElement,cv::Point(-1,-1),erode[color]);
    cv::dilate(threshold_frame.at(color),threshold_frame.at(color),dilateElement,cv::Point(-1,-1),dilate[color]);
}

void Vision::searchTags(int color) {
    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    tags.at(color).clear();

    cv::findContours(threshold_frame.at(color),contours,hierarchy,cv::RETR_CCOMP,cv::CHAIN_APPROX_NONE);

    for(int i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if(area >= areaMin[color]) {
            cv::Moments moment = moments(cv::Mat(contours[i]));

            // seta as linhas para as tags principais do pick-a-tag
            if(color == MAIN) {
                tags.at(color).push_back(Tag(cv::Point(moment.m10/area, moment.m01/area), area));

                // tem que ter jogado a tag no vetor antes de mexer nos valores dela
                cv::Vec4f line;
                cv::fitLine(cv::Mat(contours[i]),line,2,0,0.01,0.01);
                tags.at(color).at(tags.at(color).size() - 1).setLine(line);
            } else if(color == ADV) {
                if(tags.at(color).size() >= 3) {
                    // pega o menor índice
                    int menor = 0;

                    for(int j = 1; j < 3; j++) {
                        if(tags.at(color).at(j).area < tags.at(color).at(menor).area)
                        menor = j;
                    }

                    if(menor < tags.at(color).size() && area > tags.at(color).at(menor).area)
                    tags.at(color).at(menor) = Tag(cv::Point(moment.m10/area, moment.m01/area), area);
                } else {
                  tags.at(color).push_back(Tag(cv::Point(moment.m10/area, moment.m01/area), area));
                }
            } else {
                tags.at(color).push_back(Tag(cv::Point(moment.m10/area, moment.m01/area), area));
            }
        }
    }
}

void Vision::pick_a_tag() {
    // OUR ROBOTS
    for(unsigned short i = 0; i < tags.at(MAIN).size() && i<3; i++) {
        std::vector<Tag> tempTags;

        Robots::_Status robot;
        // Posição do robô
        robot.position = (cv::Point_<int>) tags.at(MAIN).at(i).position;

        // Cálculo da orientação de acordo com os pontos rear e front
        robot.orientation = atan2((tags.at(MAIN).at(i).frontPoint.y - robot.position.y) * 1.3 / height,
                                  (tags.at(MAIN).at(i).frontPoint.x - robot.position.x) * 1.5 / width);

        // Armazena a tag
        tempTags.push_back(tags.at(MAIN).at(i));

        // Para cada tag principal, verifica quais são as secundárias correspondentes
        for (unsigned j = 0;
             j < tags.at(GREEN).size(); j++) { // (não usei unsigned short pois pode ter muitas tags verdes)
            // já faz a atribuição verificando se o valor retornado é 0 (falso); além disso, altera a orientação caso esteja errada
            int tmpSide = inSphere(&robot, &tempTags, tags.at(GREEN).at(j).position);
            if (tmpSide) {
                tags.at(GREEN).at(j).left = tmpSide > 0;
                // calculos feitos, joga tag no vetor
                tempTags.push_back(tags.at(GREEN).at(j));

                // identifica se já tem mais de duas tags (amarela e uma verde.)
                if (tempTags.size() > 2) {
                    break;
                }
            }
        }

        // Dá nome aos bois (robôs)
        if (tempTags.size() > 2) { // se tem três tags, é o full pick-a
            Robots::set_position(2, robot.position);
            Robots::set_orientation(2, robot.orientation);
            Robots::set_secondary_tag(2, tempTags.at(0).frontPoint);
            //robot_list.at(2).rearPoint = tempTags.at(0).rearPoint;
        } else if (tempTags.size() > 1) { // não classifica se não tiver encontrado pelo menos uma verde
            if (tempTags.at(1).left) {
                Robots::set_position(0, robot.position);
                Robots::set_orientation(0, robot.orientation);
                Robots::set_secondary_tag(0, tempTags.at(0).frontPoint);
            } else {
                Robots::set_position(1, robot.position);
                Robots::set_orientation(1, robot.orientation);
                Robots::set_secondary_tag(1, tempTags.at(0).frontPoint);
            }
        }
    } // OUR ROBOTS

    // ADV ROBOTS
    advRobots.clear();
    for(int i = 0; i < tags.at(ADV).size(); i++) {
        advRobots.push_back(tags.at(ADV).at(i).position);
    }
    // BALL POSITION
    if (!tags[BALL].empty())
        ball = tags.at(BALL).at(0).position;
}

int Vision::inSphere(Robots::_Status * robot, std::vector<Tag> * tempTags, cv::Point secondary) {
    // se esta secundária faz parte do robô
    if(calcDistance(robot->position, secondary) <= ROBOT_RADIUS) {
        if(calcDistance(tempTags->at(0).frontPoint, secondary) < calcDistance(tempTags->at(0).rearPoint, secondary)) {
            tempTags->at(0).switchPoints();
            // recalcula a orientação com os novos pontos (isso só é feito uma vez em cada robô, se necessário)
            robot->orientation = atan2((tempTags->at(0).frontPoint.y-robot->position.y)*1.3/height,(tempTags->at(0).frontPoint.x-robot->position.x)*1.5/width);
        }

        double secSide = atan2((secondary.y-robot->position.y)*1.3/height,(secondary.x-robot->position.x)*1.5/width);

        // Cálculo do ângulo de orientação para diferenciar robôs de mesma cor
        return (atan2(sin(secSide-robot->orientation+3.1415), cos(secSide-robot->orientation+3.1415))) > 0 ? 1 : -1;
    }

    return 0;
}

double Vision::calcDistance(cv::Point p1, cv::Point p2) {
    return sqrt(pow(p1.x-p2.x,2) + pow(p1.y-p2.y,2));
}

void Vision::switchMainWithAdv() {
    int tmp;

    for(int i = MIN; i <= MAX; i++) {
        tmp = hue[MAIN][i];
        hue[MAIN][i] = hue[ADV][i];
        hue[ADV][i] = tmp;

        tmp = saturation[MAIN][i];
        saturation[MAIN][i] = saturation[ADV][i];
        saturation[ADV][i] = tmp;

        tmp = value[MAIN][i];
        value[MAIN][i] = value[ADV][i];
        value[ADV][i] = tmp;
    }

    tmp = areaMin[MAIN];
    areaMin[MAIN] = areaMin[ADV];
    areaMin[ADV] = tmp;

    tmp = erode[MAIN];
    erode[MAIN] = erode[ADV];
    erode[ADV] = tmp;

    tmp = dilate[MAIN];
    dilate[MAIN] = dilate[ADV];
    dilate[ADV] = tmp;

    tmp = blur[MAIN];
    blur[MAIN] = blur[ADV];
    blur[ADV] = tmp;
}

cv::Mat Vision::getSplitFrame() {
    cv::Mat horizontal[2], vertical[2];

    for (int index = 0; index < 3; index++)
        cv::cvtColor(threshold_frame.at(index), threshold_frame.at(index), cv::COLOR_GRAY2RGB);


    cv::pyrDown(in_frame, vertical[0]);
    cv::pyrDown(threshold_frame.at(0), vertical[1]);

    cv::hconcat(vertical, 2, horizontal[0]);

    cv::pyrDown(threshold_frame.at(1), vertical[0]);
    cv::pyrDown(threshold_frame.at(2), vertical[1]);

    cv::hconcat(vertical, 2, horizontal[1]);

    cv::vconcat(horizontal, 2, splitFrame);

    // cv::imwrite("teste.png", splitFrame);

    return splitFrame;
}

void Vision::setCalibParams(int H[5][2], int S[5][2], int V[5][2], int Amin[5], int E[5], int D[5], int B[5]) {
    for (int i = 0; i < 5; i++) {
        areaMin[i] = Amin[i];
        erode[i] = E[i];
        dilate[i] = D[i];
        blur[i] = B[i];
        for (int j = 0; j < 2; j++) {
            hue[i][j] = H[i][j];
            saturation[i][j] = S[i][j];
            value[i][j] = V[i][j];
        }
    }
}

void Vision::savePicture(std::string in_name) {
    cv::Mat frame = in_frame.clone();
    std::string picName = "media/pictures/" + in_name + ".png";

    cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);
    cv::imwrite(picName, frame);
}

void Vision::startNewVideo(std::string in_name) {
    std::string videoName = "media/videos/" + in_name + ".avi";

    video.open(videoName, cv::VideoWriter::fourcc('M','J','P','G'), 30, cv::Size(width,height));
    debug_log("Started a new video recording.");
    bOnAir = true;
}

bool Vision::recordToVideo() {
    if (!video.isOpened()) return false;

    cv::Mat frame = in_frame.clone();
    cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);

    video.write(frame);

    return true;
}

bool Vision::finishVideo() {
    if (!video.isOpened()) return false;

    debug_log("Finished video recording.");
    video.release();
    bOnAir = false;

    return true;
}

bool Vision::isRecording() {
    return bOnAir;
}

cv::Point Vision::getBall() {
    return ball;
}

cv::Mat Vision::getThreshold(int index) {
    cv::cvtColor(threshold_frame.at(index), threshold_frame.at(index), cv::COLOR_GRAY2RGB);
    return threshold_frame.at(index);
}

int Vision::getHue(int index0, int index1) {
    return hue[index0][index1];
}

int Vision::getSaturation(int index0, int index1) {
    return saturation[index0][index1];
}

int Vision::getValue(int index0, int index1) {
    return value[index0][index1];
}

int Vision::getErode(int index) {
    return erode[index];
}

int Vision::getDilate(int index) {
    return dilate[index];
}

int Vision::getBlur(int index) {
    return blur[index];
}

int Vision::getAmin(int index) {
    return areaMin[index];
}

void Vision::setHue(int index0, int index1, int inValue) {
    if(index0 >= 0 && index0 < TOTAL_COLORS && (index1 == 0 || index1 == 1)) {
        hue[index0][index1] = inValue;
    } else {
        debug_error("Vision:setHue: could not set (invalid index)");
    }
}

void Vision::setSaturation(int index0, int index1, int inValue) {
    if(index0 >= 0 && index0 < TOTAL_COLORS && (index1 == 0 || index1 == 1)) {
        saturation[index0][index1] = inValue;
    } else {
        debug_error("Vision:setSaturation: could not set (invalid index)");
    }
}

void Vision::setValue(int index0, int index1, int inValue) {
  if(index0 >= 0 && index0 < TOTAL_COLORS && (index1 == 0 || index1 == 1)) {
      value[index0][index1] = inValue;
  } else{
      debug_error("Vision:setValue: could not set (invalid index)");
  }
}

void Vision::setErode(int index, int inValue) {
    if(index >= 0 && index < TOTAL_COLORS) {
        erode[index] = inValue;
    } else {
        debug_error("Vision:setErode: could not set (invalid index)");
    }
}

void Vision::setDilate(int index, int inValue) {
    if(index >= 0 && index < TOTAL_COLORS) {
        dilate[index] = inValue;
    } else {
        debug_error("Vision:setDilate: could not set (invalid index)");
    }
}

void Vision::setBlur(int index, int inValue) {
    if(index >= 0 && index < TOTAL_COLORS) {
        blur[index] = inValue;
    } else {
        debug_error("Vision:setBlur: could not set (invalid index)");
    }
}

void Vision::setAmin(int index, int inValue) {
    if(index >= 0 && index < TOTAL_COLORS) {
        areaMin[index] = inValue;
    } else {
        debug_error("Vision:setAmin: could not set (invalid index)");
    }
}

void Vision::setFrameSize(int inWidth, int inHeight) {
    if (inWidth >= 0) width = inWidth;
    if (inHeight >= 0) height = inHeight;
}

int Vision::getFrameHeight() {
    return height;
}

int Vision::getFrameWidth() {
    return width;
}

std::vector<cv::Point> Vision::getAdvs() {
    return advRobots;
}

void Vision::preloadHSV() {
    for(int i = 0; i < 4; i++) {
        setBlur(i, 5);
    }
    setHue(0, 0, 20);
    setHue(0, 1, 30);
    setSaturation(0, 0, 50);
    setValue(0, 0, 150);
    setErode(0, 1);
    setAmin(0, 70);

    setHue(1, 0, 31);
    setHue(1, 1, 60);
    setSaturation(1, 0, 45);
    setValue(1, 0, 150);
    setAmin(1, 40);

    setHue(2, 0, 1);
    setHue(2, 1, 15);
    setSaturation(2, 0, 70);
    setValue(2, 0, 130);
    setAmin(2, 50);

    setHue(3, 0, 55);
    setHue(3, 1, 150);
    setSaturation(3, 0, 50);
    setValue(3, 0, 150);
    setErode(3, 1);
    setAmin(3, 70);
}

Vision::Vision(int w, int h) : width(w), height(h), bOnAir(false) {
    // Variables Init
    cv::Mat mat;
    std::vector<Tag> tagVec;

    for(int i = 0; i < TOTAL_COLORS; i++) {
        threshold_frame.push_back(mat);
        tags.push_back(tagVec);
    }
}

Vision::~Vision() {}
