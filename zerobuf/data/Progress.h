
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_DATA_PROGRESS_H
#define ZEROBUF_DATA_PROGRESS_H

#include <zerobuf/types.h>
#include <zerobuf/data/api.h>
#include <zerobuf/data/detail/progress.h> // base class

namespace zerobuf
{
namespace data
{
/** Drop-in progress meter for boost::progress_display. */
class Progress : public detail::Progress
{
public:
    ZEROBUFDATA_API explicit Progress( unsigned long expected );
    ZEROBUFDATA_API Progress( const std::string& operation,
                              unsigned long expected );

    ZEROBUFDATA_API void restart( unsigned long expected );
    ZEROBUFDATA_API unsigned long operator+=( unsigned long inc );
    unsigned long operator++() { return operator += ( 1 ); }
    unsigned long count() const { return _count; }

private:
    unsigned long _update();

    unsigned long _expected;
    unsigned long _count;
};
}
}
#endif
