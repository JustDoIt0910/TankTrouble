//
// Created by zr on 23-2-20.
//

#ifndef TANK_TROUBLE_AGENT_SMITH_H
#define TANK_TROUBLE_AGENT_SMITH_H
#include <utility>
#include <vector>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "util/Vec.h"
#include "Object.h"
#include "AStar.h"
#include "DodgeStrategy.h"

#define LEFT_SIDE 0
#define RIGHT_SIDE 1

namespace TankTrouble
{
    class Block;
    class LocalController;

    class AgentSmith
    {
    public:
        typedef std::pair<int, Object::PosInfo> PredictingShell;
        typedef std::unordered_map<int, Object::PosInfo> PredictingShellList;
        typedef std::pair<uint64_t , util::Vec> KeyPoint;
        struct BallisticSegment
        {
            int shellId;
            int seq;
            KeyPoint start;
            KeyPoint end;
            util::Vec center;
            double angle;
            double length;
            double distanceToTarget;
            BallisticSegment(int id, int seq, KeyPoint s, KeyPoint e, double len, double a, double dis):
                    shellId(id), seq(seq), start(std::move(s)), end(std::move(e)),
                    length(len), angle(a), distanceToTarget(dis)
            {center = util::Vec((start.second.x() + end.second.x()) / 2,
                                (start.second.y() + end.second.y()) / 2);}
            static BallisticSegment invalid(){return {-1, -1, KeyPoint(), KeyPoint(), 0, 0, 0};}
            [[nodiscard]] bool isValid() const {return shellId != -1;}
        };
        typedef std::vector<BallisticSegment> Ballistic;
        typedef std::unordered_map<int, Ballistic> Ballistics;

        const static int MAX_PREDICT_STEP;
        const static int THREATENED_RANGE;
        const static int MAX_DODGING_SHELLS;
        const static int ATTACKING_RANGE;

        explicit AgentSmith(LocalController* ctl):
            ctl(ctl),
            aStar(new AStar),
            prevFireTime(-1000),
            prevThreat(BallisticSegment::invalid()),
            dangerSegment(BallisticSegment::invalid()){}

        PredictingShellList getIncomingShells(const Object::PosInfo& smithPos);

        void ballisticsPredict(const PredictingShellList& shells, uint64_t globalSteps);

        void ballisticPredict(PredictingShell shell, Ballistic& ballistic, uint64_t globalSteps);

        void getDodgeStrategy(const Object::PosInfo& smithPos, uint64_t globalSteps);

        void initAStar(AStar::BlockList* blocks);

        void attack(const Object::PosInfo& smith, const Object::PosInfo& enemy,
                    uint64_t globalSteps);

        bool safeToMove(uint64_t globalSteps, const Object::PosInfo& cur, int movingStatus);

    private:
        static bool segmentCmp(const BallisticSegment& s1, const BallisticSegment& s2);

        Object::PosInfo getShellPosition(int id, uint64_t globalSteps);

        bool checkFeasible(uint64_t step, const Object::PosInfo& tryPos);

        DodgeStrategy tryRotation(uint64_t globalSteps);

        DodgeStrategy tryRotatingAndMoving(uint64_t globalSteps);

        DodgeStrategy tryRotatingWithMoving(uint64_t globalSteps);

        DodgeStrategy dodgeToSide(uint64_t globalSteps, const Object::PosInfo& cur,
                                  int whichSide, int angleGran);

        bool movingCurve(uint64_t globalSteps, int direction, int rotation,
                         const Object::PosInfo& cur, int angleGran,
                         uint64_t * rotatingSteps, uint64_t* straightSteps);

        bool tryMovingStraight(uint64_t globalSteps, int direction,
                              const Object::PosInfo& cur, uint64_t * takingSteps);

        enum CheckResult {SAFE, DIE, UNKNOWN};
        CheckResult checkWillDie(uint64_t step, const Object::PosInfo& pos);

        Object::PosInfo tryAiming(const Object::PosInfo& smith, const Object::PosInfo& enemy);

        LocalController* ctl;
        Ballistics ballistics;

        Object::PosInfo smithPos;
        std::vector<BallisticSegment> threats;
        std::unordered_set<int> potentialThreats;

        std::unique_ptr<AStar> aStar;
        uint64_t prevFireTime;
        BallisticSegment prevThreat;
        BallisticSegment dangerSegment;
    };
}

#endif //TANK_TROUBLE_AGENT_SMITH_H
