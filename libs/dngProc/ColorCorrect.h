//
// Created by imsac on 13/7/2022.
//

#ifndef IMA_COLORCORRECT_H
#define IMA_COLORCORRECT_H

#include "DngImage.h"
#include "DngTemp.h"
#include "ColorCore.h"




class CC {
public:
    Illuminant illuminant1;
    Illuminant illuminant2;
    Matriz *calibration1 = nullptr;
    Matriz *calibration2 = nullptr;
    Matriz colorMatrix1;
    Matriz colorMatrix2;
    Matriz *forward1 = nullptr;
    Matriz *forward2 = nullptr;
    Vec3 neutralColorPoint;
    Matriz sensor2Rgb;

    inline explicit CC(DngImg &dngImg) {
        Illuminant i1 = getIll(dngImg.illuminant1);
        Illuminant i2 = getIll(dngImg.illuminant2);

        neutralColorPoint = Vec3(dngImg.camNeutral);

        if (i1.temp > i2.temp) {
            if (dngImg.calibration1 != nullptr) {
                calibration1 = new Matriz(dngImg.calibration2);
                calibration2 = new Matriz(dngImg.calibration1);
            }
            colorMatrix1 = dngImg.color2;
            colorMatrix2 = dngImg.color1;
            if (dngImg.forward1 != nullptr) {
                forward1 = new Matriz(dngImg.forward2);
                forward2 = new Matriz(dngImg.forward1);
            }
            illuminant1 = i2;
            illuminant2 = i1;
        }
        else
        {
            if (dngImg.calibration1 != nullptr) {
                this->calibration1 = new Matriz(dngImg.calibration1);
                this->calibration2 = new Matriz(dngImg.calibration2);
            }
            this->colorMatrix1 = dngImg.color1;
            this->colorMatrix2 = dngImg.color2;
            if (dngImg.forward1 != nullptr) {
                this->forward1 = new Matriz(dngImg.forward1);
                this->forward2 = new Matriz(dngImg.forward2);
            }
            this->illuminant1 = i1;
            this->illuminant2 = i2;
        }
    }

    inline CC(Illuminant illuminant1,
       Illuminant illuminant2,
       Matriz calibration1,
       Matriz calibration2,
       Matriz color1,
       Matriz color2,
       Matriz forward1,
       Matriz forward2,
       Vec3 neutralColorPoint) {
        this->neutralColorPoint = neutralColorPoint;
        if (illuminant1.temp > illuminant2.temp) {
            this->calibration1 = &calibration2;
            this->calibration2 = &calibration1;
            this->colorMatrix1 = color2;
            this->colorMatrix2 = color1;
            this->forward1 = &forward2;
            this->forward2 = &forward1;
            this->illuminant1 = illuminant2;
            this->illuminant2 = illuminant1;
        } else {
            this->calibration1 = &calibration1;
            this->calibration2 = &calibration2;
            this->colorMatrix1 = color1;
            this->colorMatrix2 = color2;
            this->forward1 = &forward1;
            this->forward2 = &forward2;
            this->illuminant1 = illuminant1;
            this->illuminant2 = illuminant2;
        }
    }

    inline Matriz cameraToPcs(float temp, float tint) {
        DngTemp xy(temp, tint);
        return cameraToPcs(xy);
    }

    inline Matriz cameraToPcs(DngTemp xy) {
        float t1 = illuminant1.temp;
        float t2 = illuminant2.temp;
        float t = xy.getTemp()(0);
        float g;
        if (t <= t1)
            g = 1.f;
        else if (t >= t2)
            g = 0.f;
        else {
            float invT = 1.f / t;
            g = (invT - (1.f / t2)) / ((1.f / t1) - (1.f / t2));
        }

        Matriz outputTransform;
        Matriz colorM1 = normalizeCM(colorMatrix1);
        Matriz colorM2 = normalizeCM(colorMatrix2);
        Matriz colorM = mix(colorM1, colorM2, g);

        Matriz cat = chromaticAdaptation_xy(I_D50.xyz, xy.getXy(), bradford);
        Matriz XYZToCameraD50 = colorM * cat;
        outputTransform = XYZToCameraD50.inverse();

        return outputTransform;
    }

    inline static Matriz pcsToSrgb() {
        Matriz srgbToPcs {0.4361f, 0.3851f, 0.1431f,
                         0.2225f, 0.7169f, 0.0606f,
                         0.0139f, 0.0971f, 0.7141f};

        Vec3 w1 = Vec3(1.);
        Vec3 w2 = xy2XYZ(I_D50.xyz);
        Matriz s;

        s(0) = w2(0) / w1(0);
        s(4) = w2(1) / w1(1);
        s(8) = w2(2) / w1(2);

        Matriz outSrgbToPcs = s * srgbToPcs;
        Matriz outPcsToSrgb = outSrgbToPcs.inverse();
        return outPcsToSrgb;
    }

    inline DngTemp cameraNeutralToXy() {
        Vec3 asShot = neutralColorPoint;
        float t1 = illuminant1.temp;
        float t2 = illuminant2.temp;
        float max = Vmax(asShot);
        Vec3 cameraNeutralVector;
        cameraNeutralVector(0) = asShot(0) * (1.f / max);
        cameraNeutralVector(1) = asShot(1) * (1.f / max);
        cameraNeutralVector(2) = asShot(2) * (1.f / max);

        int maxIters = 30;
        DngTemp last(I_D50.xyz);

        for (int i = 0; i < maxIters; i++) {
            float t = last.getTemp()(0);
            float invT = 1.f / t;
            float g =
                    t <= t1 ? 1.f :
                    t >= t2 ? 0.f :
                    (invT - (1.f / t2)) / ((1.f / t1) - (1.f / t2));

            Matriz color = mix(normalizeCM(colorMatrix1), normalizeCM(colorMatrix2), g);
            Matriz xyzToCamera = color;

            Vec3 r = cameraNeutralVector * xyzToCamera.inverse();
            DngTemp next(XYZ2xy(r));

            if (abs(next(0) - last(0)) + abs(next(1) - last(1)) < 0.0000001)
                return next;

            if (i == maxIters - 1) {
                next(0) = (last(0) + next(0)) * 0.5f;
                next(1) = (last(1) + next(1)) * 0.5f;
            }
            last = next;
        }
        return last;
    }

    inline static Matriz mapWhiteMatrix(DngTemp white1, DngTemp white2) {

        Vec3 w1Mat = white1.getXYZ() * bradford;
        Vec3 w2Mat = white2.getXYZ() * bradford;

        Vec3 w1 = w1Mat;
        Vec3 w2 = w2Mat;

        // Negative white coordinates are kind of meaningless.
        w1 = max(w1, 0.f);
        w2 = max(w2, 0.f);

        Matriz a(0.f);

        Vec3 w(0.f);
        w(0) = w1(0) > 0.f ? w2(0) / w1(0) : 10.f;
        w(1) = w1(1) > 0.f ? w2(1) / w1(1) : 10.f;
        w(2) = w1(2) > 0.f ? w2(2) / w1(2) : 10.f;
        clamp(w, 0.1f, 10.f);

        a(0) = w(0);
        a(4) = w(1);
        a(8) = w(2);

        return (((Matriz)bradford).inverse() * a) * bradford;
    }

    inline static Matriz normalizeCM(const Matriz& colorMatrix) {
        Vec3 tmp = Vmap(colorMatrix, xy2XYZ(I_D50.xyz));
        float maxVal = Vmax(tmp);
        return colorMatrix * (1.f / maxVal);
    }

    inline static Matriz normalizeFM(const Matriz& forwardMatrix) {
        Vec3 tmp(1.);
        Vec3 xyz;
        xyz(0) = tmp(0) * forwardMatrix(0) + tmp(1) * forwardMatrix(1) + tmp(2) * forwardMatrix(2);
        xyz(1) = tmp(0) * forwardMatrix(3) + tmp(1) * forwardMatrix(4) + tmp(2) * forwardMatrix(5);
        xyz(2) = tmp(0) * forwardMatrix(6) + tmp(1) * forwardMatrix(7) + tmp(2) * forwardMatrix(8);
        Matriz m{
                1.f / xyz(0), 0, 0,
                0, 1.f / xyz(1), 0,
                0, 0, 1.f / xyz(2)
        };
        Matriz intermediate = m * forwardMatrix;
        Matriz m2{
                xy2XYZ(I_D50.xyz)(0), 0, 0,
                0, xy2XYZ(I_D50.xyz)(1), 0,
                0, 0, xy2XYZ(I_D50.xyz)(2)
        };
        return m2 * intermediate;
    }
};


#endif //IMA_COLORCORRECT_H
