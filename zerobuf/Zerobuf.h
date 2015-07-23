
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_ZEROBUF_H
#define ZEROBUF_ZEROBUF_H

#include <zerobuf/api.h>
#include <zerobuf/Types.h>
#include <zerobuf/jsoncpp/json/json-forwards.h>

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
    virtual void notifyUpdated() {}

    ZEROBUF_API const void* getZerobufData() const;
    ZEROBUF_API size_t getZerobufSize() const;
    ZEROBUF_API void setZerobufData( const void* data, size_t size );

protected:
    Zerobuf() : _alloc( 0 ) {}
    explicit Zerobuf( Allocator* alloc ) : _alloc( alloc ) {}
    ZEROBUF_API virtual ~Zerobuf();

    ZEROBUF_API Zerobuf& operator = ( const Zerobuf& rhs );
    Allocator* getAllocator() { return _alloc; }
    const Allocator* getAllocator() const { return _alloc; }

    ZEROBUF_API bool _parseJSON( const std::string&, Json::Value& );

private:
    Allocator* const _alloc;

    explicit Zerobuf( const Zerobuf& );
};

}

#endif
