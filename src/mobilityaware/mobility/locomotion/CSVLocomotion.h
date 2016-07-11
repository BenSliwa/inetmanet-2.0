/*
 * CSVLocomotion.h
 *
 *  Created on: Jun 20, 2016
 *      Author: sliwa
 */

#ifndef CSVLOCOMOTION_H_
#define CSVLOCOMOTION_H_

#include "Locomotion.h"
#include "CSVMobilityDataAccess.h"

class CSVLocomotion : public Locomotion
{
public:
    CSVLocomotion();
    virtual ~CSVLocomotion();

    void init(CSVMobilityDataAccess *_dataAccess, ReynoldsMobilityModel *_reynolds);

    SteeringVector handleSteeringVector(SteeringVector _vector);

private:
    CSVMobilityDataAccess *p_dataAccess;
};

#endif /* CSVLOCOMOTION_H_ */
