/*
 * NeighborScore.cc
 *
 *  Created on: Jul 6, 2016
 *      Author: sliwa
 */

#include "NeighborScore.h"
#include <algorithm>

NeighborScore::NeighborScore() :
    pathScoreFactor(1),
    linkScoreFactor(1),
    schedulingFactor(1),
    rssiFactor(1)
{
}

NeighborScore::~NeighborScore()
{

}

void NeighborScore::clear()
{
    pathScore = 0;
    linkScore = 0;
    scheduling = 0;
    rssi = 0;
}

void NeighborScore::updatePathScore(double _value)
{
    update(pathScore, _value);
}

void NeighborScore::update(double &_property, double _value)
{
    _property = std::max(_property, _value);
}

double NeighborScore::computeScore()
{
    double totalFactor = pathScoreFactor + linkScoreFactor + schedulingFactor + rssiFactor;
    double score = pathScore * pathScoreFactor + linkScore * linkScoreFactor + scheduling * schedulingFactor + rssi * rssiFactor;
    score /= totalFactor;

    return score;
}
