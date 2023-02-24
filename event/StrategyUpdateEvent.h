//
// Created by zr on 23-2-21.
//

#ifndef TANK_TROUBLE_STRATEGY_UPDATE_EVENT_H
#define TANK_TROUBLE_STRATEGY_UPDATE_EVENT_H

#include "reactor/Event.h"
#include "smithAI/DodgeStrategy.h"

namespace TankTrouble
{
    class StrategyUpdateEvent : public ev::Event
    {
    public:
        explicit StrategyUpdateEvent(Strategy* strategy);
        StrategyUpdateEvent(): StrategyUpdateEvent(nullptr){}
        Strategy* getStrategy();

    private:
        Strategy* strategy;
    };
}
#endif //TANK_TROUBLE_STRATEGY_UPDATE_EVENT_H
