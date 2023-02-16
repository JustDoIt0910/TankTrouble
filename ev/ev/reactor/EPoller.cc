//
// Created by zr on 23-2-9.
//

#include "EPoller.h"
#include "Channel.h"
#include <cassert>
#include <unistd.h>
#include <strings.h>

namespace ev::reactor
{
    EPoller::EPoller(EventLoop *loop):
        ownerLoop(loop),
        epollFd(::epoll_create1(EPOLL_CLOEXEC)),
        events(InitEventListSize){}

    EPoller::~EPoller() {::close(epollFd);}

    bool EPoller::hasChannel(Channel *channel)
    {
        ownerLoop->assertInLoopThread();
        auto it = channels.find(channel->fd());
        return (it != channels.end() && it->second == channel);
    }

    void EPoller::updateChannel(Channel* channel)
    {
        ownerLoop->assertInLoopThread();
        Channel::ChannelStatus status = channel->status();
        /* New Channel 没有调用过update, 没有注册到epoll实例中，也没有添加到poller的channels中
         * Added Channel 调用过update，添加到epoll实例和channels中
         * Deleted Channel 使用NoneEvents调用update，fd从epoll实例中删除，但Channel对象依然在channels中,
         * 可以通过update重新注册到epoll中，转化为Added Channel*/
        int fd = channel->fd();
        if(status == Channel::New || status == Channel::Deleted)
        {
            if(status == Channel::New)
            {
                // New Channel 不应该在poller的channels中
                assert(channels.find(fd) == channels.end());
                channels[fd] = channel;
            }
            else
            {
                // Deleted Channel 应该在channels中
                assert(channels.find(fd) != channels.end());
                assert(channels[fd] == channel);
            }
            assert(channel->getInterestEvents() != Channel::NoneEvent);
            update(EPOLL_CTL_ADD, channel);
            channel->setStatus(Channel::Added);
        }
        else
        {
            // Added Channel 已经注册过，需要修改
            assert(channels.find(fd) != channels.end());
            assert(channels[fd] == channel);
            if(channel->getInterestEvents() == Channel::NoneEvent)
            {
                update(EPOLL_CTL_DEL, channel);
                channel->setStatus(Channel::Deleted);
            }
            else
                update(EPOLL_CTL_MOD, channel);
        }
    }

    void EPoller::removeChannel(Channel *channel)
    {
        ownerLoop->assertInLoopThread();
        int fd = channel->fd();
        Channel::ChannelStatus status = channel->status();
        assert(status != Channel::New);
        assert(channels.find(fd) != channels.end());
        assert(channels[fd] == channel);
        assert(channel->getInterestEvents() == Channel::NoneEvent);
        channels.erase(channel->fd());
        //如果是 Added Channel, 还需要从epoll实例中删除fd
        if(status == Channel::Added)
            update(EPOLL_CTL_DEL, channel);
        channel->setStatus(Channel::New);
    }

    void EPoller::update(int op, Channel *channel) const
    {
        struct epoll_event event{};
        bzero(&event, sizeof(event));
        event.events = channel->getInterestEvents();
        event.data.ptr = channel;
        if(::epoll_ctl(epollFd, op, channel->fd(), &event) < 0)
        {
            // TODO handle error
        }
    }

    Timestamp EPoller::poll(int timeoutMs, ChannelList &activeChannels)
    {
        int eventNum = ::epoll_wait(epollFd, &events[0], static_cast<int>(events.size()), timeoutMs);
        Timestamp returnTime = Timestamp::now();
        if(eventNum > 0)
        {
            fillActiveChannels(eventNum, activeChannels);
            //事件数组满了，进行扩容
            if(static_cast<size_t>(eventNum) == events.size())
                events.resize(2 * events.size());
        }
        else if(eventNum < 0)
        {
            // TODO handle error
        }
        return returnTime;
    }

    void EPoller::fillActiveChannels(int eventNum, ChannelList& activeChannels)
    {
        assert(static_cast<size_t>(eventNum) <= events.size());
        for(int i = 0; i < eventNum; i++)
        {
            struct epoll_event event = events[i];
            auto channel = static_cast<Channel*>(event.data.ptr);
            channel->setOccurredEvent(static_cast<int>(event.events));
            activeChannels.push_back(channel);
        }
    }
}