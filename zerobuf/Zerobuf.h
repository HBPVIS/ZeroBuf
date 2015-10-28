
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_ZEROBUF_H
#define ZEROBUF_ZEROBUF_H

#include <zerobuf/api.h>
#include <zerobuf/Types.h>
#include <servus/uint128_t.h>
#include <memory> // std::unique_ptr


namespace Json
{
class Value;
}

namespace zerobuf
{

/**
 * Base class for all zerobufs.
 *
 * Zerobuf objects can serialize/deserialize directly from their member storage
 * and from and to JSON.
 */
class Zerobuf
{
public:
    virtual servus::uint128_t getZerobufType() const = 0;
    virtual Schema getSchema() const = 0;
    virtual void notifyUpdated() {}

    ZEROBUF_API const void* getZerobufData() const;
    ZEROBUF_API size_t getZerobufSize() const;
    ZEROBUF_API void setZerobufData( const void* data, size_t size );

    ZEROBUF_API std::string toJSON() const;
    ZEROBUF_API void fromJSON( const std::string& json );

    ZEROBUF_API bool operator==( const Zerobuf& rhs ) const;
    ZEROBUF_API bool operator!=( const Zerobuf& rhs ) const;

    /* @internal */
    const Allocator* getAllocator() const;

protected:
    Zerobuf();
    explicit Zerobuf( Allocator* alloc ); // takes ownership of alloc
    Zerobuf( const Zerobuf& zerobuf ) = delete;
    ZEROBUF_API virtual ~Zerobuf();

    ZEROBUF_API Zerobuf& operator=( const Zerobuf& rhs );
    Allocator* getAllocator();

    void _setZerobufArray( const void* data, size_t size, size_t arrayNum );

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif
