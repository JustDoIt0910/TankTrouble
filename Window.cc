//
// Created by zr on 23-2-16.
//

#include "Window.h"

#include <memory>
#include "defs.h"
#include "event/ControlEvent.h"
#include "controller/LocalController.h"

namespace TankTrouble
{
    Window::Window():
        ctl(nullptr),
        KeyUpPressed(false), KeyDownPressed(false),
        KeyLeftPressed(false), KeyRightPressed(false),
        spacePressed(false)
    {
        set_title("TankTrouble");
        set_default_size(WINDOW_WIDTH, WINDOW_HEIGHT);
        set_resizable(false);

        entryView.signal_choose_local().
            connect(sigc::mem_fun(*this, &Window::on_choose_local));
        add(entryView);
        entryView.show();
    }

    void Window::on_choose_local()
    {
        remove();
        ctl = std::make_unique<LocalController>();
        ctl->start();
        add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
        gameArea = std::make_unique<GameArea>(ctl.get());
        add(*gameArea);
        gameArea->show();
    }

    bool Window::on_key_press_event(GdkEventKey* key_event)
    {
        if(!KeyUpPressed && key_event->keyval == GDK_KEY_Up)
        {
            KeyUpPressed = true;
            auto* event = new ControlEvent(ControlEvent::Forward);
            ctl->dispatchEvent(event);
        }
        else if(!KeyDownPressed && key_event->keyval == GDK_KEY_Down)
        {
            KeyDownPressed = true;
            auto* event = new ControlEvent(ControlEvent::Backward);
            ctl->dispatchEvent(event);
        }
        else if(!KeyLeftPressed && key_event->keyval == GDK_KEY_Left)
        {
            KeyLeftPressed = true;
            auto* event = new ControlEvent(ControlEvent::RotateCCW);
            ctl->dispatchEvent(event);
        }
        else if(!KeyRightPressed && key_event->keyval == GDK_KEY_Right)
        {
            KeyRightPressed = true;
            auto* event = new ControlEvent(ControlEvent::RotateCW);
            ctl->dispatchEvent(event);
        }
        else if(!spacePressed && key_event->keyval == GDK_KEY_space)
        {
            spacePressed = true;
            auto* event = new ControlEvent(ControlEvent::Fire);
            ctl->dispatchEvent(event);
        }
        return true;
    }

    bool Window::on_key_release_event(GdkEventKey* key_event)
    {
        if(key_event->keyval == GDK_KEY_Up)
        {
            KeyUpPressed = false;
            auto* event = new ControlEvent(ControlEvent::StopForward);
            ctl->dispatchEvent(event);
        }
        else if(key_event->keyval == GDK_KEY_Down)
        {
            KeyDownPressed = false;
            auto* event = new ControlEvent(ControlEvent::StopBackward);
            ctl->dispatchEvent(event);
        }
        else if(key_event->keyval == GDK_KEY_Left)
        {
            KeyLeftPressed = false;
            auto* event = new ControlEvent(ControlEvent::StopRotateCCW);
            ctl->dispatchEvent(event);
        }
        else if(key_event->keyval == GDK_KEY_Right)
        {
            KeyRightPressed = false;
            auto* event = new ControlEvent(ControlEvent::StopRotateCW);
            ctl->dispatchEvent(event);
        }
        else if(key_event->keyval == GDK_KEY_space) {spacePressed = false;}
        return true;
    }
}