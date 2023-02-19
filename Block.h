//
// Created by zr on 23-2-17.
//

#ifndef TANK_TROUBLE_BLOCK_H
#define TANK_TROUBLE_BLOCK_H
#include "util/Vec.h"
#include <utility>
#include <cairomm/context.h>

namespace TankTrouble
{
    class Block
    {
    public:
        Block() = default;
        Block(int id, const util::Vec& start, const util::Vec& end);
        void draw(const Cairo::RefPtr<Cairo::Context>& cr);
        [[nodiscard]] bool isHorizon() const;
        [[nodiscard]] util::Vec center();
        [[nodiscard]] int height() const;
        [[nodiscard]] int width() const;
        [[nodiscard]] int id() const;
        [[nodiscard]] std::pair<util::Vec, util::Vec> border(int n) const;
        const static int BLOCK_WIDTH = 4;

    private:
        int _id;
        util::Vec _center;
        util::Vec tl, tr, bl, br;
        bool horizon;
        util::Vec _start, _end;
        int _height;
        int _width;
        //计算碰撞位置的包围盒(顺序上下左右)
        std::pair<util::Vec, util::Vec> _border[4];
    };
}

#endif //TANK_TROUBLE_BLOCK_H
