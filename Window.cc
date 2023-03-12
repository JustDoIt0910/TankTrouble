//
// Created by zr on 23-2-16.
//

#include "Window.h"

#include <memory>
#include "view/GameView.h"
#include "view/GameLobby.h"
#include "defs.h"
#include "event/ControlEvent.h"
#include "controller/LocalController.h"
#include "controller/OnlineController.h"

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
        add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
        entryView.signal_choose_local().connect(sigc::mem_fun(*this, &Window::onUserChooseLocal));
        entryView.signal_choose_online().connect(sigc::mem_fun(*this, &Window::onUserChooseOnline));
        loginView.signal_login_clicked().connect(sigc::mem_fun(*this, &Window::onUserLogin));
        add(entryView);

        loginSuccessNotifier.connect(sigc::mem_fun(*this, &Window::onLoginSuccess));
        roomUpdateNotifier.connect(sigc::mem_fun(*this, &Window::onRoomsUpdate));
        gameOnNotifier.connect(sigc::mem_fun(*this, &Window::onGameBegin));
        gameOffNotifier.connect(sigc::mem_fun(*this, &Window::onGameOff));

        entryView.show();
    }

    void Window::notifyLoginSuccess() {loginSuccessNotifier.emit();}

    void Window::notifyRoomUpdate() {roomUpdateNotifier.emit();}

    void Window::notifyGameOn() {gameOnNotifier.emit();}

    void Window::notifyGameOff() {gameOffNotifier.emit();}

    void Window::onUserChooseLocal()
    {
        remove();
        ctl = std::make_unique<LocalController>();
        ctl->start();
        gameView = std::make_unique<GameView>(ctl.get());
        gameView->signal_quit_game().connect(sigc::mem_fun(*this, &Window::toEntryView));
        add(*gameView);
        gameView->show();
    }

    void Window::onUserChooseOnline()
    {
        remove();
        ctl = std::make_unique<OnlineController>(this, Inet4Address("127.0.0.1", 8080));
        ctl->start();
        add(loginView);
        loginView.show();
    }

    void Window::onUserLogin(const std::string& nickname)
    {
        auto* onlineController = dynamic_cast<OnlineController*>(ctl.get());
        onlineController->login(nickname);
    }

    void Window::toEntryView()
    {
        remove();
        if(gameView) gameView.reset();
        if(gameLobby) gameLobby.reset();
        if(ctl) ctl.reset();
        add(entryView);
        entryView.show();
    }

    void Window::onLoginSuccess()
    {
        remove();
        auto* onlineController = dynamic_cast<OnlineController*>(ctl.get());
        gameLobby = std::make_unique<GameLobby>(onlineController);
        gameLobby->signal_logout().connect(sigc::mem_fun(*this, &Window::toEntryView));
        add(*gameLobby);
        gameLobby->show();
    }

    void Window::onRoomsUpdate()
    {
        if(gameLobby)
            gameLobby->getRoomInfo();
    }

    void Window::onGameBegin()
    {
        remove();
        gameView = std::make_unique<GameView>(ctl.get());
        add(*gameView);
        gameView->signal_quit_game().
            connect(sigc::mem_fun(*this, &Window::onGameOff));
        gameView->show();
    }

    void Window::onGameOff()
    {
        if(!gameLobby)
            return;
        remove();
        add(*gameLobby);
        gameLobby->show();
        gameLobby->getRoomInfo();
    }

    bool Window::on_key_press_event(GdkEventKey* key_event)
    {
        if(!ctl)
            return Gtk::Window::on_key_press_event(key_event);
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
        return Gtk::Window::on_key_press_event(key_event);
    }

    bool Window::on_key_release_event(GdkEventKey* key_event)
    {
        if(!ctl)
            return Gtk::Window::on_key_press_event(key_event);
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
        return Gtk::Window::on_key_release_event(key_event);
    }

    Window::~Window() = default;
}