//
// Created by zr on 23-2-18.
//

#ifndef TANK_TROUBLE_ID_H
#define TANK_TROUBLE_ID_H

namespace TankTrouble::util
{
    class Id
    {
    public:
        static int getTankId();
        static int getBlockId();
        static int getShellId();

    private:
        static int globalTankId;
        static int globalBlockId;
        static int globalShellId;
    };
}

#endif //TANK_TROUBLE_ID_H
