
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 *                          grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_ZEROBUF_H
#define ZEROBUF_ZEROBUF_H

#include <zerobuf/api.h>
#include <zerobuf/Allocator.h> // MSVC needs it for std::unique_ptr
#include <zerobuf/types.h>
#include <zerobuf/json.h> // friend
#include <servus/serializable.h> // base class
#include <servus/uint128_t.h> // used inline in operator <<

namespace zerobuf
{

/**
 * Base class for all Zerobuf serializable objects.
 *
 * The zerobufCxx.py code generator creates subclasses of this class. Zerobuf
 * objects can serialize/deserialize directly from their member storage and from
 * and to JSON. All members are zero-initialized, unless they do have a default
 * value. fromJSON() will compact the Zerobuf.
 */
class Zerobuf : public servus::Serializable
{
public:
    ZEROBUF_API virtual ~Zerobuf();

    /** @return the static size of this object in bytes. */
    virtual size_t getZerobufStaticSize() const = 0;

    /** @return the number of dynamics fields of this object. */
    virtual size_t getZerobufNumDynamics() const = 0;

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
    ZEROBUF_API void reset( AllocatorPtr allocator );

    /** @internal Check consistency of zerobuf */
    ZEROBUF_API void check() const;

protected:
    ZEROBUF_API explicit Zerobuf( AllocatorPtr alloc ); // takes ownership of alloc

    /** Called if any data in this object has changed. */
    ZEROBUF_API virtual void notifyChanged() {}

    // used by generated ZeroBuf objects
    ZEROBUF_API const Allocator& getAllocator() const;
    ZEROBUF_API Allocator& getAllocator();

    ZEROBUF_API void _copyZerobufArray( const void* data, size_t size,
                                        size_t arrayNum );

    ZEROBUF_API virtual void _parseJSON( const Json::Value& json );
    ZEROBUF_API virtual void _createJSON( Json::Value& json ) const;
    ZEROBUF_API friend void fromJSON( const Json::Value&,Zerobuf& );
    ZEROBUF_API friend void toJSON( const Zerobuf&, Json::Value& );

    ZEROBUF_API bool _fromBinary( const void* data, const size_t size ) override;

private:
    AllocatorPtr _allocator;

    Zerobuf() = delete;
    Zerobuf( const Zerobuf& zerobuf ) = delete;

    ZEROBUF_API Data _toBinary() const final;
    ZEROBUF_API bool _fromJSON( const std::string& json ) final;
    ZEROBUF_API std::string _toJSON() const final;
};

inline std::ostream& operator << ( std::ostream& os, const Zerobuf& zerobuf )
{
    return os << "\"" << zerobuf.getTypeName() << "\" : " << zerobuf.toJSON();
}

}

#endif
