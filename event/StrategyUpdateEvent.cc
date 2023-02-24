//
// Created by zr on 23-2-21.
//

#include "StrategyUpdateEvent.h"

namespace TankTrouble
{
    StrategyUpdateEvent::StrategyUpdateEvent(Strategy* strategy):
        ev::Event("StrategyUpdateEvent", 2),
        strategy(strategy){}

    Strategy* StrategyUpdateEvent::getStrategy(){return strategy;}
}