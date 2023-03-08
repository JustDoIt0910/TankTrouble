#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>

namespace ev::net
{
    const size_t Buffer::CheapPrepend;
    const size_t Buffer::InitialSize;

    Buffer::Buffer(size_t initialSize)
    : buffer_(CheapPrepend + initialSize),
    readerIndex_(CheapPrepend),
    writerIndex_(CheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == CheapPrepend);
    }

    Buffer::Buffer(Buffer&& buffer) noexcept
        :readerIndex_(CheapPrepend),
        writerIndex_(CheapPrepend) { swap(buffer); }

    void Buffer::swap(Buffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    size_t Buffer::readableBytes() const { return writerIndex_ - readerIndex_; }

    size_t Buffer::writableBytes() const { return buffer_.size() - writerIndex_; }

    size_t Buffer::prependableBytes() const { return readerIndex_; }

    const char* Buffer::peek() const { return begin() + readerIndex_; }

    void Buffer::retrieve(size_t len)
    {
        assert(len <= readableBytes());
        if (len < readableBytes())
            readerIndex_ += len;
        else
            retrieveAll();
    }

    void Buffer::retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void Buffer::retrieveInt64() {retrieve(sizeof(int64_t));}

    void Buffer::retrieveInt32() {retrieve(sizeof(int32_t));}

    void Buffer::retrieveInt16() {retrieve(sizeof(int16_t));}

    void Buffer::retrieveInt8() {retrieve(sizeof(int8_t));}

    void Buffer::retrieveAll()
    {
        readerIndex_ = CheapPrepend;
        writerIndex_ = CheapPrepend;
    }

    std::string Buffer::retrieveAllAsString() {return retrieveAsString(readableBytes());}

    std::string Buffer::retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void Buffer::append(const std::string_view& str) {append(str.data(), str.size());}

    void Buffer::append(const void* data, size_t len) {append(static_cast<const char*>(data), len);}

    void Buffer::append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    void Buffer::ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char* Buffer::beginWrite() { return begin() + writerIndex_; }

    const char* Buffer::beginWrite() const { return begin() + writerIndex_; }

    void Buffer::hasWritten(size_t len)
    {
        assert(len <= writableBytes());
        writerIndex_ += len;
    }

    void Buffer::unWrite(size_t len)
    {
        assert(len <= readableBytes());
        writerIndex_ -= len;
    }

    void Buffer::appendInt64(int64_t x)
    {
        uint64_t be64 = hostToNetwork64(x);
        append(&be64, sizeof be64);
    }

    void Buffer::appendInt32(int32_t x)
    {
        uint32_t be32 = hostToNetwork32(x);
        append(&be32, sizeof be32);
    }

    void Buffer::appendInt16(int16_t x)
    {
        uint16_t be16 = hostToNetwork16(x);
        append(&be16, sizeof be16);
    }

    void Buffer::appendInt8(int8_t x)
    {
        append(&x, sizeof x);
    }

    int64_t Buffer::readInt64()
    {
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }

    int32_t Buffer::readInt32()
    {
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }

    int16_t Buffer::readInt16()
    {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }

    int8_t Buffer::readInt8()
    {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    int64_t Buffer::peekInt64() const
    {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, peek(), sizeof be64);
        return static_cast<int64_t>(networkToHost64(be64));
    }

    int32_t Buffer::peekInt32() const
    {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof be32);
        return static_cast<int32_t>(networkToHost32(be32));
    }

    int16_t Buffer::peekInt16() const
    {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        return static_cast<int16_t>(networkToHost16(be16));
    }

    int8_t Buffer::peekInt8() const
    {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }

    void Buffer::prependInt64(int64_t x)
    {
        uint64_t be64 = hostToNetwork64(x);
        prepend(&be64, sizeof be64);
    }

    void Buffer::prependInt32(int32_t x)
    {
        uint32_t be32 = hostToNetwork32(x);
        prepend(&be32, sizeof be32);
    }

    void Buffer::prependInt16(int16_t x)
    {
        uint16_t be16 = hostToNetwork16(x);
        prepend(&be16, sizeof be16);
    }

    void Buffer::prependInt8(int8_t x) {prepend(&x, sizeof x);}

    void Buffer::prepend(const void* data, size_t len)
    {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d+len, begin()+readerIndex_);
    }

    void Buffer::shrink(size_t reserve)
    {
        buffer_.resize(reserve);
        buffer_.shrink_to_fit();
    }

    size_t Buffer::internalCapacity() const {return buffer_.capacity();}

    ssize_t Buffer::readFd(int fd, int* savedErrno)
    {
        // saved an ioctl()/FIONREAD call to tell how much to read
        char extrabuf[65536];
        struct iovec vec[2];
        const size_t writable = writableBytes();
        vec[0].iov_base = begin()+writerIndex_;
        vec[0].iov_len = writable;
        vec[1].iov_base = extrabuf;
        vec[1].iov_len = sizeof extrabuf;
        // when there is enough space in this buffer, don't read into extrabuf.
        // when extrabuf is used, we read 128k-1 bytes at most.
        const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
        const ssize_t n = ::readv(fd, vec, iovcnt);
        if (n < 0)
            *savedErrno = errno;
        else if (static_cast<size_t>(n) <= writable)
            writerIndex_ += n;
        else
        {
            writerIndex_ = buffer_.size();
            append(extrabuf, n - writable);
        }
        return n;
    }

    char* Buffer::begin() { return &*buffer_.begin(); }

    const char* Buffer::begin() const { return &*buffer_.begin(); }

    void Buffer::makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + CheapPrepend)
        {
            // FIXME: move readable data
            buffer_.resize(writerIndex_+len);
        }
        else
        {
            // move readable data to the front, make space inside buffer
            assert(CheapPrepend < readerIndex_);
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + CheapPrepend);
            readerIndex_ = CheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }
}