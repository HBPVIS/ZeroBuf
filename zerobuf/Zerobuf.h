
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_ZEROBUF_H
#define ZEROBUF_ZEROBUF_H

#include <zeq/api.h>
#include <zerobuf/types.h>
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
    virtual void notifyUpdated() {};

    ZEQ_API const void* getZerobufData() const;
    ZEQ_API size_t getZerobufSize() const;
    ZEQ_API void setZerobufData( const void* data, size_t size );

protected:
    Zerobuf() : _alloc( 0 ) {}
    explicit Zerobuf( Allocator* alloc ) : _alloc( alloc ) {}
    ZEQ_API virtual ~Zerobuf();

    ZEQ_API Zerobuf& operator = ( const Zerobuf& rhs );
    Allocator* getAllocator() { return _alloc; }
    const Allocator* getAllocator() const { return _alloc; }

    ZEQ_API bool _parseJSON( const std::string&, Json::Value& );

private:
    Allocator* const _alloc;

    explicit Zerobuf( const Zerobuf& );
};

}

#endif
