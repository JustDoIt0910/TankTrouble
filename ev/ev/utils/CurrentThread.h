//
// Created by zr on 23-2-9.
//

#ifndef EV_CURRENTTHREAD_H
#define EV_CURRENTTHREAD_H

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace ev
{
    extern __thread int cachedTid;
    void cacheTid();

    inline int currentTreadId()
    {
        if(unlikely(cachedTid == 0))
        {
            cacheTid();
        }
        return cachedTid;
    }
}

#endif //EV_CURRENTTHREAD_H
