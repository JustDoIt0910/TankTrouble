//
// Created by zr on 23-2-21.
//

#include "StrategyUpdateEvent.h"

namespace TankTrouble
{
    StrategyUpdateEvent::StrategyUpdateEvent(const DodgeStrategy& strategy):
        ev::Event("StrategyUpdateEvent", 2),
        strategy(strategy){}

    DodgeStrategy StrategyUpdateEvent::getStrategy(){return strategy;}
}