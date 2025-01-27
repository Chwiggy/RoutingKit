#ifndef SEGMENTS_INTERSECT
#define SEGMENTS_INTERSECT

#include <iostream>

// see Franklin Antonio, Ch.IV.6 Faster line intersection. In: Graphic Gems III
namespace RoutingKit {

bool segments_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    // segments have common endpoints
    if ((x1==x3 && y1==y3) || (x1==x4 && y1==y4) || (x2==x3 && y2==y3) || (x2==x4 && y2==y4)) {
        return true;
    }

    float dx12 = x1-x2;
    float dx13 = x1-x3;
    float dx34 = x3-x4;
    float dy12 = y1-y2;
    float dy13 = y1-y3;
    float dy34 = y3-y4;
    float denominator = dx12*dy34 - dy12*dx34;
    float numerator1  = dx13*dy34 - dy13*dx34;
    float numerator2  = dx13*dy12 - dy13*dx12;

    // TODO: micro-optimization: we don't need to caclulate numerator2 before numerator1 is evaluated
        
    bool rv;
    if (denominator >= 0) {
        rv = (0 <= numerator1) && (numerator1 <= denominator)
            && (0 <= numerator2) && (numerator2 <= denominator);
    } else {
        rv = (0 >= numerator1) && (numerator1 >= denominator)
            && (0 >= numerator2) && (numerator2 >= denominator);
    } 
    return rv;
}
}
#endif
