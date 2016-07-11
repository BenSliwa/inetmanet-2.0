/*
 * PathScoreMetric.cpp
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#include "PathScoreMetric.h"
#include <math.h>
#include <algorithm>
#include <iostream>

Define_Module(PathScoreMetric);

PathScoreMetric::PathScoreMetric() : RoutingMetric()
{

}

PathScoreMetric::~PathScoreMetric()
{

}

void PathScoreMetric::init(int _NP, float _dMax_m)
{
    m_NP = _NP;
    m_dMax_m = _dMax_m;
}

void PathScoreMetric::initialize()
{
    m_pathTrend_max = par("pathTrend").doubleValue();
    m_alpha = par("alpha").doubleValue();
}

double PathScoreMetric::updatePathScore(double _currentScore, double _currentDistance_m, double _predictedDistance_m, double _distancePerIteration_own, double _distancePerIteration_neighbor)
{
    float pathDirection = _predictedDistance_m - _currentDistance_m;
    float linkScore = computeLinkScore(_currentDistance_m, _predictedDistance_m);
    float score = _currentScore * linkScore;

    // path trend
    if(m_NP>0)
    {
       // float maxDirectionDistance_old = m_distancePerIteration*2*m_NP;
        float maxDirectionDistance = (_distancePerIteration_own + _distancePerIteration_neighbor) * m_NP;

        pathDirection = limitAbsoluteValue(pathDirection, maxDirectionDistance);
        pathDirection = scale(pathDirection, -maxDirectionDistance, maxDirectionDistance, 1, -1);
        pathDirection *= m_pathTrend_max;

        score += pathDirection;
    }

    return score;
}

double PathScoreMetric::computeLinkScore(double _currentDistance_m, double _predictedDistance_m)
{
    float distance = limitValue(_currentDistance_m, 0, m_dMax_m);
    float predictedDistance = limitValue(_predictedDistance_m, 0, m_dMax_m);

    float currentScore = 1-pow(distance/m_dMax_m, m_alpha);
    float predictedScore = 1-pow(predictedDistance/m_dMax_m, m_alpha);

    float score = std::min(currentScore, predictedScore);
    return score;
}
