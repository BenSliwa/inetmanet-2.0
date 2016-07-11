/*
 * PathScoreMetric.h
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#ifndef PATHSCOREMETRIC_H_
#define PATHSCOREMETRIC_H_

#include "RoutingMetric.h"

class PathScoreMetric : public RoutingMetric
{
public:
    PathScoreMetric();
    virtual ~PathScoreMetric();


    void init(int _NP, float _dMax_m);


    double updatePathScore(double _currentScore, double _currentDistance_m, double _predictedDistance_m, double _distancePerIteration_own, double _distancePerIteration_neighbor);
    double computeLinkScore(double _currentDistance_m, double _predictedDistance_m);

protected:
    virtual void initialize();

private:
    float m_pathTrend_max;
    int m_NP;

    float m_alpha;
    float m_dMax_m;
};

#endif /* PATHSCOREMETRIC_H_ */
