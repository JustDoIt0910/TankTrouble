//
// Created by zr on 23-2-17.
//

#ifndef TANK_TROUBLE_BLOCK_H
#define TANK_TROUBLE_BLOCK_H
#include "util/Vec.h"
#include <cairomm/context.h>

namespace TankTrouble
{
    class Block
    {
    public:
        Block(const util::Vec& start, const util::Vec& end);
        void draw(const Cairo::RefPtr<Cairo::Context>& cr);
        [[nodiscard]] bool isHorizon() const;
        const static int BLOCK_WIDTH = 4;

    private:
        util::Vec _start, _end;
        util::Vec tl, tr, bl, br;
        bool horizon;
    };
}

#endif //TANK_TROUBLE_BLOCK_H
