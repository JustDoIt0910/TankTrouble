//
// Created by zr on 23-2-20.
//

#include "AgentSmith.h"
#include "../Controller.h"
#include "../Shell.h"

namespace TankTrouble
{
    void AgentSmith::ballisticsPredict(const PredictingShellList& shells, uint64_t globalSteps)
    {
        for(const auto& shell: shells)
        {
            Ballistic ballistic;
            int sid = shell.first;
            Object::PosInfo cur = shell.second;
            ballistic.push_back(std::make_pair(globalSteps, cur.pos));
            for(int i = 0; i < MAX_PREDICT_STEP - 1; i++)
            {
                Object::PosInfo next = Shell::getNextPosition(cur, 0, 0);
                int collisionId = ctl->checkShellBlockCollision(cur, next);
                if(collisionId)
                {
                    next = ctl->getBouncedPosition(cur, next, collisionId);
                    ballistic.push_back(std::make_pair(globalSteps + i + 1, next.pos));
                }
                cur = next;
            }
            ballistic.push_back(std::make_pair(globalSteps + MAX_PREDICT_STEP, cur.pos));
            ballistics[sid] = ballistic;
        }
    }

    Object::PosInfo AgentSmith::getShellPosition(int id, uint64_t step)
    {
        Ballistic ballistic = ballistics[id];
        for(int i = 0; i < ballistic.size(); i++)
        {
            if(ballistic[i].first < step)
                continue;
            util::Vec start = ballistic[i - 1].second;
            util::Vec end = ballistic[i].second;
            uint64_t span = ballistic[i].first - ballistic[i - 1].first;
            util::Vec pos(static_cast<double>(step - ballistic[i - 1].first) * (end.x() - start.x()) / static_cast<double>(span) + start.x(),
                          static_cast<double>(step - ballistic[i - 1].first) * (end.y() - start.y()) / static_cast<double>(span) + start.y());
            return {pos, 0.0};
        }
        return Object::PosInfo::invalid();
    }

    DodgeStrategy AgentSmith::getDodgeStrategy(const std::vector<int>& shells, uint64_t globalSteps)
    {
        
    }
}