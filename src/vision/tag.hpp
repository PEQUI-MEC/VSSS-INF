/**
 * @file tag.hpp
 * @author Bryan Lincoln @ Pequi Mecânico
 * @date 28/09/2017
 * @brief A Tag Class
 * @see https://www.facebook.com/NucleoPMec/
 * @sa https://www.instagram.com/pequimecanico
 */

#ifndef TAG_HPP_
#define TAG_HPP_

#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>

#define ROBOT_RADIUS 17 // valor encontrado empiricamente (quanto menor, mais próximos os robôs podem ficar sem dar errado)

class Tag {

public:
    cv::Point position;
    cv::Point frontPoint;
    cv::Point rearPoint;

    double area;

    bool left;

    /**
     * Constroi a tag.
     * @param pos Posição da tag.
     * @param myarea Corresponde ao tagArea.
     */
    Tag(cv::Point pos, double myarea) ;

    /**
     * Seta os pontos frontPoint e rearPoint arbitrariamente.
     * @param myLine Linha encontrada pelo fitline.
     */
    void setLine(cv::Vec4f myLine);

    /**
     * Inverte os pontos frontPoint e rearPoint depois que a visão determinar a ordem correta.
     */
    void switchPoints();
};

#endif /* TAG_HPP_ */
