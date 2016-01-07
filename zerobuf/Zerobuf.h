
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 *                          grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_ZEROBUF_H
#define ZEROBUF_ZEROBUF_H

#include <zerobuf/api.h>
#include <zerobuf/types.h>
#include <servus/uint128_t.h>

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
    /** @return the type of this object, hashed from its schema. */
    virtual uint128_t getZerobufType() const = 0;

    /** @return the static size of this object in bytes. */
    virtual size_t getZerobufStaticSize() const = 0;

    /** @return the number of dynamics fields of this object. */
    virtual size_t getZerobufNumDynamics() const = 0;

    /** @return the schemas describing this ZeroBuf object. */
    virtual Schemas getSchemas() const = 0;

    /** Called if any data in this object is about to change. */
    virtual void notifyChanging() {}

    /** Called by ZeroEQ subscriber upon receive() of this object. */
    virtual void notifyReceived() {}

    /** @return the pointer to the zerobuf. */
    ZEROBUF_API const void* getZerobufData() const;

    /** @return the size of the zerobuf. */
    ZEROBUF_API size_t getZerobufSize() const;

    /**
     * Remove unused holes from the zerobuf.
     *
     * Compaction only occurs if the current allocation exceeds the relative
     * threshold to the optimal size, that is, if unusedMemory divided by
     * optimalMemory is above the threshold.
     *
     * @param threshold the compaction threshold
     */
    ZEROBUF_API virtual void compact( float threshold = 0.1f );

    /** Copy the raw data into the zerobuf. */
    ZEROBUF_API void copyZerobufData( const void* data, size_t size );

    /** Convert the given instance to a JSON representation. */
    ZEROBUF_API std::string toJSON() const;

    /**
     * Convert the given JSON string into this object.
     *
     * When a parse error occurs, the object may be partially updated by the
     * already parsed data.
     *
     * @return true on success, false on error.
     */
    ZEROBUF_API bool fromJSON( const std::string& json );

    /** Assignment operator. */
    ZEROBUF_API Zerobuf& operator = ( const Zerobuf& rhs );

    /** Move ctor. */
    ZEROBUF_API Zerobuf( Zerobuf&& rhs );

    /** Move operator. May copy data if zerobuf is not movable */
    ZEROBUF_API Zerobuf& operator = ( Zerobuf&& rhs );

    /** @return true if both objects contain the same data */
    ZEROBUF_API bool operator == ( const Zerobuf& rhs ) const;

    /** @return true if both objects contain different data */
    ZEROBUF_API bool operator != ( const Zerobuf& rhs ) const;

    /** @internal */
    void reset( AllocatorPtr allocator ) { _allocator.swap( allocator ); }

    /** @internal Check consistency of zerobuf */
    ZEROBUF_API void check() const;

protected:
    explicit Zerobuf( AllocatorPtr alloc ); // takes ownership of alloc
    ZEROBUF_API virtual ~Zerobuf();

    // used by generated ZeroBuf objects
    ZEROBUF_API const Allocator& getAllocator() const;
    ZEROBUF_API Allocator& getAllocator();

    ZEROBUF_API void _copyZerobufArray( const void* data, size_t size,
                                        size_t arrayNum );
private:
    AllocatorPtr _allocator;

    Zerobuf() = delete;
    Zerobuf( const Zerobuf& zerobuf ) = delete;
};

inline std::ostream& operator << ( std::ostream& os, const Zerobuf& zerobuf )
{
    return os << zerobuf.getZerobufType() << ": " << zerobuf.toJSON();
}

}

#endif
