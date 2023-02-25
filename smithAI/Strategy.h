//
// Created by zr on 23-2-25.
//

#ifndef TANK_TROUBLE_STRATEGY_H
#define TANK_TROUBLE_STRATEGY_H
#include <cstdint>

namespace TankTrouble
{
    class Tank;
    class Controller;
    class Strategy
    {
    public:
        enum StrategyType {Dodge, Contact, Attack};
        explicit Strategy(StrategyType type): _type(type){}
        [[nodiscard]] StrategyType type() const {return _type;}
        virtual bool update(Controller* ctl, Tank* tank, uint64_t globalStep) = 0;
        virtual ~Strategy() = default;

    private:
        StrategyType _type;
    };
}

#endif //TANK_TROUBLE_STRATEGY_H
