//
// Created by zr on 23-2-25.
//

#include "AttackStrategy.h"
#include "Tank.h"
#include "Controller.h"
#include "util/Math.h"

namespace TankTrouble
{
    bool AttackStrategy::update(Controller* ctl, Tank* tank, uint64_t globalStep)
    {
        if(done) return false;
        Object::PosInfo cur = tank->getCurrentPosition();
        if(std::abs(cur.angle - attackingPos.angle) < 4)
        {
            tank->rotateCW(false);
            tank->rotateCCW(false);
            ctl->fire(tank);
            done = true;
            return false;
        }
        else
        {
            util::Vec vAttack = util::getUnitVector(attackingPos.angle);
            util::Vec vt = util::getUnitVector(cur.angle);
            if(vt.cross(vAttack) > 0)
                tank->rotateCW(true);
            else tank->rotateCCW(true);
        }
        return true;
    }

    void AttackStrategy::cancelAttack() {done = true;}
}