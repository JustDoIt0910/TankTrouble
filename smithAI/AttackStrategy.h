//
// Created by zr on 23-2-25.
//

#ifndef TANK_TROUBLE_ATTACK_STRATEGY_H
#define TANK_TROUBLE_ATTACK_STRATEGY_H
#include "Strategy.h"
#include "Object.h"

namespace TankTrouble
{
    class AttackStrategy : public Strategy
    {
    public:
        explicit AttackStrategy(const Object::PosInfo& pos):
                Strategy(Strategy::Attack),
                attackingPos(pos),
                done(false){}

        bool update(Controller* ctl, Tank* tank, uint64_t globalStep) override;
        void cancelAttack();

    private:
        Object::PosInfo attackingPos;
        bool done;
    };
};

#endif //TANK_TROUBLE_ATTACK_STRATEGY_H
