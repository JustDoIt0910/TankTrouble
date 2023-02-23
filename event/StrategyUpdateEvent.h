//
// Created by zr on 23-2-21.
//

#ifndef TANK_TROUBLE_STRATEGY_UPDATE_EVENT_H
#define TANK_TROUBLE_STRATEGY_UPDATE_EVENT_H

#include "reactor/Event.h"
#include "../smithAI/DodgeStrategy.h"

namespace TankTrouble
{
    class StrategyUpdateEvent : public ev::Event
    {
    public:
        explicit StrategyUpdateEvent(const DodgeStrategy& strategy);
        StrategyUpdateEvent(): StrategyUpdateEvent(DodgeStrategy(0)){}
        DodgeStrategy getStrategy();

    private:
        DodgeStrategy strategy;
    };
}
#endif //TANK_TROUBLE_STRATEGY_UPDATE_EVENT_H
