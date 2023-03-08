//
// Created by zr on 23-2-9.
//

#ifndef EV_EPOLLER_H
#define EV_EPOLLER_H
#include "EventLoop.h"
#include "utils/Timestamp.h"
#include "utils/noncopyable.h"
#include <sys/epoll.h>
#include <vector>
#include <map>

namespace ev::reactor
{
    class EPoller : public noncopyable
    {
    public:
        explicit EPoller(EventLoop* loop);
        ~EPoller();
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);

        typedef std::vector<Channel*> ChannelList;
        Timestamp poll(int timeoutMs, ChannelList& activeChannels);

        bool hasChannel(Channel* channel);

        const static int InitEventListSize = 16;
        // 10s epoll_wait超时事件
        const static int EpollTimeoutMs = 1000 * 10;

    private:
        void fillActiveChannels(int eventNum, ChannelList& activeChannels);
        void update(int op, Channel* channel) const;

        int epollFd;
        EventLoop* ownerLoop;
        typedef std::map<int, Channel*> ChannelMap;
        ChannelMap channels;
        std::vector<struct epoll_event> events;
    };
}

#endif //EV_EPOLLER_H
