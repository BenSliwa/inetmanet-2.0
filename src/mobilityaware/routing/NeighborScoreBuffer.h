/*
 * NeighborScoreBuffer.h
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#ifndef NEIGHBORSCOREBUFFER_H_
#define NEIGHBORSCOREBUFFER_H_

#include <deque>

class NeighborScoreBuffer
{
public:
    NeighborScoreBuffer();
    virtual ~NeighborScoreBuffer();

    void updateScore(double _score);
    double shift();
    double getMean();

    void info();

private:
    double m_currentScoreCandidate;
    std::deque<double> m_buffer;
};

#endif /* NEIGHBORSCOREBUFFER_H_ */
