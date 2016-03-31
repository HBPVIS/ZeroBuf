
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include "Progress.h"

namespace zerobuf
{
namespace data
{
Progress::Progress( const unsigned long expected )
    : _expected( expected )
    , _count( 0 )
{
    _update();
}

Progress::Progress( const std::string& operation, unsigned long expected )
    : detail::Progress( operation, 0.f )
    , _expected( expected )
    , _count( 0 )
{
    _update();
}

void Progress::restart( const unsigned long expected )
{
    _expected = expected;
    _count = 0;
    _update();
}

unsigned long Progress::operator+=( unsigned long inc )
{
    _count += inc;
    return _update();
}

unsigned long Progress::_update()
{
    _count = std::min( _count, _expected );
    const float amount = _expected == 0 ? 1.f :
                         float( _count ) / float( _expected );
    if( amount == getAmount( ))
        return _count;

    setAmount( amount );
    return _count;
}

}
}
