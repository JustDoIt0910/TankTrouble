//
// Created by zr on 23-3-3.
//

#ifndef EV_NONCOPYABLE_H
#define EV_NONCOPYABLE_H

namespace ev
{
    class noncopyable
    {
    public:
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };
}

#endif //EV_NONCOPYABLE_H
