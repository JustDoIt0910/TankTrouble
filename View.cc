//
// Created by zr on 23-2-8.
//

#include "View.h"
#include "defs.h"
#include "util/Vec.h"
#include <glibmm/main.h>
#include "util/Math.h"

namespace TankTrouble
{
    View::View(Controller* c): ctl(c)
    {
        Glib::signal_timeout().connect(
                sigc::mem_fun(*this, &View::on_timeout),
                20);
    }

    static void drawOutline(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        View::drawRect(cr, BLACK,util::Vec(0, 0), util::Vec(4, 0),
                       util::Vec(0, WINDOW_HEIGHT), util::Vec(4, WINDOW_HEIGHT));
        View::drawRect(cr, BLACK,util::Vec(WINDOW_WIDTH - 4, 0), util::Vec(WINDOW_WIDTH, 0),
                       util::Vec(WINDOW_WIDTH - 4, WINDOW_HEIGHT), util::Vec(WINDOW_WIDTH, WINDOW_HEIGHT));
        View::drawRect(cr, BLACK,util::Vec(0, 0), util::Vec(WINDOW_WIDTH, 0),
                       util::Vec(0, 4), util::Vec(WINDOW_WIDTH - 1, 4));
        View::drawRect(cr, BLACK,util::Vec(0, WINDOW_HEIGHT - 4), util::Vec(WINDOW_WIDTH, WINDOW_HEIGHT - 4),
                       util::Vec(0, WINDOW_HEIGHT), util::Vec(WINDOW_WIDTH, WINDOW_HEIGHT));
    }


    bool View::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
//        cr->set_line_width(1.0);
//        util::Vec p1(119, 100);
//        util::Vec p2(200, 100);
//        util::Vec p3(120, 99);
//        util::Vec p4(170, 200);
//        cr->move_to(p1.x(), p1.y());
//        cr->line_to(p2.x(), p2.y());
//        cr->move_to(p3.x(), p3.y());
//        cr->line_to(p4.x(), p4.y());
//        cr->stroke();
//
//        util::Vec i;
//        if(util::intersectionOfSegments(p1, p2, p3, p4, &i))
//        {
//            cr->arc(i.x(), i.y(), 3, 0.0, 2 * M_PI);
//            cr->fill();
//        }

        drawOutline(cr);
        Controller::BlockList* blocks = ctl->getBlocks();
        for(auto& block: *blocks)
            block.second.draw(cr);

        Controller::ObjectListPtr snapshot = ctl->getObjects();
        for(auto &entry: *snapshot)
            entry.second->draw(cr);

        return true;
    }

    void View::drawRect(const Cairo::RefPtr<Cairo::Context>& cr,
                        Color color,
                        util::Vec tl, util::Vec tr, util::Vec bl, util::Vec br)
    {
        cr->set_source_rgb(color[0], color[1], color[2]);
        cr->set_line_width(1.0);
        cr->move_to(tl.x(), tl.y());
        cr->line_to(tr.x(), tr.y());
        cr->line_to(br.x(), br.y());
        cr->line_to(bl.x(), bl.y());
        cr->close_path();
        cr->fill();
    }

    bool View::on_timeout()
    {
        auto win = get_window();
        if (win)
        {
            Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                             get_allocation().get_height());
            win->invalidate_rect(r, false);
        }
        return true;
    }
}