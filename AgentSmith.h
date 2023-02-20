//
// Created by zr on 23-2-20.
//

#ifndef TANK_TROUBLE_AGENT_SMITH_H
#define TANK_TROUBLE_AGENT_SMITH_H
#include <utility>
#include <vector>
#include <unordered_map>
#include "util/Vec.h"
#include "Object.h"

namespace TankTrouble
{
    class Block;
    class Controller;

    class AgentSmith
    {
    public:
        typedef std::unordered_map<int, std::vector<util::Vec>> Ballistics;
        typedef std::unordered_map<int, Object::PosInfo> PredictingShellList;

        const static int MAX_PREDICT_STEP = 50;

        explicit AgentSmith(Controller* ctl): ctl(ctl){}
        Ballistics ballisticsPredict(const PredictingShellList& shells);

    private:
        Controller* ctl;
    };
}

#endif //TANK_TROUBLE_AGENT_SMITH_H
