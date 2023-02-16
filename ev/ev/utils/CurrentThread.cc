//
// Created by zr on 23-2-10.
//
#include "CurrentThread.h"
#include <type_traits>
#include <sys/types.h>
#include <unistd.h>
#include <syscall.h>

namespace ev
{
    __thread int cachedTid = 0;
    static_assert(std::is_same_v<int, pid_t>, "pid_t should be int");

    void cacheTid()
    {
        if(!cachedTid)
            cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}
