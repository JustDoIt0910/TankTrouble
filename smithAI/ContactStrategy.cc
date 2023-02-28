//
// Created by zr on 23-2-25.
//

#include <iostream>
#include "ContactStrategy.h"
#include "Tank.h"
#include "util/Math.h"
#include "Controller.h"
#include "AgentSmith.h"

namespace TankTrouble
{
    bool ContactStrategy::update(Controller* ctl, Tank* tank, uint64_t globalStep)
    {
        bool isRotating = tank->isRotatingCW() || tank->isRotatingCCW();
        bool isForwarding = tank->isForwarding();
        tank->stop();
        if(next >= route.size())
            return false;
        Object::PosInfo tankPos = tank->getCurrentPosition();
        int nextX = route[next].first;
        int nextY = route[next].second;
        util::Vec gridCenter(MAP_A_STAR_X_TO_REAL_X(nextX), MAP_A_STAR_Y_TO_REAL_Y(nextY));
        if(util::distanceOfTwoPoints(tankPos.pos, gridCenter) < 15)
            next++;
        util::Vec vn = gridCenter - tankPos.pos;
        util::Vec vt = util::getUnitVector(tankPos.angle);
        if(util::angleBetweenVectors(vt, vn) < 45.0)
            tank->forward(true);
        if(util::angleBetweenVectors(vt, vn) >= 12.0 && !(isRotating && tankPos.angle == prevPos.angle))
        {
            if(vt.cross(vn) >= 0)
                tank->rotateCW(true);
            else
                tank->rotateCCW(true);
        }
        if(isForwarding && tankPos.pos == prevPos.pos)
        {
            Object::PosInfo tryPos = Tank::getNextPosition(tankPos, ROTATING_CW, 0, 0);
            if(ctl->checkTankBlockCollision(tryPos, tryPos) == 0)
                tank->rotateCW(true);
            else tank->rotateCCW(true);
            tank->forward(false);
        }
        if(tankPos == prevPos)
        {
            stuckSteps++;
            if(stuckSteps > 5)
            {
                tank->rotateCW(false);
                tank->rotateCCW(false);
                tank->backward(true);
            }
        }
        else stuckSteps = 0;
        prevPos = tankPos;
        if(!ctl->smith->safeToMove(globalStep, tankPos, tank->getMovingStatus()))
            tank->stop();
        return true;
    }
}