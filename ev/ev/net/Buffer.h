#ifndef EV_BUFFER_H
#define EV_BUFFER_H

#include <algorithm>
#include <vector>
#include <cassert>
#include <cstring>
#include <string>
#include "utils/noncopyable.h"
#include "Endian.h"

namespace ev::net
{
    class Buffer : public noncopyable
    {
    public:
        static const size_t CheapPrepend = 8;
        static const size_t InitialSize = 1024;

        explicit Buffer(size_t initialSize = InitialSize);
        Buffer(Buffer&& buffer) noexcept;

        void swap(Buffer& rhs);
        [[nodiscard]] size_t readableBytes() const;
        [[nodiscard]] size_t writableBytes() const;
        [[nodiscard]] size_t prependableBytes() const;
        [[nodiscard]] const char* peek() const;

        void retrieve(size_t len);
        void retrieveUntil(const char* end);
        void retrieveInt64();
        void retrieveInt32();
        void retrieveInt16();
        void retrieveInt8();
        void retrieveAll();
        std::string retrieveAllAsString();
        std::string retrieveAsString(size_t len);

        void append(const std::string_view& str);
        void append(const void* data, size_t len);
        void append(const char* data, size_t len);

        void ensureWritableBytes(size_t len);
        char* beginWrite();
        [[nodiscard]] const char* beginWrite() const;
        void hasWritten(size_t len);
        void unWrite(size_t len);

        void appendInt64(int64_t x);
        void appendInt32(int32_t x);
        void appendInt16(int16_t x);
        void appendInt8(int8_t x);
        void prependInt64(int64_t x);
        void prependInt32(int32_t x);
        void prependInt16(int16_t x);
        void prependInt8(int8_t x);
        void prepend(const void* /*restrict*/ data, size_t len);

        int64_t readInt64();
        int32_t readInt32();
        int16_t readInt16();
        int8_t readInt8();
        [[nodiscard]] int64_t peekInt64() const;
        [[nodiscard]] int32_t peekInt32() const;
        [[nodiscard]] int16_t peekInt16() const;
        [[nodiscard]] int8_t peekInt8() const;


        void shrink(size_t reserve);
        [[nodiscard]] size_t internalCapacity() const;
        ssize_t readFd(int fd, int* savedErrno);

    private:
        char* begin();
        [[nodiscard]] const char* begin() const;
        void makeSpace(size_t len);

    private:
        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;
    };

}

#endif  // EV_BUFFER_H
