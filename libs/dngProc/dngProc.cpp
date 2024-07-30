#include "dngProc.h"
#include "ColorCorrect.h"
#include <omp.h>

DngImg *dngImge;
Vec3 neutralPoint;
Matriz sensorToIntermediate;

uint16_t getPix(int x, int y)
{
    if(x < 0 || y < 0 || x >= dngImge->width || y >= dngImge->height)
        return 0;
    return dngImge->data[x + y * dngImge->width];
}

int par(int x ,int y)
{
    int a = (x / 2) * 2;
    int b = (y / 2) * 2;
    if(a == x && y == b)
        return 0;
    else if(a != x && y == b)
        return 1;
    else if(a == x && y != b)
        return 2;
    else //if(a != x && y != b)
        return 3;
}

float gm(int px, int py, int channel)
{
    if(dngImge->gm.empty())
        return 1.f;
    GainMap& gm = dngImge->gm;
    float x = (float)px / (float) dngImge->width;
    float y = (float)py / (float) dngImge->height;
    int w = gm.width;
    int h = gm.height;

    int x0 = (int)(x * (float)w);
    int x1 = x0 + 1 > w - 1 ? w - 1 : x0 + 1;
    int y0 = (int)(y * (float)h);
    int y1 = y0 + 1 > h - 1 ? h - 1 : y0 + 1;
    float sx = x * (float)w - float(x0);
    float sy = y * (float)h - float(y0);
    float a = gm(x0, y0, channel);
    float b = gm(x1, y0, channel);
    float c = gm(x0, y1, channel);
    float d = gm(x1, y1, channel);

    float wa = (1.f - sx) * (1.f - sy);
    float wb = (sx      ) * (1.f - sy);
    float wc = (1.f - sx) * (sy      );
    float wd = (sx      ) * (sy      );

    return a * wa + b * wb + c * wc + d * wd;
}

float linearization(int x, int y)
{
    auto color = (float)(getPix(x, y));
    auto blackPoint = (float)(dngImge->blackLevel[par(x, y)]);
    auto whitePoint = (float)(dngImge->whiteLevel);
    color = (color - blackPoint) / (whitePoint - blackPoint) * gm(x, y, par(x, y));
    return color < 0.f ? 0.f : color > 1.f ? 1.f : color;
}

float getMosaic(int x, int y, int a, int b)
{
    int x0 = x + a;
    int y0 = y + b;
    return linearization(x0, y0);
}

void demoBRGGB(int x, int y, Vec3 &color) {
    float r = 0;
    float g = 0;
    float b = 0;
    int p = par(x, y);

    if (p == 0)              //par par
    {
        /* B G B
         * G R G
         * B G B
         * */
        b   += getMosaic(x, y, -1,-1);
        g   += getMosaic(x, y,  0,-1);
        b   += getMosaic(x, y,  1,-1);
        g   += getMosaic(x, y, -1, 0);
        r   += getMosaic(x, y,  0, 0);
        g   += getMosaic(x, y,  1, 0);
        b   += getMosaic(x, y, -1, 1);
        g   += getMosaic(x, y,  0, 1);
        b   += getMosaic(x, y,  1, 1);
        g /= 4.f;
        b /= 4.f;
    }
    else if (p == 1)         //impar par
    {
        /*
         * G B G
         * R G R
         * G B G
         * */
        g    += getMosaic(x, y, -1, -1);
        b    += getMosaic(x, y,  0, -1);
        g    += getMosaic(x, y,  1, -1);
        r    += getMosaic(x, y, -1,  0);
        g    += getMosaic(x, y,  0,  0);
        r    += getMosaic(x, y,  1,  0);
        g    += getMosaic(x, y, -1,  1);
        b    += getMosaic(x, y,  0,  1);
        g    += getMosaic(x, y,  1,  1);
        r /= 2.0;
        g /= 5.0;
        b /= 2.0;
    }
    else if (p == 2)         //par impar
    {
        /*
         * G R G
         * B G B
         * G R G
         * */
        g    += getMosaic(x, y, -1, -1);
        r    += getMosaic(x, y,  0, -1);
        g    += getMosaic(x, y,  1, -1);
        b    += getMosaic(x, y, -1,  0);
        g    += getMosaic(x, y,  0,  0);
        b    += getMosaic(x, y,  1,  0);
        g    += getMosaic(x, y, -1,  1);
        r    += getMosaic(x, y,  0,  1);
        g    += getMosaic(x, y,  1,  1);
        r /= 2.0;
        g /= 5.0;
        b /= 2.0;
    }
    else //if (p == 3)         //impar impar
    {
        /*
         * R G R
         * G B G
         * R G R
         * */
        r    += getMosaic(x, y, -1, -1);
        g    += getMosaic(x, y,  0, -1);
        r    += getMosaic(x, y,  1, -1);
        g    += getMosaic(x, y, -1,  0);
        b    += getMosaic(x, y,  0,  0);
        g    += getMosaic(x, y,  1,  0);
        r    += getMosaic(x, y, -1,  1);
        g    += getMosaic(x, y,  0,  1);
        r    += getMosaic(x, y,  1,  1);
        r /= 4.0;
        g /= 4.0;
    }

    color(0) = r;
    color(1) = g;
    color(2) = b;
}

void demoBBGGR(int x, int y, Vec3 &color) {
    float r = 0;
    float g = 0;
    float b = 0;
    int p = par(x, y);

    if (p == 0)              //par par
    {
        /* R G R
         * G B G
         * R G R
         * */
        r   += getMosaic(x, y, -1,-1);
        g   += getMosaic(x, y,  0,-1);
        r   += getMosaic(x, y,  1,-1);
        g   += getMosaic(x, y, -1, 0);
        b   += getMosaic(x, y,  0, 0);
        g   += getMosaic(x, y,  1, 0);
        r   += getMosaic(x, y, -1, 1);
        g   += getMosaic(x, y,  0, 1);
        r   += getMosaic(x, y,  1, 1);
        g /= 4.f;
        r /= 4.f;
    }
    else if (p == 1)         //impar par
    {
        /*
         * G R G
         * B G B
         * G R G
         * */
        g    += getMosaic(x, y, -1, -1);
        r    += getMosaic(x, y,  0, -1);
        g    += getMosaic(x, y,  1, -1);
        b    += getMosaic(x, y, -1,  0);
        g    += getMosaic(x, y,  0,  0);
        b    += getMosaic(x, y,  1,  0);
        g    += getMosaic(x, y, -1,  1);
        r    += getMosaic(x, y,  0,  1);
        g    += getMosaic(x, y,  1,  1);
        r /= 2.0;
        g /= 5.0;
        b /= 2.0;
    }
    else if (p == 2)         //par impar
    {
        /*
         * G B G
         * R G R
         * G B G
         * */
        g    += getMosaic(x, y, -1, -1);
        b    += getMosaic(x, y,  0, -1);
        g    += getMosaic(x, y,  1, -1);
        r    += getMosaic(x, y, -1,  0);
        g    += getMosaic(x, y,  0,  0);
        r    += getMosaic(x, y,  1,  0);
        g    += getMosaic(x, y, -1,  1);
        b    += getMosaic(x, y,  0,  1);
        g    += getMosaic(x, y,  1,  1);
        r /= 2.0;
        g /= 5.0;
        b /= 2.0;
    }
    else //if (p == 3)         //impar impar
    {
        /*
         * B G B
         * G R G
         * B G B
         * */
        b    += getMosaic(x, y, -1, -1);
        g    += getMosaic(x, y,  0, -1);
        b    += getMosaic(x, y,  1, -1);
        g    += getMosaic(x, y, -1,  0);
        r    += getMosaic(x, y,  0,  0);
        g    += getMosaic(x, y,  1,  0);
        b    += getMosaic(x, y, -1,  1);
        g    += getMosaic(x, y,  0,  1);
        b    += getMosaic(x, y,  1,  1);
        b /= 4.0;
        g /= 4.0;
    }

    color(0) = r;
    color(1) = g;
    color(2) = b;
}

void applyCS(Vec3 &color)
{
    color = clamp(color, 0.f, neutralPoint);
    color = color * sensorToIntermediate;
    color = clamp(color, 0.f, 1.f);
}

void saturation(Vec3 &color, float sat)
{
    float wp = (color(0) / 3.f + color(1) / 3.f + color(2) / 3.f);

     color(0) = (color(0) - wp) * sat + wp;
     color(1) = (color(1) - wp) * sat + wp;
     color(2) = (color(2) - wp) * sat + wp;
}

void gain(Vec3 &c, float curve)
{
    c(0) = pow(c(0), 1.f / curve);
    c(1) = pow(c(1), 1.f / curve);
    c(2) = pow(c(2), 1.f / curve);
}

void setCS()
{
    neutralPoint = Vec3(dngImge->camNeutral);
    CC color(*dngImge);
    DngTemp temp = color.cameraNeutralToXy();
    Matriz camToPcs = color.cameraToPcs(temp);
//    Matriz pcsToSrgb = color.pcsToSrgb();
    Matriz pcsToSrgb = CC::pcsToSrgb();

    sensorToIntermediate = pcsToSrgb * camToPcs;
}

/*
uint8_t *processRaw(DngImg &dngImg, double gamma, double sat)
{
    dngImge = &dngImg;
    setCS();
    int rowStride = (dngImge->width) * 3;
    int len = rowStride * (dngImge->height);

    auto *img = (uint8_t*) malloc(len * sizeof (uint8_t));

    int threads = omp_get_num_threads();
    omp_set_num_threads(threads);
#pragma omp parallel for
    for(int y = 0; y < dngImg.height; y++) {
        for (int x = 0; x < dngImg.width; ++x) {
            
        Vec3 color(0.);
        if(dngImge->cfa == DngImg::CFA_RGGB)
            demoBRGGB(x, y, color);
        else
            demoBBGGR(x, y, color);

        applyCS(color);
        saturation(color, sat);
        gain(color, gamma);
        img[x * 3 + 0 + y * rowStride] = static_cast<uint8_t> (color(0) * 255);
        img[x * 3 + 1 + y * rowStride] = static_cast<uint8_t> (color(1) * 255);
        img[x * 3 + 2 + y * rowStride] = static_cast<uint8_t> (color(2) * 255);
        }
    }

    /*for (int y = 0; y < dngImge->height; ++y) {
        for (int x = 0; x < dngImge->width; ++x) {
            Vec3 color(0.);
            if(dngImge->cfa == RGGB){
                demoBRGGB(getPixA, x, y, color);
            }
            else {
                demoBBGGR(getPixA, x, y, color);
            }
            applyCS(color);
//        saturation(color, 1.2);
            gain(color, 2.2f);
            img[x * 3 + 0 + y * rowStride] = static_cast<uint8_t> (color(0) * 255);
            img[x * 3 + 1 + y * rowStride] = static_cast<uint8_t> (color(1) * 255);
            img[x * 3 + 2 + y * rowStride] = static_cast<uint8_t> (color(2) * 255);
            color.release();
        }
    }*

    return img;
}

*/

/*
uint8_t *processRaw(DngImg &dngImg, double gamma, double sat)
{
    dngImge = &dngImg;
    setCS();
    int rowStride = (dngImge->width) * 3;
    int len = rowStride * (dngImge->height);

    auto *img = (uint8_t*) malloc(len * sizeof (uint8_t));

    XyFun f = [&img, rowStride, gamma, sat](int x, int y)
    {
        Vec3 color(0.);
        if(dngImge->cfa == RGGB)
            demoBRGGB(x, y, color);
        else
            demoBBGGR(x, y, color);

        applyCS(color);
        saturation(color, sat);
        gain(color, gamma);
        img[x * 3 + 0 + y * rowStride] = static_cast<uint8_t> (color(0) * 255);
        img[x * 3 + 1 + y * rowStride] = static_cast<uint8_t> (color(1) * 255);
        img[x * 3 + 2 + y * rowStride] = static_cast<uint8_t> (color(2) * 255);
        color.release();
    };

    Proc proc(8);
    proc.compute(dngImge->width, dngImge->height, f);

    return img;
}

*/