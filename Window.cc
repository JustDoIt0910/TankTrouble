//
// Created by zr on 23-2-16.
//

#include "Window.h"
#include "event/ControlEvent.h"

namespace TankTrouble
{
    Window::Window(Controller* ctl):
        KeyUpPressed(false), KeyDownPressed(false), KeyLeftPressed(false), KeyRightPressed(false),
        spacePressed(false),
        ctl(ctl)
    {
        set_title("TankTrouble");
        set_default_size(600, 400);
        set_resizable(false);
        add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
    }

    bool Window::on_key_press_event(GdkEventKey* key_event)
    {
        if(!KeyUpPressed && key_event->keyval == GDK_KEY_Up)
        {
            KeyUpPressed = true;
            ControlEvent* event = new ControlEvent(ControlEvent::Forward);
            ctl->dispatchEvent(event);
        }
        else if(!KeyDownPressed && key_event->keyval == GDK_KEY_Down)
        {
            KeyDownPressed = true;
            ControlEvent* event = new ControlEvent(ControlEvent::Backward);
            ctl->dispatchEvent(event);
        }
        else if(!KeyLeftPressed && key_event->keyval == GDK_KEY_Left)
        {
            KeyLeftPressed = true;
            ControlEvent* event = new ControlEvent(ControlEvent::RotateCCW);
            ctl->dispatchEvent(event);
        }
        else if(!KeyRightPressed && key_event->keyval == GDK_KEY_Right)
        {
            KeyRightPressed = true;
            ControlEvent* event = new ControlEvent(ControlEvent::RotateCW);
            ctl->dispatchEvent(event);
        }
        else if(!spacePressed && key_event->keyval == GDK_KEY_space)
        {
            spacePressed = true;
            ControlEvent* event = new ControlEvent(ControlEvent::Fire);
            ctl->dispatchEvent(event);
        }
        return true;
    }

    bool Window::on_key_release_event(GdkEventKey* key_event)
    {
        if(key_event->keyval == GDK_KEY_Up)
        {
            KeyUpPressed = false;
            ControlEvent* event = new ControlEvent(ControlEvent::StopForward);
            ctl->dispatchEvent(event);
        }
        else if(key_event->keyval == GDK_KEY_Down)
        {
            KeyDownPressed = false;
            ControlEvent* event = new ControlEvent(ControlEvent::StopBackward);
            ctl->dispatchEvent(event);
        }
        else if(key_event->keyval == GDK_KEY_Left)
        {
            KeyLeftPressed = false;
            ControlEvent* event = new ControlEvent(ControlEvent::StopRotateCCW);
            ctl->dispatchEvent(event);
        }
        else if(key_event->keyval == GDK_KEY_Right)
        {
            KeyRightPressed = false;
            ControlEvent* event = new ControlEvent(ControlEvent::StopRotateCW);
            ctl->dispatchEvent(event);
        }
        else if(key_event->keyval == GDK_KEY_space) {spacePressed = false;}
        return true;
    }
}