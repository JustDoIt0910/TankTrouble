//
// Created by zr on 23-3-9.
//

#include "OnlineController.h"
#include "Window.h"
#include "net/TcpClient.h"

namespace TankTrouble
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    OnlineController::OnlineController(Window* win, Inet4Address serverAddr):
        Controller(),
        interface(win),
        serverAddress(serverAddr)
    {
        codec.registerHandler(MSG_LOGIN_RESP,
                              std::bind(&OnlineController::onLoginSuccess, this, _1, _2, _3));
    }

    OnlineController::~OnlineController() = default;

    void OnlineController::start()
    {
        controlThread = std::thread(&OnlineController::run, this);
        std::unique_lock<std::mutex> lk(mu);
        cv.wait(lk, [this]() -> bool { return this->started; });
    }

    void OnlineController::run()
    {
        ev::reactor::EventLoop loop;
        controlLoop = &loop;
        client = std::make_unique<TcpClient>(controlLoop, serverAddress);
        client->setMessageCallback(std::bind(&Codec::handleMessage, &codec, _1, _2, _3));
        client->setConnectionCallback(std::bind(&OnlineController::sendLoginMessage, this, _1));
        {
            std::unique_lock<std::mutex> lk(mu);
            started = true;
            cv.notify_all();
        }
        loop.loop();
        controlLoop = nullptr;
    }

    void OnlineController::login(const std::string& name)
    {
        nickname = name;
        client->connect();
    }

    void OnlineController::sendLoginMessage(const TcpConnectionPtr& conn)
    {
        Message login = codec.getEmptyMessage(MSG_LOGIN);
        login.setField<Field<std::string>>("nickname", nickname);
        Codec::sendMessage(conn, MSG_LOGIN, login);
    }

    /********************************** message handlers **************************************/

    void OnlineController::onLoginSuccess(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        std::string name = message.getField<Field<std::string>>("nickname").get();
        uint32_t score = message.getField<Field<uint32_t>>("score").get();
        if(name != nickname)
            return;
        std::lock_guard<std::mutex> lg(userInfoMu);
        userInfo.nickname_ = name;
        userInfo.score_ = score;
    }
}