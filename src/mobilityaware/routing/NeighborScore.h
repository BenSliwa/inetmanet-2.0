/*
 * NeighborScore.h
 *
 *  Created on: Jul 6, 2016
 *      Author: sliwa
 */

#ifndef NEIGHBORSCORE_H_
#define NEIGHBORSCORE_H_

class NeighborScore {
public:
    NeighborScore();
    virtual ~NeighborScore();


    void clear();
    void updatePathScore(double _value);
    void update(double &_property, double _value);

    double computeScore();


    double pathScore;
    double linkScore;
    double scheduling;
    double rssi;


    double pathScoreFactor;
    double linkScoreFactor;
    double schedulingFactor;
    double rssiFactor;

};

#endif /* NEIGHBORSCORE_H_ */
