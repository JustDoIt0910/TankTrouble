//
// Created by zr on 23-2-9.
//

#ifndef EV_CHANNEL_H
#define EV_CHANNEL_H
#include "EventLoop.h"
#include "../utils/Timestamp.h"
#include <sys/epoll.h>

namespace ev::reactor
{
    class Channel
    {
    public:
        typedef std::function<void(Timestamp)> ReadCallback;
        typedef std::function<void()> WriteCallback;
        typedef std::function<void()> CloseCallback;
        typedef std::function<void()> ErrorCallback;

        enum ChannelStatus {New, Added, Deleted};
        const static int NoneEvent = 0;
        const static int ReadEvent = EPOLLIN | EPOLLPRI;
        const static int WriteEvent = EPOLLOUT;

        Channel(EventLoop* loop, int fd);
        ~Channel();

        void setReadCallback(ReadCallback cb);
        void setWriteCallback(WriteCallback cb);
        void setCloseCallback(CloseCallback cb);
        void setErrorCallback(ErrorCallback cb);

        void enableReading();
        void enableWriting();
        void disableReading();
        void disableWriting();
        void disableAll();

        void remove();
        void tie(const std::shared_ptr<void>& obj);
        void handleEvent(Timestamp receiveTime);

        [[nodiscard]] int fd() const;
        [[nodiscard]] ChannelStatus status() const;
        void setStatus(ChannelStatus status);
        [[nodiscard]] int getInterestEvents() const;
        void setOccurredEvent(int occurred);
        [[nodiscard]] EventLoop* ownerLoop() const;

    private:
        void update();
        void handleEventWithGuard(Timestamp receiveTime);

        int _fd;
        int interestEvents;
        int occurredEvents;
        EventLoop* _ownerLoop;
        bool tied;
        bool addedToLoop;
        bool eventHandling;
        std::weak_ptr<void> tieWeakPtr;
        ChannelStatus _status;
        ReadCallback readCallback;
        WriteCallback writeCallback;
        CloseCallback closeCallback;
        ErrorCallback errorCallback;
    };
}

#endif //EV_CHANNEL_H
