//
// Created by zr on 23-2-17.
//

#include "Block.h"
#include "View.h"
#include "Shell.h"
#include <cassert>

namespace TankTrouble
{
    Block::Block(int id, const util::Vec& start, const util::Vec& end):
        _id(id), _start(start), _end(end)
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
            _width = static_cast<int>(bl.y() - tl.y());
            _height = static_cast<int>(tr.x() - tl.x());
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
            _height = static_cast<int>(bl.y() - tl.y());
            _width = static_cast<int>(tr.x() - tl.x());
        }
        _center = util::Vec((tl.x() + tr.x()) / 2, (tl.y() + bl.y()) / 2);
        util::Vec btl(tl.x() - Shell::RADIUS, tl.y() - Shell::RADIUS);
        util::Vec btr(tr.x() + Shell::RADIUS, tr.y() - Shell::RADIUS);
        util::Vec bbl(bl.x() - Shell::RADIUS, bl.y() + Shell::RADIUS);
        util::Vec bbr(br.x() + Shell::RADIUS, br.y() + Shell::RADIUS);
        _border[0] = std::make_pair(btl, btr);
        _border[1] = std::make_pair(bbl, bbr);
        _border[2] = std::make_pair(btl, bbl);
        _border[3] = std::make_pair(btr, bbr);
    }

    void Block::draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        cr->save();
        View::drawRect(cr, BLACK, tl, tr, bl, br);
        cr->set_line_width(1.0);
        cr->restore();
    }

    bool Block::isHorizon() const {return horizon;}

    util::Vec Block::center() const {return _center;}

    int Block::height() const {return _height;}

    int Block::width() const {return _width;}

    util::Vec Block::start() const {return _start;}

    util::Vec Block::end() const {return _end;}

    int Block::id() const {return  _id;}

    std::pair<util::Vec, util::Vec> Block::border(int n) const
    {
        assert(n >= 0 && n < 4);
        return _border[n];
    }
}