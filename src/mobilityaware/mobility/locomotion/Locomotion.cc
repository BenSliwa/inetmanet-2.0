/*
 * Locomotion.cc
 *
 *  Created on: Jun 14, 2016
 *      Author: mio
 */

#include <Locomotion.h>


Locomotion::Locomotion() :
    p_reynolds(0)
{
}

Locomotion::~Locomotion()
{

}

void Locomotion::init(ReynoldsMobilityModel *_reynolds)
{
    p_reynolds = _reynolds;
}
