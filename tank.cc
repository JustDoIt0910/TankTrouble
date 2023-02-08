//
// Created by zr on 23-2-8.
//

#include "tank.h"
#include "util/math.h"


tank::Tank::Tank(const util::Cord& p, double angle, const Color& c):
    Object(p, angle, tank::TANK_MOVING_STEP, c), remainBullets(5), dir(MOVING_STATIONARY)
{
    auto corners = util::getCornerCord(pos, angle, Tank::TANK_WIDTH, Tank::TANK_HEIGHT);
    topLeft = corners[0]; topRight = corners[1]; bottomLeft = corners[2]; bottomRight = corners[3];
}

void tank::Tank::draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    cr->save();
    cr->set_source_rgb(color[0], color[1], color[2]);
    cr->set_line_width(1.0);
    cr->move_to(topLeft.x(), topLeft.y());
    cr->line_to(topRight.x(), topRight.y());
    cr->line_to(bottomRight.x(), bottomRight.y());
    cr->line_to(bottomLeft.x(), bottomLeft.y());
    cr->close_path();
    cr->fill();

    cr->set_source_rgb(color[0] - 0.2, color[1], color[2]);
    cr->arc(pos.x(), pos.y(), 6, 0.0, 2 * M_PI);
    cr->fill();

    cr->set_line_width(7.0);
    cr->move_to(pos.x(), pos.y());
    util::Cord to = util::polar2Cart(angle, 17, pos);
    cr->line_to(to.x(), to.y());
    cr->stroke();
    cr->restore();
}

void tank::Tank::move()
{

}