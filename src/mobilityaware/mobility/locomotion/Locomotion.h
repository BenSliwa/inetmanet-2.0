/*
 * Locomotion.h
 *
 *  Created on: Jun 14, 2016
 *      Author: sliwa
 */

#ifndef LOCOMOTION_H_
#define LOCOMOTION_H_

#include "ReynoldsMobilityModel.h"
#include "SteeringVector.h"
#include "csimplemodule.h"

class Locomotion : public cSimpleModule
{
public:
    Locomotion();
    virtual ~Locomotion();

    void init(ReynoldsMobilityModel *_reynolds);

    virtual SteeringVector handleSteeringVector(SteeringVector _vector) = 0;

protected:
    ReynoldsMobilityModel *p_reynolds;
};

#endif /* LOCOMOTION_H_ */
