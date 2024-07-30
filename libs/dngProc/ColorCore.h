
#ifndef COLOR_CORE
#define COLOR_CORE

#include "matrix_vector.h"
#include <cmath>


struct Illuminant
{
    float temp;
    Vec3 xyz;
};

static const Illuminant I_SA{2856.f, Vec3(0.447570f, 0.407450f, 1.f - (0.447570f + 0.407450f))};
static const Illuminant I_SB{4874.f, Vec3(0.348420f, 0.351610f, 1.f - (0.348420f + 0.351610f))};
static const Illuminant I_SC{6774.f, Vec3(0.310060f, 0.316160f, 1.f - (0.310060f + 0.316160f))};
static const Illuminant I_D50{5003.f, Vec3(0.345670f, 0.358500f, 1.f - (0.345670f + 0.358500f))};
static const Illuminant I_D55{5503.f, Vec3(0.332420f, 0.347430f, 1.f - (0.332420f + 0.347430f))};
static const Illuminant I_D65{6504.f, Vec3(0.312710f, 0.329020f, 1.f - (0.312710f + 0.329020f))};
static const Illuminant I_D75{7504.f, Vec3(0.299020f, 0.314850f, 1.f - (0.299020f + 0.314850f))};

static Illuminant getIll(int i) {
    switch (i) {
        case 0:
            return I_SA;
        case 1:
            return I_SB;
        case 2:
            return I_SC;
        case 3:
            return I_D50;
        case 4:
            return I_D55;
        case 5:
            return I_D65;
        case 6:
            return I_D75;
        default:
            return I_D50;
    }
}


static Vec3 xy2XYZ(const Vec3& xyCoord) {
    Vec3 temp = clamp(xyCoord, 0.000001f, 0.999999f);

    if (temp(0) + temp(1) > 0.999999f) {
        float scale = 0.999999f / (temp(0) + temp(1));
        temp = temp * scale;
    }
    return {
            temp(0) / temp(1),
            1.f,
            (1.f - temp(0) - temp(1)) / temp(1)
    };
}
static Vec3 XYZ2xy(Vec3& coord) {
    float X = coord(0);
    float Y = coord(1);
    float Z = coord(2);
    float total = X + Y + Z;
    return {
            X / total,
            Y / total,
            Z / total
    };
}

static Matriz getXYZ(float rx, float ry, float gx, float gy, float bx, float by, float wx, float wy) {
        float rz = 1.f - (rx + ry);
        float gz = 1.f - (gx + gy);
        float bz = 1.f - (bx + by);
        float wz = 1.f - (wx + wy);

        float wxScaled = wx / wy;
        float wyScaled = 1;
        float wzScaled = wz / wy;
        Vec3 wMatrix = {wxScaled, wyScaled, wzScaled};
        Matriz xyz = {
                rx, gx, bx,
                ry, gy, by,
                rz, gz, bz
        };
        Matriz xyz1 = xyz.inverse();
//            Vec3 achromaticVector = xyz1 * wMatrix;
        Vec3 achromaticVector = wMatrix * xyz1;

        Matriz aMat = {
                achromaticVector(0), 0, 0,
                0, achromaticVector(1), 0,
                0, 0, achromaticVector(2),
        };

        return xyz * aMat;
    }
static Matriz chromaticAdaptation_XYZ(Vec3& ws, Vec3& wd, Matriz m) {
        Vec3 S = ws * m;
        Vec3 D = wd * m;
        Matriz coneResponseDomain = {
                D(0) / S(0), 0, 0,
                0, D(1) / S(1), 0,
                0, 0, D(2) / S(2),
        };
        Matriz M = (m.inverse() * coneResponseDomain) * m;
        return M;
    }
static Matriz chromaticAdaptation_xy(Vec3 ws, Vec3 wd, Matriz m) {
        Vec3 wsScaled = xy2XYZ(ws);
        Vec3 wdScaled = xy2XYZ(wd);
        Vec3 S = wsScaled * m;
        Vec3 D = wdScaled * m;
        Matriz coneResponseDomain{
                D(0) / S(0), 0, 0,
                0, D(1) / S(1), 0,
                0, 0, D(2) / S(2),
        };
        Matriz M = (m.inverse() * (coneResponseDomain)) * (m);
        return M;
    }

static const Matriz xyzScaling1(0.f, 0.f, 1.f,
                                0.f, 1.f, 0.f,
                                1.f, 0.f, 0.f);
static const Matriz xyzScaling(0.f, 0.f, 1.f,
                               0.f, 1.f, 0.f,
                               1.f, 0.f, 0.f);
static const Matriz bradford(0.895100f, 0.266400f, -0.161400f,
                             -0.750200f, 1.713500f, 0.036700f,
                             0.038900f, -0.068500f, 1.029600f);

//const Illuminant Illuminant::I_SA(2856.f, Vec3(0.447570f, 0.407450f, 1.f - (0.447570f + 0.407450f)));
//const Illuminant Illuminant::I_SB(4874.f, Vec3(0.348420f, 0.351610f, 1.f - (0.348420f + 0.351610f)));
//const Illuminant Illuminant::I_SC(6774.f, Vec3(0.310060f, 0.316160f, 1.f - (0.310060f + 0.316160f)));
//const Illuminant Illuminant::I_D50(5003.f, Vec3(0.345670f, 0.358500f, 1.f - (0.345670f + 0.358500f)));
//const Illuminant Illuminant::I_D55(5503.f, Vec3(0.332420f, 0.347430f, 1.f - (0.332420f + 0.347430f)));
//const Illuminant Illuminant::I_D65(6504.f, Vec3(0.312710f, 0.329020f, 1.f - (0.312710f + 0.329020f)));
//const Illuminant Illuminant::I_D75(7504.f, Vec3(0.299020f, 0.314850f, 1.f - (0.299020f + 0.314850f)));

//#define bradford Matriz(0.895100f, 0.266400f, -0.161400f, \
//                             -0.750200f, 1.713500f, 0.036700f,\
//                             0.038900f, -0.068500f, 1.029600f)


#define  XYZ_SRGB_REC709_D65  getXYZ(0.640f, 0.330f, 0.300f, 0.600f, 0.150f, 0.060f, 0.3127f, 0.3290f).inverse()
#define  XYZ_WIDE_GAMUT_RGB_D50  getXYZ(0.7347f, 0.2653f, 0.1152f, 0.8264f, 0.1566f, 0.0177f, 0.3457f,0.3585f).inverse()
#define  XYZ_P3_DCI  getXYZ(0.680f, 0.320f, 0.256f, 0.690f, 0.150f, 0.060f, 0.314f, 0.351f).inverse()
#define  XYZ_P3_D65  getXYZ(0.680f, 0.320f, 0.256f, 0.690f, 0.150f, 0.060f, 0.3127f, 0.3290f).inverse()
#define  XYZ_REC2020  getXYZ(0.708f, 0.292f, 0.170f, 0.797f, 0.131f, 0.046f, 0.3127f, 0.3290f).inverse()
#define  XYZ_ALEXA_WIDE_GAMUT  getXYZ(0.6840f, 0.3130f, 0.2210f, 0.8480f, 0.0861f, -0.1020f, 0.3127f,0.3290f).inverse()
#define  XYZ_CANON_CINEMA_GAMUT  getXYZ(0.740f, 0.270f, 0.170f, 1.140f, 0.080f, -0.100f, 0.3127f,0.3290f).inverse()
#define  XYZ_V_GAMUT  getXYZ(0.730f, 0.280f, 0.165f, 0.840f, 0.100f, -0.030f, 0.3127f, 0.3290f).inverse()
#define  XYZ_S_GAMUT  getXYZ(0.730f, 0.280f, 0.140f, 0.855f, 0.100f, -0.050f, 0.3127f, 0.3290f).inverse()
#define  XYZ_S_GAMUT_CINE  getXYZ(0.766f, 0.275f, 0.225f, 0.800f, 0.089f, -0.087f, 0.3127f, 0.3290f).inverse()

/*
class ColorTemperature {
private:
    constexpr static double TintScale = -3000.0;

    class ruvt {
    public:
        double r;
        double u;
        double v;
        double t;

        ruvt(double r, double u, double v, double t) {
            this->r = r;
            this->u = u;
            this->v = v;
            this->t = t;
        }
    };

    static ruvt *TempTable()
    {
        return new ruvt[31]{
                    ruvt(0, 0.18006, 0.26352, -0.24341),
                    ruvt(10, 0.18066, 0.26589, -0.25479),
                    ruvt(20, 0.18133, 0.26846, -0.26876),
                    ruvt(30, 0.18208, 0.27119, -0.28539),
                    ruvt(40, 0.18293, 0.27407, -0.30470),
                    ruvt(50, 0.18388, 0.27709, -0.32675),
                    ruvt(60, 0.18494, 0.28021, -0.35156),
                    ruvt(70, 0.18611, 0.28342, -0.37915),
                    ruvt(80, 0.18740, 0.28668, -0.40955),
                    ruvt(90, 0.18880, 0.28997, -0.44278),
                    ruvt(100, 0.19032, 0.29326, -0.47888),
                    ruvt(125, 0.19462, 0.30141, -0.58204),
                    ruvt(150, 0.19962, 0.30921, -0.70471),
                    ruvt(175, 0.20525, 0.31647, -0.84901),
                    ruvt(200, 0.21142, 0.32312, -1.0182),
                    ruvt(225, 0.21807, 0.32909, -1.2168),
                    ruvt(250, 0.22511, 0.33439, -1.4512),
                    ruvt(275, 0.23247, 0.33904, -1.7298),
                    ruvt(300, 0.24010, 0.34308, -2.0637),
                    ruvt(325, 0.24702, 0.34655, -2.4681),
                    ruvt(350, 0.25591, 0.34951, -2.9641),
                    ruvt(375, 0.26400, 0.35200, -3.5814),
                    ruvt(400, 0.27218, 0.35407, -4.3633),
                    ruvt(425, 0.28039, 0.35577, -5.3762),
                    ruvt(450, 0.28863, 0.35714, -6.7262),
                    ruvt(475, 0.29685, 0.35823, -8.5955),
                    ruvt(500, 0.30505, 0.35907, -11.324),
                    ruvt(525, 0.31320, 0.35968, -15.628),
                    ruvt(550, 0.32129, 0.36011, -23.325),
                    ruvt(575, 0.32931, 0.36038, -40.770),
                    ruvt(600, 0.33724, 0.36051, -116.45)
        };
    }
public:
    static Vec3 colorTemperatureXy(double mTemperature, double mTint) {
        Vec3 result = Vec3(new double[3]);
        // Find inverse temperature to use as index.
        double r = 1.0E6 / mTemperature;
        // Convert tint to offset is uv space.
        double offset = mTint * (1.0 / TintScale);
        // Search for line pair containing coordinate.
        for (int index = 0; index <= 29; index++) {
            if (r < TempTable()[index + 1].r || index == 29) {
                // Find relative weight of first line.
                double f = (TempTable()[index + 1].r - r) / (TempTable()[index + 1].r - TempTable()[index].r);
                // Interpolate the black body coordinates.
                double u = TempTable()[index].u * f + TempTable()[index + 1].u * (1.0 - f);
                double v = TempTable()[index].v * f + TempTable()[index + 1].v * (1.0 - f);
                // Find vectors along slope for each line.
                double uu1 = 1.0;
                double vv1 = TempTable()[index].t;
                double uu2 = 1.0;
                double vv2 = TempTable()[index + 1].t;
                double len1 = sqrt(1.0 + vv1 * vv1);
                double len2 = sqrt(1.0 + vv2 * vv2);
                uu1 /= len1;
                vv1 /= len1;
                uu2 /= len2;
                vv2 /= len2;
                // Find vector from black body point.
                double uu3 = uu1 * f + uu2 * (1.0 - f);
                double vv3 = vv1 * f + vv2 * (1.0 - f);
                double len3 = sqrt(uu3 * uu3 + vv3 * vv3);
                uu3 /= len3;
                vv3 /= len3;
                // Adjust coordinate along this vector.
                u += uu3 * offset;
                v += vv3 * offset;
                // Convert to xy coordinates.
                result(0) = 1.5 * u / (u - 4.0 * v + 2.0);
                result(1) = v / (u - 4.0 * v + 2.0);
                result(2) = 1 - (result(0) + result(1));
                break;
            }
        }
        return result;
    }

    static Vec3 colorTemperatureXYZ(double temperature, double tint) {
        Vec3 temp = colorTemperatureXy(temperature, tint);
        return xy2XYZ(temp);
    }

    static Vec3 CCT(Vec3 xy) {
        double mTemperature = 0;
        double mTint = 0;
        // Convert to uv space.
        double u = 2.0 * xy(0) / (1.5 - xy(0) + 6.0 * xy(1));
        double v = 3.0 * xy(1) / (1.5 - xy(0) + 6.0 * xy(1));

        // Search for line pair coordinate is between.
        double lastDt = 0.0;

        double lastDv = 0.0;
        double lastDu = 0.0;

        for (int index = 1; index <= 30; index++) {

            // Convert slope to delta-u and delta-v, with length 1.
            double du = 1.0;
            double dv = TempTable()[index].t;

            double len = sqrt(1.0 + dv * dv);

            du /= len;
            dv /= len;

            // Find delta from black body point to test coordinate.
            double uu = u - TempTable()[index].u;
            double vv = v - TempTable()[index].v;

            // Find distance above or below line.
            double dt = -uu * dv + vv * du;

            // If below line, we have found line pair.
            if (dt <= 0.0 || index == 30) {

                // Find fractional weight of two lines.
                if (dt > 0.0) {
                    dt = 0.0;
                }

                dt = -dt;

                double f;

                if (index == 1) {
                    f = 0.0;
                } else {
                    f = dt / (lastDt + dt);
                }

                // Interpolate the temperature.
                mTemperature = 1.0E6 / (TempTable()[index - 1].r * f +
                                        TempTable()[index].r * (1.0 - f));

                // Find delta from black body point to test coordinate.
                uu = u - (TempTable()[index - 1].u * f +
                          TempTable()[index].u * (1.0 - f));

                vv = v - (TempTable()[index - 1].v * f +
                          TempTable()[index].v * (1.0 - f));

                // Interpolate vectors along slope.
                du = du * (1.0 - f) + lastDu * f;
                dv = dv * (1.0 - f) + lastDv * f;

                len = sqrt(du * du + dv * dv);

                du /= len;
                dv /= len;

                // Find distance along slope.
                mTint = (uu * du + vv * dv) * TintScale;

                break;
            }

            // Try next line pair.
            lastDt = dt;

            lastDu = du;
            lastDv = dv;
        }
        return Vec3(new double []{mTemperature, mTint, 0});
    }

    /*Vec3 colorTemperatureRGB(double temperature, double tint) {
        Vec3 temp = colorTemperatureXYZ(temperature, tint);
        return ColorSpaceXYZ_SRGB_REC709_D65.multiplyVec(temp);
    }*

    Vec3 whiteBalance(float temperature, float tint, float temperaturePoint, float tintPoint) {
        Vec3 temp1 = colorTemperatureXYZ(temperature, tint);
        Vec3 whitePoint = colorTemperatureXYZ(temperaturePoint, tintPoint);
        Vec3 whitePoint1 = Vec3(new double[]
                                        {
                                                1 - whitePoint(0),
                                                1 - whitePoint(1),
                                                1 - whitePoint(2),
                                        });
        Vec3 whiteBalanceCentered = Vec3(new double[]
                                                 {
                                                         (temp1(0) + whitePoint1(0)),
                                                         (temp1(1) + whitePoint1(1)),
                                                         (temp1(2) + whitePoint1(2)),
                                                 });

        return Vec3(new double[]
                            {
                                    1 / whiteBalanceCentered(0),
                                    1 / whiteBalanceCentered(1),
                                    1 / whiteBalanceCentered(2),
                            });
    }


    /*public static float cct(float R, float G, float B) {
            float X = (-0.14282f * R) + (1.54924f * G) + (-0.95641f * B);
            float Y = (-0.32466f * R) + (1.57837f * G) + (-0.73191f * B);
            float Z = (-0.68202f * R) + (0.77073f * G) + (0.56332f * B);
            float x = X / (X + Y + Z);
            float y = Y / (X + Y + Z);

            float n = (x - 0.3320f) / (0.1858f - y);
            float CCT = 437f * (float) Math.pow(n, 3f) + 3601f * (float) Math.pow(n, 2f) + 6861f * n + 5517f;
            //float CCT = 449f * (float)Math.pow(n, 3f) + 3525f * (float)Math.pow(n, 2f) + 6823.3f * n + 5520.33f;
            //float CCT = 449f * n * 3 + 3525f * n * 2 + 6823.3f * n + 5520.33f;
            return CCT;
        }

    public static float cct(float x, float y) {
            float n = (x - 0.3320f) / (0.1858f - y);
            float CCT = 437f * (float) Math.pow(n, 3f) + 3601f * (float) Math.pow(n, 2f) + 6861f * n + 5517f;
            //float CCT = 449f * (float)Math.pow(n, 3f) + 3525f * (float)Math.pow(n, 2f) + 6823.3f * n + 5520.33f;
            //float CCT = 449f * n * 3 + 3525f * n * 2 + 6823.3f * n + 5520.33f;
            return CCT;
        }

    public static float cct(Matriz xy) {
            float x = (float) xy.m[0];
            float y = (float) xy.m[1];
            float n = (x - 0.3320f) / (0.1858f - y);
            float CCT = 437f * (float) Math.pow(n, 3f) + 3601f * (float) Math.pow(n, 2f) + 6861f * n + 5517f;
            //float CCT = 449f * (float)Math.pow(n, 3f) + 3525f * (float)Math.pow(n, 2f) + 6823.3f * n + 5520.33f;
            //float CCT = 449f * n * 3 + 3525f * n * 2 + 6823.3f * n + 5520.33f;
            return CCT;
        }

    public static double CCT1(Matriz xy) {
            double x = xy.m[0];
            double y = xy.m[1];
            double n = (x - 0.332) / (y - 0.1858);
            return -449 * Math.pow(n, 3) + 3525 * Math.pow(n, 2) - 6823.3 * n + 5520.33;
        }*
};
*/
#endif
//#define COLOR_CORE