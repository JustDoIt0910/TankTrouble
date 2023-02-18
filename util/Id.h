//
// Created by zr on 23-2-18.
//

#ifndef TANK_TROUBLE_ID_H
#define TANK_TROUBLE_ID_H
#include <mutex>

namespace TankTrouble::util
{
    class Id
    {
    public:
        static int getTankId();
        static int getBlockId();

    private:
        static int globalTankId;
        static std::mutex tankIdMu;
        static int globalBlockId;
        static std::mutex blockIdMu;
    };
}

#endif //TANK_TROUBLE_ID_H
