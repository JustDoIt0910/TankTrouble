//
// Created by zr on 23-2-17.
//

#include "Block.h"
#include "View.h"
#include <cassert>

namespace TankTrouble
{
    Block::Block(const util::Vec& start, const util::Vec& end):
        _start(start), _end(end)
    {
        assert(_start.y() == _end.y() || _start.x() == _end.x());
        horizon = _start.y() == _end.y();
        if(horizon)
        {
            if(_start.x() > _end.x())
                _start.swap(_end);
            tl = util::Vec(_start.x() - static_cast<double>(BLOCK_WIDTH) / 2,
                            _start.y() - static_cast<double>(BLOCK_WIDTH) / 2);
            bl = util::Vec(_start.x() - static_cast<double>(BLOCK_WIDTH) / 2,
                            _start.y() + static_cast<double>(BLOCK_WIDTH) / 2);
            tr = util::Vec(_end.x() + static_cast<double>(BLOCK_WIDTH) / 2,
                            _end.y() - static_cast<double>(BLOCK_WIDTH) / 2);
            br = util::Vec(_end.x() + static_cast<double>(BLOCK_WIDTH) / 2,
                            _end.y() + static_cast<double>(BLOCK_WIDTH) / 2);
        }
        else
        {
            if(_start.y() > _end.y())
                _start.swap(_end);
            tl = util::Vec(_start.x() - static_cast<double>(BLOCK_WIDTH) / 2,
                            _start.y() - static_cast<double>(BLOCK_WIDTH) / 2);
            tr = util::Vec(_start.x() + static_cast<double>(BLOCK_WIDTH) / 2,
                            _start.y() - static_cast<double>(BLOCK_WIDTH) / 2);
            bl = util::Vec(_end.x() - static_cast<double>(BLOCK_WIDTH) / 2,
                            _end.y() + static_cast<double>(BLOCK_WIDTH) / 2);
            br = util::Vec(_end.x() + static_cast<double>(BLOCK_WIDTH) / 2,
                            _end.y() + static_cast<double>(BLOCK_WIDTH) / 2);
        }
    }

    void Block::draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        cr->save();
        View::drawRect(cr, BLACK, tl, tr, bl, br);
        cr->restore();
    }

    bool Block::isHorizon() const {return horizon;}
}