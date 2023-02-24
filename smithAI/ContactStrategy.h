//
// Created by zr on 23-2-25.
//

#ifndef TANK_TROUBLE_CONTACT_STRATEGY_H
#define TANK_TROUBLE_CONTACT_STRATEGY_H
#include "Strategy.h"
#include "AStar.h"
#include "Object.h"

namespace TankTrouble
{
    class ContactStrategy : public Strategy
    {
    public:
        explicit ContactStrategy(AStar::AStarResult route):
            Strategy(Strategy::Contact),
            route(std::move(route)),
            next(1){}

        bool update(Controller* ctl, Tank* tank, uint64_t globalStep) override;

    private:
        AStar::AStarResult route;
        int next;
        Object::PosInfo prevPos;
    };
};

#endif //TANK_TROUBLE_CONTACT_STRATEGY_H
