//
// Created by imsac on 7/1/2022.
//

#ifndef DNG_IMAGE
#define DNG_IMAGE

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdint>

using namespace std;

class GainMap {
public:
    float *gm = nullptr;
    int width;
    int height;

    GainMap();
    ~GainMap();

    GainMap(int w, int h);
    GainMap(const GainMap& gainMap);

    GainMap &operator=(const GainMap &gainMap);

    void release();

    bool empty();

    float &operator()(int x, int y, int c);

};

class DngImg {
public:
    static const int ILLUMINANT_SA = 0;
    static const int ILLUMINANT_SB = 1;
    static const int ILLUMINANT_SC = 2;
    static const int ILLUMINANT_D_50 = 3;
    static const int ILLUMINANT_D_55 = 4;
    static const int ILLUMINANT_D_65 = 5;
    static const int ILLUMINANT_D_75 = 6;

    static const int ORIENTATION_NORMAL = 0;
    static const int ORIENTATION_ROTATE90 = 1;
    static const int ORIENTATION_ROTATE180 = 2;
    static const int ORIENTATION_ROTATE270 = 3;

    static const int CFA_RGGB = 0;
    static const int CFA_BGGR = 1;
    static const int CFA_GRBG = 2;
    static const int CFA_GBRG = 3;

    int width;
    int height;
    int rowStride;
    int dataLength;

    int cfa;
    GainMap gm;
    int gmW;
    int gmH;
    uint32_t whiteLevel;
    uint32_t *blackLevel = nullptr;

    double aperture;                    //lens aperture
    double exposureTime;                //sensor exposition time in seconds
    double focalLength;                 //focal length
    uint32_t iso;                 //sensor iso

    int orientation;                    //sensor orientation

    double *calibration1 = nullptr;
    double *calibration2 = nullptr;
    double *color1 = nullptr;
    double *color2 = nullptr;
    double *forward1 = nullptr;
    double *forward2 = nullptr;
    double* camNeutral = nullptr;
    int illuminant1;
    int illuminant2;

    uint16_t *data = nullptr;

    DngImg();
    DngImg(const DngImg& dngImg);
    ~DngImg();

    DngImg &operator=(const DngImg &dngImg);

    void log();
};

DngImg &dngRead(const string& in);
DngImg &dngRead(void* data, int size);
void dngWrite(DngImg& dngImg, const string& out);
void dngWrite(DngImg& dngImg, uint8_t*& data, uint64_t& len);




#endif