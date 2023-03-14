//
// Created by zr on 23-3-9.
//

#include "OnlineController.h"
#include "Window.h"
#include "util/Id.h"
#include "Shell.h"
#include "event/ControlEvent.h"
#include "net/TcpClient.h"
#include <fcntl.h>

static std::unordered_map<int, Color> colorMap = {
        {1, RED},
        {2, BLUE},
        {3, GREEN},
        {4, YELLOW}
};

void setNonBlocking(int fd)
{
    int opts = ::fcntl(fd, F_GETFL);
    if(opts < 0)
        abort();
    opts |= O_NONBLOCK;
    if(::fcntl(fd, F_SETFL, opts) < 0)
        abort();
}

namespace TankTrouble
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    OnlineController::OnlineController(Window* win, Inet4Address serverAddr):
        Controller(),
        interface(win),
        serverAddress(serverAddr),
        joinedRoomId(0),
        udpSocket(-1),
        handshakeSuccess(false)
    {
        codec.registerHandler(MSG_LOGIN_RESP,
                              std::bind(&OnlineController::onLoginSuccess, this, _1, _2, _3));
        codec.registerHandler(MSG_ROOM_INFO,
                              std::bind(&OnlineController::onRoomsUpdate, this, _1, _2, _3));
        codec.registerHandler(MSG_JOIN_ROOM_RESP,
                              std::bind(&OnlineController::onJoinRoomRespond, this, _1, _2, _3));
        codec.registerHandler(MSG_GAME_ON,
                              std::bind(&OnlineController::onGameOn, this, _1, _2, _3));
        codec.registerHandler(MSG_UPDATE_BLOCKS,
                              std::bind(&OnlineController::onBlocksUpdate, this, _1, _2, _3));
//        codec.registerHandler(MSG_UPDATE_OBJECTS,
//                              std::bind(&OnlineController::onObjectsUpdate, this, _1, _2, _3));
        codec.registerHandler(MSG_UPDATE_SCORES,
                              std::bind(&OnlineController::onScoresUpdate, this, _1, _2, _3));
        codec.registerHandler(MSG_GAME_OFF,
                              std::bind(&OnlineController::onGameOff, this, _1, _2, _3));
    }

    OnlineController::~OnlineController()
    {
        if(udpChannel)
        {
            ev::CountDownLatch latch(1);
            controlLoop->runInLoop([this, &latch] () {
                udpChannel->disableAll();
                udpChannel->remove();
                latch.countDown();
            });
            latch.wait();
        }
        ::close(udpSocket);
    }

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
        auto* controlEvent = new ControlEvent;
        loop.addEventListener(controlEvent,
                              [this](ev::Event* event){this->controlEventHandler(event);});
        {
            std::unique_lock<std::mutex> lk(mu);
            started = true;
            cv.notify_all();
        }
        loop.loop();
        controlLoop = nullptr;
    }

    /********************************* Interface for main window *********************************/

    void OnlineController::login(const std::string& name)
    {
        nickname = name;
        client->connect();
    }

    void OnlineController::createNewRoom(const std::string &name, uint8_t cap)
    {
        Message newRoom = codec.getEmptyMessage(MSG_NEW_ROOM);
        newRoom.setField<Field<std::string>>("room_name", name);
        newRoom.setField<Field<uint8_t>>("player_num", cap);
        Buffer buf = Codec::packMessage(MSG_NEW_ROOM, newRoom);
        client->send(buf);
    }

    void OnlineController::joinRoom(uint8_t roomId)
    {
        Message joinRoom = codec.getEmptyMessage(MSG_JOIN_ROOM);
        joinRoom.setField<Field<uint8_t>>("join_room_id", roomId);
        Buffer buf = Codec::packMessage(MSG_JOIN_ROOM, joinRoom);
        client->send(buf);
    }

    void OnlineController::quitGame()
    {
        Message quitRoom = codec.getEmptyMessage(MSG_QUIT_ROOM);
        quitRoom.setField<Field<std::string>>("msg", "quit");
        Buffer buf = Codec::packMessage(MSG_QUIT_ROOM, quitRoom);
        client->send(buf);
        joinedRoomId = 0;
    }

    OnlineUser OnlineController::getUserInfo()
    {
        std::lock_guard<std::mutex> lg(userInfoMu);
        return userInfo;
    }

    std::vector<RoomInfo> OnlineController::getRoomInfos(uint8_t* currentRoomId)
    {
        std::lock_guard<std::mutex> lg(roomsInfoMu);
        *currentRoomId = joinedRoomId;
        return roomsInfo;
    }

    void OnlineController::sendLoginMessage(const TcpConnectionPtr& conn)
    {
        udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if(udpSocket < 0)
            abort();
        setNonBlocking(udpSocket);
        Message login = codec.getEmptyMessage(MSG_LOGIN);
        login.setField<Field<std::string>>("nickname", nickname);
        Codec::sendMessage(conn, MSG_LOGIN, login);
    }

    void OnlineController::sendControlMessage(uint8_t action, uint8_t enable)
    {
        Message control = codec.getEmptyMessage(MSG_CONTROL);
        control.setField<Field<uint8_t>>("action", action);
        control.setField<Field<uint8_t>>("enable", enable);
        Buffer buf = Codec::packMessage(MSG_CONTROL, control);
        client->send(buf);
    }

    void OnlineController::controlEventHandler(ev::Event *event)
    {
        auto* ce = dynamic_cast<ControlEvent*>(event);
        switch (ce->operation())
        {
            case ControlEvent::Forward:
                sendControlMessage(ControlEvent::Forward, 1); break;
            case ControlEvent::Backward:
                sendControlMessage(ControlEvent::Backward, 1); break;
            case ControlEvent::RotateCW:
                sendControlMessage(ControlEvent::RotateCW, 1); break;
            case ControlEvent::RotateCCW:
                sendControlMessage(ControlEvent::RotateCCW, 1); break;
            case ControlEvent::StopForward:
                sendControlMessage(ControlEvent::Forward, 0); break;
            case ControlEvent::StopBackward:
                sendControlMessage(ControlEvent::Backward, 0); break;
            case ControlEvent::StopRotateCW:
                sendControlMessage(ControlEvent::RotateCW, 0); break;
            case ControlEvent::StopRotateCCW:
                sendControlMessage(ControlEvent::RotateCCW, 0); break;
            case ControlEvent::Fire:
                sendControlMessage(ControlEvent::Fire, 1); break;
        }
    }

    /********************************** message handlers **************************************/

    void OnlineController::onLoginSuccess(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        std::string name = message.getField<Field<std::string>>("nickname").get();
        uint32_t score = message.getField<Field<uint32_t>>("score").get();
        userId = message.getField<Field<uint32_t>>("user_id").get();
        if(name != nickname)
            return;
        udpChannel = std::make_unique<ev::reactor::Channel>(controlLoop, udpSocket);
        udpChannel->setReadCallback([this] (ev::Timestamp) {udpRead();});
        udpChannel->enableReading();
        udpHandshake();
        std::lock_guard<std::mutex> lg(userInfoMu);
        userInfo.nickname_ = name;
        userInfo.score_ = score;
        interface->notifyLoginSuccess();
    }

    void OnlineController::onRoomsUpdate(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        auto rooms = message.getArray<StructField<uint8_t, std::string, uint8_t, uint8_t>>("room_infos");
        std::vector<RoomInfo> newRoomInfos;
        for(int i = 0; i < rooms.length(); i++)
        {
            auto id = rooms.get(i).get<uint8_t>("room_id");
            auto name = rooms.get(i).get<std::string>("room_name");
            auto cap = rooms.get(i).get<uint8_t>("room_cap");
            auto players = rooms.get(i).get<uint8_t>("room_players");
            RoomStatus status = players < cap ? Waiting : Playing;
            newRoomInfos.emplace_back(id, name, cap, players, status);
        }
        std::lock_guard<std::mutex> lg(roomsInfoMu);
        roomsInfo = std::move(newRoomInfos);
        interface->notifyRoomUpdate();
    }

    void OnlineController::onJoinRoomRespond(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        uint8_t roomId = message.getField<Field<uint8_t>>("join_room_id").get();
        uint8_t code = message.getField<Field<uint8_t>>("operation_status").get();
        std::lock_guard<std::mutex> lg(roomsInfoMu);
        if(code == Codec::JOIN_ROOM_SUCCESS)
            joinedRoomId = roomId;
    }

    void OnlineController::onGameOn(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        auto players = message.getArray<StructField<uint8_t, std::string>>("players_info");
        std::lock_guard<std::mutex> lg(playersInfoMu);
        for(int i = 0; i < players.length(); i++)
        {
            auto playerId = players.get(i).get<uint8_t>("player_id");
            auto playerNickname = players.get(i).get<std::string>("player_nickname");
            playersInfo[playerId] = PlayerInfo(playerNickname, colorMap[playerId]);
        }
        interface->notifyGameOn();
    }

    void OnlineController::onBlocksUpdate(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        auto blocksData = message.getArray<StructField<uint8_t, uint64_t, uint64_t>>("blocks");
        std::lock_guard<std::mutex> lg(blocksMu);
        blocks.clear();
        for(int i = 0; i < blocksData.length(); i++)
        {
            bool horizon = (blocksData.get(i).get<uint8_t>("is_horizon") == 1);
            auto x_ = blocksData.get(i).get<uint64_t>("center_x");
            auto y_ = blocksData.get(i).get<uint64_t>("center_y");
            double x, y;
            memcpy(&x, &x_, sizeof(double));
            memcpy(&y, &y_, sizeof(double));
            blocks[i + 1] = Block(horizon, util::Vec(x, y));
        }
    }

    void OnlineController::onObjectsUpdate(Message message)
    {
        std::lock_guard<std::mutex> lg(mu);
        if(!snapshot.unique())
            snapshot.reset(new ObjectList);
        assert(snapshot.unique());
        snapshot->clear();
        int uselessShellId = MAX_TANK_ID + 1;
        auto tanks = message.getArray<StructField<uint8_t, uint64_t, uint64_t, uint64_t>>("tanks");
        auto shells = message.getArray<StructField<uint64_t, uint64_t>>("shells");
        for(int i = 0; i < tanks.length(); i++)
        {
            int tankId = tanks.get(i).get<uint8_t>("id");
            auto x_ = tanks.get(i).get<uint64_t>("center_x");
            auto y_ = tanks.get(i).get<uint64_t>("center_y");
            auto angle_ = tanks.get(i).get<uint64_t>("angle");
            double x, y, angle;
            memcpy(&x, &x_, sizeof(double));
            memcpy(&y, &y_, sizeof(double));
            memcpy(&angle, &angle_, sizeof(double));
            (*snapshot)[tankId] = std::make_unique<Tank>(tankId, util::Vec(x, y), angle, colorMap[tankId]);
        }
        for(int i = 0; i < shells.length(); i++)
        {
            auto x_ = shells.get(i).get<uint64_t>("x");
            auto y_ = shells.get(i).get<uint64_t>("y");
            double x, y;
            memcpy(&x, &x_, sizeof(double));
            memcpy(&y, &y_, sizeof(double));
            (*snapshot)[uselessShellId] = std::make_unique<Shell>(
                    uselessShellId++, util::Vec(x, y), 0.0, 0);
        }
    }

    void OnlineController::onScoresUpdate(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        auto scores = message.getArray<StructField<uint8_t, uint32_t>>("scores");
        std::lock_guard<std::mutex> lg(playersInfoMu);
        for(int i = 0; i < scores.length(); i++)
        {
            auto playerId = scores.get(i).get<uint8_t>("player_id");
            auto playerScore = scores.get(i).get<uint32_t>("score");
            playersInfo[playerId].score_ = playerScore;
        }
    }

    void OnlineController::onGameOff(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        std::string msg = message.getField<Field<std::string>>("msg").get();
        if(msg != "off")
            return;
        interface->notifyGameOff();
    }

    void OnlineController::udpRead()
    {
        char buf[1024];
        struct sockaddr servAddr{};
        auto len = static_cast<socklen_t>(sizeof(servAddr));
        ssize_t n = ::recvfrom(udpSocket, buf, sizeof(buf), 0, &servAddr, &len);
        Buffer data;
        data.append(buf, n);
        if(data.readableBytes() < HeaderLen)
            return;
        FixHeader header = getHeader(&data);
        if(data.readableBytes() >= HeaderLen + header.messageLen)
        {
            data.retrieve(HeaderLen);
            if(header.messageType == MSG_UPDATE_OBJECTS)
            {
                Message message = codec.getEmptyMessage(MSG_UPDATE_OBJECTS);
                message.fill(&data);
                onObjectsUpdate(std::move(message));
            }
            else if(header.messageType == MSG_UDP_HANDSHAKE)
            {
                Message message = codec.getEmptyMessage(MSG_UDP_HANDSHAKE);
                message.fill(&data);
                std::string ack = message.getField<Field<std::string>>("msg").get();
                if(ack == "handshake")
                    handshakeSuccess = true;
            }
        }
        else return;
    }

    void OnlineController::udpHandshake()
    {
        assert(userId != 0);
        Message handshake = codec.getEmptyMessage(MSG_UDP_HANDSHAKE);
        handshake.setField<Field<uint32_t>>("user_id", userId);
        handshake.setField<Field<std::string>>("msg", "handshake");
        Buffer buf = Codec::packMessage(MSG_UDP_HANDSHAKE, handshake);
        Inet4Address servUdpAddr(serverAddress.toIp(), serverAddress.port() + 1);
        sendto(udpSocket, buf.peek(), buf.readableBytes(), 0,
               servUdpAddr.getSockAddr(), static_cast<socklen_t>(sizeof(sockaddr)));
        controlLoop->runAfter(0.5, [this] () {checkRetransmission();});
    }

    void OnlineController::checkRetransmission()
    {
        if(handshakeSuccess)
            return;
        udpHandshake();
    }
}