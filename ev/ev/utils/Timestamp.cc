//
// Created by zr on 23-2-10.
//
#include "Timestamp.h"
#include <sys/time.h>
#include <chrono>
#include <cstring>

namespace ev
{
    Timestamp::Timestamp(int64_t microSecondsSinceEpoch):
        _microSecondsSinceEpoch(microSecondsSinceEpoch) {}

    Timestamp::Timestamp():
        _microSecondsSinceEpoch(0) {}

    bool Timestamp::isValid() const {return _microSecondsSinceEpoch > 0;}

    void Timestamp::addTime(int value, Timestamp::TimeUnit unit)
    {
        struct tm _tm{};
        memset(&_tm, 0, sizeof(_tm));
        time_t t = _microSecondsSinceEpoch / microSecondsPerSecond;
        localtime_r(&t, &_tm);
        switch (unit)
        {
            case year:
                _tm.tm_year = _isvalid_tm_field(_tm.tm_year + value, year) ? _tm.tm_year + value : _tm.tm_year;
                break;
            case month:
                _tm.tm_mon = _isvalid_tm_field(_tm.tm_mon + value, month) ? _tm.tm_mon + value : _tm.tm_mon;
                break;
            case day:
                _tm.tm_mday = _isvalid_tm_field(_tm.tm_mday + value, day) ? _tm.tm_mday + value : _tm.tm_mday;
                break;
            case hour:
                _tm.tm_hour = _isvalid_tm_field(_tm.tm_hour + value, hour) ? _tm.tm_hour + value : _tm.tm_hour;
                break;
            case minute:
                _tm.tm_min = _isvalid_tm_field(_tm.tm_min + value, minute) ? _tm.tm_min + value : _tm.tm_min;
                break;
            case second:
                _tm.tm_sec = _isvalid_tm_field(_tm.tm_sec + value, second) ? _tm.tm_sec + value : _tm.tm_sec;
                break;
            default: break;
        }
        _microSecondsSinceEpoch = mktime(&_tm) * microSecondsPerSecond;
    }

    void Timestamp::addMicroSeconds(int64_t ms) {_microSecondsSinceEpoch += ms;}

    std::string Timestamp::toFormattedString() const
    {
        if (!isValid())
            return {};
        char buf[25];
        memset(buf, 0, sizeof(buf));
        struct tm _tm{};
        memset(&_tm, 0, sizeof(_tm));
        time_t t = _microSecondsSinceEpoch / microSecondsPerSecond;
        localtime_r(&t, &_tm);
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
                _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
        return {buf};
    }

    Timestamp Timestamp::now()
    {
        return Timestamp(
                    static_cast<int64_t>(
                            std::chrono::duration_cast<std::chrono::microseconds>(
                                    std::chrono::system_clock::now().time_since_epoch()).count()
                    )
                );
    }

    Timestamp Timestamp::invalid() {return Timestamp{};}

    uint64_t Timestamp::microSecondsSinceEpoch() const {return _microSecondsSinceEpoch;}

    Timestamp Timestamp::fromUnix(time_t time)
    {return {static_cast<int64_t>(time) * microSecondsPerSecond};}

    time_t Timestamp::toUnix() const {return static_cast<time_t>(_microSecondsSinceEpoch / microSecondsPerSecond);}

    bool Timestamp::operator<(const ev::Timestamp &rhs) const
    {return _microSecondsSinceEpoch < rhs._microSecondsSinceEpoch;}

    bool Timestamp::operator==(const Timestamp& rhs) const
    {return _microSecondsSinceEpoch == rhs._microSecondsSinceEpoch;}

    uint64_t Timestamp::operator-(const Timestamp& rhs) const
    {return _microSecondsSinceEpoch - rhs._microSecondsSinceEpoch;}

    bool _isvalid_tm_field(time_t value, Timestamp::TimeUnit unit)
    {
        switch (unit)
        {
            case Timestamp::month:
                return (value >= 0 && value < 12);
            case Timestamp::day:
                return (value > 0 && value <= 31);
            case Timestamp::hour:
                return (value >= 0 && value < 24);
            case Timestamp::minute:
                return (value >= 0 && value <= 59);
            case Timestamp::second:
                return (value >= 0 && value <= 60);
            default: return true;
        }
    }
}