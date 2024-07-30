//
// Created by imsac on 19/1/2022.
//

#ifndef DNG_TEMP
#define DNG_TEMP
#include "math.h"
#include "ColorCore.h"

class DngTemp
{
private:
    Vec3 temp;
public:
    Vec3 getXy();
    Vec3 getXYZ();
    Vec3 getTemp();

    explicit DngTemp(Vec3 xy);
    DngTemp(double temp, double tint);
    float &operator () (int n)
    {
        return temp(n);
    }
};


#endif
