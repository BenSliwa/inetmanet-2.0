/*
 * CSVSteering.h
 *
 *  Created on: Jun 20, 2016
 *      Author: sliwa
 */

#ifndef CSVSTEERING_H_
#define CSVSTEERING_H_

#include "Steering.h"
#include "CSVMobilityDataAccess.h"

class CSVSteering : public Steering
{
public:
    CSVSteering();
    virtual ~CSVSteering();

    virtual void initialize();

    void init(CSVMobilityDataAccess *_dataAccess, ReynoldsMobilityModel *_reynolds);

    SteeringVector update();

private:
    CSVMobilityDataAccess *p_dataAccess;
};

#endif /* CSVSTEERING_H_ */
