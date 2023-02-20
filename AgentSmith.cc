//
// Created by zr on 23-2-20.
//

#include "AgentSmith.h"
#include "Controller.h"
#include "Shell.h"

namespace TankTrouble
{
    AgentSmith::Ballistics AgentSmith::ballisticsPredict(const PredictingShellList& shells)
    {
        AgentSmith::Ballistics ballistics;
        for(const auto& shell: shells)
        {
            std::vector<util::Vec> ballistic;
            int sid = shell.first;
            Object::PosInfo cur = shell.second;
            ballistic.push_back(cur.pos);
            for(int i = 0; i < MAX_PREDICT_STEP - 1; i++)
            {
                Object::PosInfo next = Shell::getNextPosition(cur, 0, 0);
                int collisionId = ctl->checkShellBlockCollision(cur, next);
                if(collisionId)
                {
                    next = ctl->getBouncedPosition(cur, next, collisionId);
                    ballistic.push_back(next.pos);
                }
                cur = next;
            }
            ballistic.push_back(cur.pos);
            ballistics[sid] = ballistic;
        }
        return ballistics;
    }
}