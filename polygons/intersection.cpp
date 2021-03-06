// This code is based on http://geomalgorithms.com/a03-_inclusion.html
// which is distributed under the following license:

// Copyright 2000 softSurfer, 2012 Dan Sunday
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

#include "intersection.h"

// tests if a point is left|on|right of an infinite line
// input:  three points p0, p1, and p2
// return: > 0 for p2 left of the line through p0 and p1
//         = 0 for p2 on the line
//         < 0 for p2 right of the line
inline double is_left(const double p0x,
                      const double p0y,
                      const double p1x,
                      const double p1y,
                      const double p2x,
                      const double p2y)
{
    return ((p1x - p0x) * (p2y - p0y) - (p2x - p0x) * (p1y - p0y));
}

bool crosses(const double px, const double py, const edge e)
{
    if (py > std::max(e.p1.y, e.p2.y))
        return false;
    if (py < std::min(e.p1.y, e.p2.y))
        return false;

    if (e.p1.y < e.p2.y)
    {
        // upward edge
        return (is_left(e.p1.x, e.p1.y, e.p2.x, e.p2.y, px, py) > 0.0);
    }
    else
    {
        // downward edge
        return (is_left(e.p1.x, e.p1.y, e.p2.x, e.p2.y, px, py) < 0.0);
    }
}
