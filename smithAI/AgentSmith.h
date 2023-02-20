//
// Created by zr on 23-2-20.
//

#ifndef TANK_TROUBLE_AGENT_SMITH_H
#define TANK_TROUBLE_AGENT_SMITH_H
#include <utility>
#include <vector>
#include <unordered_map>
#include "../util/Vec.h"
#include "../Object.h"
#include "DodgeStrategy.h"

namespace TankTrouble
{
    class Block;
    class Controller;

    class AgentSmith
    {
    public:
        typedef std::vector<std::pair<uint64_t , util::Vec>> Ballistic;
        typedef std::unordered_map<int, Ballistic> Ballistics;
        typedef std::unordered_map<int, Object::PosInfo> PredictingShellList;

        const static int MAX_PREDICT_STEP = 100;

        explicit AgentSmith(Controller* ctl): ctl(ctl){}
        void ballisticsPredict(const PredictingShellList& shells, uint64_t globalSteps);
        DodgeStrategy getDodgeStrategy(const std::vector<int>& shells, uint64_t globalSteps);

    private:
        Object::PosInfo getShellPosition(int id, uint64_t globalSteps);
        Controller* ctl;
        Ballistics ballistics;
    };
}

#endif //TANK_TROUBLE_AGENT_SMITH_H
