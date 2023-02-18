//
// Created by zr on 23-2-17.
//

#include "Block.h"
#include "View.h"
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
    }

    void Block::draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        cr->save();
        View::drawRect(cr, BLACK, tl, tr, bl, br);
        cr->set_font_size(20.0);
        cr->move_to(_center.x(), _center.y());
        cr->show_text(std::to_string(_id));
        cr->restore();
    }

    bool Block::isHorizon() const {return horizon;}

    util::Vec Block::center() {return _center;}

    int Block::height() const {return _height;}

    int Block::width() const {return _width;}

    int Block::id() const {return  _id;}
}