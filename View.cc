//
// Created by zr on 23-2-8.
//

#include "View.h"
#include "defs.h"
#include "util/Vec.h"
#include <glibmm/main.h>

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
        drawOutline(cr);
        Controller::BlockList* blocks = ctl->getBlocks();
        for(auto& block: *blocks)
            block.draw(cr);

        Controller::ObjectListPtr snapshot = ctl->getObjects();
        for(auto &obj: *snapshot)
            obj->draw(cr);

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