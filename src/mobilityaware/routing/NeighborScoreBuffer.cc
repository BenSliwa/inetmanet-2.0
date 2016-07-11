/*
 * NeighborScoreBuffer.cc
 *
 *  Created on: Jun 16, 2016
 *      Author: sliwa
 */

#include "NeighborScoreBuffer.h"
#include <iostream>

NeighborScoreBuffer::NeighborScoreBuffer()
{
    int s = 8;
    for(int i=0; i<s; i++)
        m_buffer.push_back(0);
}

NeighborScoreBuffer::~NeighborScoreBuffer()
{

}

void NeighborScoreBuffer::updateScore(double _score)
{
    if(_score>m_currentScoreCandidate)
        m_currentScoreCandidate = _score;
}

double NeighborScoreBuffer::shift()
{
    m_buffer.push_front(m_currentScoreCandidate);
    m_buffer.pop_back();

    m_currentScoreCandidate = 0;

    return getMean();
}

double NeighborScoreBuffer::getMean()
{
    double mean = 0;

    if(m_buffer.size()==0)
        return m_currentScoreCandidate;
    else
    {
        for(unsigned int i=0; i<m_buffer.size(); i++)
            mean += m_buffer.at(i);

        mean = mean/(float)m_buffer.size();
    }

    return mean;
}

void NeighborScoreBuffer::info()
{
    for(int i=0; i<m_buffer.size(); i++)
    {
        std::cout << m_buffer.at(i);
        if(i<m_buffer.size()-1)
            std::cout << " - ";
    }
    std::cout << "--> " << getMean() << std::endl;
}
