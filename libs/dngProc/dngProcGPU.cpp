#include "dngProc.h"
#include <Halide.h>
#include "ColorCorrect.h"

using namespace std;
using namespace Halide;

/*uint16_t *raw2wb(DngImg &img, int hPath, float denoisePower, float gammaV, float satV) {
    CC color(img);
    DngTemp temp = color.cameraNeutralToXy();
    Matriz camToPcs = color.cameraToPcs(temp);
    Matriz pcsToSrgb = CC::pcsToSrgb();
    Matriz sti = pcsToSrgb * camToPcs;

    Buffer<uint16_t> dngImg(img.data, img.width, img.height);
    Var x, y, xo, yo, xi, yi, xa, ya, xb, yb, c;
    //-----------------------------par-----------------------------
    Func par("par");
    {
        Expr x0 = (x / 2) * 2;
        Expr y0 = (y / 2) * 2;
        Expr ret = select(x == x0 && y == y0, 0,
                          x != x0 && y == y0, 1,
                          x == x0 && y != y0, 2, 3);
        par(x, y) = ret;
    }

    //-----------------------------linearize-----------------------------
    Func linearize("linearize");
    {

        Expr whitePoint = cast<float>((int) img.whiteLevel);
        Expr blackPoint = select(par(x, y) == 0, cast<float>((int) img.blackLevel[0]),
                                 par(x, y) == 1, cast<float>((int) img.blackLevel[1]),
                                 par(x, y) == 2, cast<float>((int) img.blackLevel[2]),
                                 cast<float>((int) img.blackLevel[3]));
        Expr outColor = (cast<float>(dngImg(x, y)) - blackPoint) / (whitePoint - blackPoint);// * gm(x, y, par(x, y));
        linearize(x, y) = clamp(outColor, 0.f, 1.f);
    }
    //-----------------------------getMosaic-----------------------------
    Func getMosaic("getMosaic");
    {
        getMosaic(x, y, xa, ya) = linearize(clamp(x + xa, 0, img.width - 1),
                                            clamp(y + ya, 0, img.height - 1));
    }

    //-----------------------------demosaicBRGGB-----------------------------
    Func demoBBGGR("demoBBGGR");
    {

        Expr p = par(x, y);
        Expr r = select(par(x, y) == 0,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        getMosaic(x, y, 0, 0));

        Expr g = select(par(x, y) == 0, (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                                         getMosaic(x, y, 0, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                         getMosaic(x, y, 0, 1)) / 4.f);

        Expr b = select(par(x, y) == 0, getMosaic(x, y, 0, 0),
                        par(x, y) == 1, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f);
        demoBBGGR(x, y, c) = clamp(select(c == 0, r, c == 1, g, b), 0.f, 1.f);
    }
    //-----------------------------ColorSpace-----------------------------
    Func applyCS("applyCS");
    {

        Expr cr = demoBBGGR(x, y, 0);
        Expr cg = demoBBGGR(x, y, 1);
        Expr cb = demoBBGGR(x, y, 2);

        cr = clamp(cr, 0.f, (float) img.camNeutral[0]);
        cg = clamp(cg, 0.f, (float) img.camNeutral[1]);
        cb = clamp(cb, 0.f, (float) img.camNeutral[2]);

        Expr cr1 = cr * (float) sti(0) + cg * (float) sti(1) + cb * (float) sti(2);
        Expr cg1 = cr * (float) sti(3) + cg * (float) sti(4) + cb * (float) sti(5);
        Expr cb1 = cr * (float) sti(6) + cg * (float) sti(7) + cb * (float) sti(8);

        cr1 = clamp(cr1, 0.f, 1.f);
        cg1 = clamp(cg1, 0.f, 1.f);
        cb1 = clamp(cb1, 0.f, 1.f);
        applyCS(c, x, y) = select(c == 0, cr1, c == 1, cg1, cb1);
    }


    Func wb("wb");
    {
        //        gamma(c, x, y) = pow(applyCS(c, x, y), 1.f / gammaV);
//        wb(x, y) = (gamma(0, x, y) + gamma(1, x, y) + gamma(2, x, y)) / 3.f;
        wb(x, y) = (applyCS(0, x, y) + applyCS(1, x, y) + applyCS(2, x, y)) / 3.f;
    }
    Func nr("nr");
    {
        int pathWidth = hPath * 2;
        Func wht("wht");
        Var pathX, pathY;
        Expr tileW = abs(wb(x, y) - wb(x + pathX, y + pathY));
        tileW = pow(-tileW + 1.f,  denoisePower);
        tileW = min(tileW, 1.f);
        wht(pathX, pathY, x, y) = select(pathX == 0 && pathY == 0, 1.f, tileW);

        RDom pm(0, pathWidth, 0, pathWidth);
        Expr pmx = pm.x - hPath;
        Expr pmy = pm.y - hPath;
        Expr w = wht(pmx, pmy, x, y);
        Expr pix = wb(x + pmx, y + pmy);
        Expr pixMerge = sum(pix * w);
        Expr totalWeight = sum(w);
        nr(x, y) = pixMerge / totalWeight;
//        nr(x, y) = cast<uint16_t>(pixMerge / totalWeight * 65535.f);
    }

    Func gamma("gamma");
    {
        gamma(x, y) = pow(nr(x, y), 1.f / gammaV);
//        gamma(c, x, y) = pow(applyCS(c, x, y), 1.f / gammaV);
    }


    Func outImg("outImg");
    {
        outImg(x, y) = cast<uint16_t>(gamma(x, y) * 65535.f);
    }

    outImg.compute_root();
    outImg.tile(x, y, xo, yo, xi, yi, 8, 8);
    outImg.parallel(yo).vectorize(xi);
    demoBBGGR.compute_root();
    demoBBGGR.tile(x, y, xo, yo, xi, yi, 8, 8);
    demoBBGGR.parallel(yo).vectorize(xi);
    applyCS.compute_root();
    applyCS.tile(x, y, xo, yo, xi, yi, 8, 8);
    applyCS.parallel(yo).vectorize(xi);


    Buffer<uint16_t> out = outImg.realize({img.width, img.height});

    auto *rawOut = (uint16_t *) malloc(img.width * img.height * sizeof(uint16_t));
    memcpy(rawOut, out.data(), img.width * img.height * sizeof(uint16_t));
    return rawOut;
}*/

uint8_t *raw2rgb(DngImg &img, float gammaV, float satV) {
    CC color(img);
    DngTemp temp = color.cameraNeutralToXy();
    Matriz camToPcs = color.cameraToPcs(temp);
    Matriz pcsToSrgb = CC::pcsToSrgb();
    Matriz sti = pcsToSrgb * camToPcs;

    Buffer<uint16_t> dngImg(img.data, img.width, img.height);
    Var x, y, xa, ya, xb, yb, c, yo, yi, xo, xi;
    //-----------------------------par-----------------------------
    Func par("par");
    {
        Expr x0 = (x / 2) * 2;
        Expr y0 = (y / 2) * 2;
        Expr ret = select(x == x0 && y == y0, 0,
                          x != x0 && y == y0, 1,
                          x == x0 && y != y0, 2, 3);
        par(x, y) = ret;
    }

    Func gm("gainMapFun");
    {
        if(img.gm.empty())
        {
            gm(x, y, c) = 1.f;
        }
        else {
            int w = img.gm.width;
            int h = img.gm.height;
            Buffer<float> gmBuffer(img.gm.gm, img.gm.width, img.gm.height, 4);

            Expr px = cast<float>(x) / img.width;
            Expr py = cast<float>(y) / img.height;

            Expr xA = cast<int>(px * (float) w);
            Expr yA = cast<int>(py * (float) h);
            Expr xB = clamp(xA + 1, 0, w - 1);
            Expr yB = clamp(yA + 1, 0, h - 1);
            Expr sx = (px * (float) w) - cast<float>(xA);
            Expr sy = (py * (float) h) - cast<float>(yA);
            Expr pointA = gmBuffer(xA, yA, c);
            Expr pointB = gmBuffer(xB, yA, c);
            Expr pointC = gmBuffer(xA, yB, c);
            Expr pointD = gmBuffer(xB, yB, c);

            Expr wa = (1.f - sx) * (1.f - sy);
            Expr wb = (sx) * (1.f - sy);
            Expr wc = (1.f - sx) * (sy);
            Expr wd = (sx) * (sy);

            gm(x, y, c) = pointA * wa + pointB * wb + pointC * wc + pointD * wd;
            gm.compute_root()
                    .split(y, yo, yi, 8)
                    .parallel(yo)
                    .vectorize(yi);
        }

    }

    //-----------------------------linearize-----------------------------
    Func linearize("linearize");
    {

        Expr whitePoint = cast<float>((int) img.whiteLevel);
        Expr blackPoint = select(par(x, y) == 0, cast<float>((int) img.blackLevel[0]),
                                 par(x, y) == 1, cast<float>((int) img.blackLevel[1]),
                                 par(x, y) == 2, cast<float>((int) img.blackLevel[2]),
                                 cast<float>((int) img.blackLevel[3]));
        Expr gain = select(img.gm.empty(), 1.f, gm(x, y, par(x, y)));
        Expr outColor = (cast<float>(dngImg(x, y)) - blackPoint) / (whitePoint - blackPoint) * gain;
        linearize(x, y) = clamp(outColor, 0.f, 1.f);
    }
    //-----------------------------getMosaic-----------------------------
    Func getMosaic("getMosaic");
    {
        getMosaic(x, y, xa, ya) = linearize(clamp(x + xa, 0, img.width - 1),
                                            clamp(y + ya, 0, img.height - 1));
    }

    //-----------------------------demosaicBBGGR-----------------------------
    Func demoBBGGR("demoBBGGR");
    {

        Expr p = par(x, y);
        Expr r = select(par(x, y) == 0,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        getMosaic(x, y, 0, 0));

        Expr g = select(par(x, y) == 0, (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                                         getMosaic(x, y, 0, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                         getMosaic(x, y, 0, 1)) / 4.f);

        Expr b = select(par(x, y) == 0, getMosaic(x, y, 0, 0),
                        par(x, y) == 1, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f);
        demoBBGGR(x, y, c) = clamp(select(c == 0, r, c == 1, g, b), 0.f, 1.f);
        demoBBGGR.compute_root()
                .split(y, yo, yi, 8)
                .parallel(yo)
                .vectorize(yi);

//p == 0 par par
//    R G R
//    G B G
//    R G R
//p == 1 impar par
//    G R G
//    B G B
//    G R G
//p == 2 par impar
//    G B G
//    R G R
//    G B G
//p == 3 impar impar
//    B G B
//    G R G
//    B G B

    }
    //-----------------------------demosaicBRGGB-----------------------------
    Func demoBRGGB("demoBRGGB");
    {

        Expr p = par(x, y);
        Expr b = select(par(x, y) == 0,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        getMosaic(x, y, 0, 0));

        Expr g = select(par(x, y) == 0, (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                                         getMosaic(x, y, 0, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                         getMosaic(x, y, 0, 1)) / 4.f);

        Expr r = select(par(x, y) == 0, getMosaic(x, y, 0, 0),
                        par(x, y) == 1, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f);
        demoBRGGB(x, y, c) = clamp(select(c == 0, r, c == 1, g, b), 0.f, 1.f);
        demoBRGGB.compute_root()
                .split(y, yo, yi, 8)
                .parallel(yo)
                .vectorize(yi);
    }

    //-----------------------------demosaicGRBG-----------------------------
    Func demoBiGRBG("demoBiGRBG");
    {
        Expr corners =
                (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) /
                4.f;
        Expr upDo = (getMosaic(x, y, 0, 1) + getMosaic(x, y, 0, -1)) / 2.f;
        Expr leRi = (getMosaic(x, y, 1, 0) + getMosaic(x, y, -1, 0)) / 2.f;
        Expr upDoLeRi =
                (getMosaic(x, y, 0, 1) + getMosaic(x, y, 0, -1) + getMosaic(x, y, 1, 0) + getMosaic(x, y, -1, 0)) / 4.f;
        Expr center = getMosaic(x, y, 0, 0);
        Expr centerX =
                (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1) +
                 getMosaic(x, y, 0, 0)) / 5.f;
        //p == 0 par par
//    G B G
//    R G R
//    G B G
//p == 1 impar par
//    B G B
//    G R G
//    B G B
//p == 2 par impar
//    R G R
//    G B G
//    R G R
//p == 3 impar impar
//    G R G
//    B G B
//    G R G

        Expr p = par(x, y);
        Expr r = select(par(x, y) == 0, leRi,
                        par(x, y) == 1, center,
                        par(x, y) == 2, corners,
                        upDo);

        Expr g = select(par(x, y) == 0, centerX,
                        par(x, y) == 1, upDoLeRi,
                        par(x, y) == 2, upDoLeRi,
                        centerX);

        Expr b = select(par(x, y) == 0, upDo,
                        par(x, y) == 1, corners,
                        par(x, y) == 2, center,
                        leRi);

        demoBiGRBG(x, y, c) = clamp(select(c == 0, r, c == 1, g, b), 0.f, 1.f);
        demoBiGRBG.compute_root()
                .split(y, yo, yi, 8)
                .parallel(yo)
                .vectorize(yi);
    }

    //-----------------------------demosaicGBRG-----------------------------
    Func demoBiGBRG("demoBiGBRG");
    {
        Expr corners =
                (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) /
                4.f;
        Expr upDo = (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f;
        Expr leRi = (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f;
        Expr upDoLeRi =
                (getMosaic(x, y, 0, 1) + getMosaic(x, y, 0, -1) + getMosaic(x, y, 1, 0) + getMosaic(x, y, -1, 0)) / 4.f;
        Expr center = getMosaic(x, y, 0, 0);
        Expr centerX =
                (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1) +
                 getMosaic(x, y, 0, 0)) / 5.f;
        //p == 0 par par
//    G R G
//    B G B
//    G R G
//p == 1 impar par
//    R G R
//    G B G
//    R G R
//p == 2 par impar
//    B G B
//    G R G
//    B G B
//p == 3 impar impar
//    G B G
//    R G R
//    G B G

        Expr p = par(x, y);

        Expr r = select(par(x, y) == 0, upDo,
                        par(x, y) == 1, corners,
                        par(x, y) == 2, center,
                        leRi);

        Expr g = select(par(x, y) == 0, centerX,
                        par(x, y) == 1, upDoLeRi,
                        par(x, y) == 2, upDoLeRi,
                        centerX);

        Expr b = select(par(x, y) == 0, leRi,
                        par(x, y) == 1, center,
                        par(x, y) == 2, corners,
                        upDo);

        demoBiGBRG(x, y, c) = clamp(select(c == 0, r, c == 1, g, b), 0.f, 1.f);
        demoBiGBRG.compute_root()
                .split(y, yo, yi, 8)
                .parallel(yo)
                .vectorize(yi);
    }

    //-----------------------------ColorSpace-----------------------------

    Func applyCS("applyCS");
    {
        Expr cr = select(img.cfa == DngImg::CFA_RGGB, demoBRGGB(x, y, 0),
                         img.cfa == DngImg::CFA_BGGR, demoBBGGR(x, y, 0),
                         img.cfa == DngImg::CFA_GRBG, demoBiGRBG(x, y, 0),
                         demoBiGBRG(x, y, 0));
        Expr cg = select(img.cfa == DngImg::CFA_RGGB, demoBRGGB(x, y, 1),
                         img.cfa == DngImg::CFA_BGGR, demoBBGGR(x, y, 1),
                         img.cfa == DngImg::CFA_GRBG, demoBiGRBG(x, y, 1),
                         demoBiGBRG(x, y, 1));

        Expr cb = select(img.cfa == DngImg::CFA_RGGB, demoBRGGB(x, y, 2),
                         img.cfa == DngImg::CFA_BGGR, demoBBGGR(x, y, 2),
                         img.cfa == DngImg::CFA_GRBG, demoBiGRBG(x, y, 2),
                         demoBiGBRG(x, y, 2));

//        Expr cr = demoBRGGB(x, y, 0);
//        Expr cg = demoBRGGB(x, y, 1);
//        Expr cb = demoBRGGB(x, y, 2);

        cr = clamp(cr, 0.f, (float) img.camNeutral[0]);
        cg = clamp(cg, 0.f, (float) img.camNeutral[1]);
        cb = clamp(cb, 0.f, (float) img.camNeutral[2]);

        Expr cr1 = cr * sti(0) + cg * sti(1) + cb * sti(2);
        Expr cg1 = cr * sti(3) + cg * sti(4) + cb * sti(5);
        Expr cb1 = cr * sti(6) + cg * sti(7) + cb * sti(8);

        cr1 = clamp(cr1, 0.f, 1.f);
        cg1 = clamp(cg1, 0.f, 1.f);
        cb1 = clamp(cb1, 0.f, 1.f);

        applyCS(c, x, y) = select(c == 0, cr1, c == 1, cg1, cb1);
    }

    Func gamma("gamma");
    {
        gamma(c, x, y) = pow(applyCS(c, x, y), 1.f / gammaV);
    }


    Func rawP("rawP");

    rawP(c, x, y) = cast<uint8_t>(gamma(c, x, y) * 255.f);

//    Target gpu = get_host_target().with_feature(Target::CUDA);
//    gpu.set_feature(Target::CUDA, true);
    rawP.compute_root()
            .split(y, yo, yi, 8)
            .parallel(yo)
            .vectorize(yi);
//    rawP.gpu_tile(x, y, xa, ya, xb, yb, 4, 4);
//    rawP.gpu_blocks(xa, ya);
//    rawP.gpu_threads(xb, yb);
//    rawP.compile_jit(gpu);

    Buffer<uint8_t> out = rawP.realize({3, img.width, img.height});

    auto *rawOut = (uint8_t *) malloc(img.width * img.height * 3);
    memcpy(rawOut, out.data(), img.width * img.height * 3);

    return rawOut;
}

/*uint8_t* dngProcess(DngImg &img, float gamma, float sat)
{
    CC color(img);
    DngTemp temp = color.cameraNeutralToXy();
    Matriz camToPcs = color.cameraToPcs(temp);
    Matriz pcsToSrgb = CC::pcsToSrgb();
    Matriz sti = pcsToSrgb * camToPcs;

    Buffer<uint16_t> dngImg(img.data, img.width, img.height);
    Var x, y, xa, ya, xb, yb, c;
    //-----------------------------par-----------------------------
    Func par;
    {
        Expr x0 = (x / 2) * 2;
        Expr y0 = (y / 2) * 2;
        Expr ret = select(x == x0 && y == y0, 0,
                          x != x0 && y == y0, 1,
                          x == x0 && y != y0, 2, 3);
        par(x, y) = ret;
    }

    //-----------------------------linearize-----------------------------
    Func linearize;
    {

        Expr whitePoint = cast<float>((int) img.whiteLevel);
        Expr blackPoint = select(par(x, y) == 0, cast<float>((int) img.blackLevel[0]),
                                 par(x, y) == 1, cast<float>((int) img.blackLevel[1]),
                                 par(x, y) == 2, cast<float>((int) img.blackLevel[2]),
                                 cast<float>((int) img.blackLevel[3]));
        Expr outColor = (cast<float>(dngImg(x, y)) - blackPoint) / (whitePoint - blackPoint);// * gm(x, y, par(x, y));
        linearize(x, y) = clamp(outColor, 0.f, 1.f);
        linearize.compute_root();
        linearize.parallel(y);
    }
    //-----------------------------getMosaic-----------------------------
    Func getMosaic;
    {
        getMosaic(x, y, xa, ya) = linearize(clamp(x + xa, 0, img.width - 1),
                                            clamp(y + ya, 0, img.height - 1));
    }

    //-----------------------------demosaicBRGGB-----------------------------
    Func demoBBGGR;
    {

        Expr p = par(x, y);
        Expr r = select(par(x, y) == 0,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        getMosaic(x, y, 0, 0));

        Expr g = select(par(x, y) == 0, (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                                         getMosaic(x, y, 0, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                         getMosaic(x, y, 0, 1)) / 4.f);

        Expr b = select(par(x, y) == 0, getMosaic(x, y, 0, 0),
                        par(x, y) == 1, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f);
        demoBBGGR(x, y, c) = clamp(select(c == 0, r, c == 1, g, b), 0.f, 1.f);
        demoBBGGR.compute_root();
        demoBBGGR.parallel(y);

//p == 0 par par
//    R G R
//    G B G
//    R G R
//p == 1 impar par
//    G R G
//    B G B
//    G R G
//p == 2 par impar
//    G B G
//    R G R
//    G B G
//p == 3 impar impar
//    B G B
//    G R G
//    B G B

    }
    //-----------------------------demosaicBRGGB-----------------------------
    Func demoBRGGB;
    {

        Expr p = par(x, y);
        Expr b = select(par(x, y) == 0,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        getMosaic(x, y, 0, 0));

        Expr g = select(par(x, y) == 0, (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                                         getMosaic(x, y, 0, 1)) / 4.f,
                        par(x, y) == 1, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        par(x, y) == 2, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) +
                                         getMosaic(x, y, -1, 1) + getMosaic(x, y, 1, 1)) / 5.f,
                        (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) +
                         getMosaic(x, y, 0, 1)) / 4.f);

        Expr r = select(par(x, y) == 0, getMosaic(x, y, 0, 0),
                        par(x, y) == 1, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                        par(x, y) == 2, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                        (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                         getMosaic(x, y, 1, 1)) / 4.f);
        demoBRGGB(x, y, c) = clamp(select(c == 0, r, c == 1, g, b), 0.f, 1.f);
        demoBRGGB.compute_root();
        demoBRGGB.parallel(y);
    }
    //-----------------------------ColorSpace-----------------------------

    Func applyCS;
    {
        Expr cr = demoBRGGB(x, y, 0);
        Expr cg = demoBRGGB(x, y, 1);
        Expr cb = demoBRGGB(x, y, 2);

        cr = clamp(cr, 0.f, (float) img.camNeutral[0]);
        cg = clamp(cg, 0.f, (float) img.camNeutral[1]);
        cb = clamp(cb, 0.f, (float) img.camNeutral[2]);

        Expr cr1 = cr * (float) sti(0) + cg * (float) sti(1) + cb * (float) sti(2);
        Expr cg1 = cr * (float) sti(3) + cg * (float) sti(4) + cb * (float) sti(5);
        Expr cb1 = cr * (float) sti(6) + cg * (float) sti(7) + cb * (float) sti(8);

        cr1 = clamp(cr1, 0.f, 1.f);
        cg1 = clamp(cg1, 0.f, 1.f);
        cb1 = clamp(cb1, 0.f, 1.f);

        applyCS(c, x, y) = select(c == 0, cr1, c == 1, cg1, cb1);
    }

    Func gamma;
    {
        gamma(c, x, y) = pow(applyCS(c, x, y), 1.f / gammaV);
    }


    Func rawP;

    rawP(c, x, y) = cast<uint8_t>(gamma(c, x, y) * 255.f);

//    Target gpu = get_host_target().with_feature(Target::CUDA);
//    gpu.set_feature(Target::CUDA, true);
    rawP.compute_root();
    rawP.parallel(y);
//    rawP.gpu_tile(x, y, xa, ya, xb, yb, 4, 4);
//    rawP.gpu_blocks(xa, ya);
//    rawP.gpu_threads(xb, yb);
//    rawP.compile_jit(gpu);

    Buffer<uint8_t> out = rawP.realize({3, img.width, img.height});

    auto *rawOut = (uint8_t *) malloc(img.width * img.height * 3);
    memcpy(rawOut, out.data(), img.width * img.height * 3);

    cout << "processed" << endl;
    return rawOut;
}*/

/*
Func rgb2yuv(Func input) {

    Var x, y, c;
    Func out;

    Expr r = input(0, x, y);
    Expr g = input(1, x, y);
    Expr b = input(2, x, y);

    out(c, x, y) = 0.f;

    out(0, x, y) = 0.298900f * r + 0.587000f * g + 0.114000f * b;           // Y
    out(1, x, y) = -0.168935f * r - 0.331655f * g + 0.500590f * b;           // U
    out(2, x, y) = 0.499813f * r - 0.418531f * g - 0.081282f * b;           // V

    out.compute_root().parallel(y).vectorize(x, 16);
    out.update(0).parallel(y).vectorize(x, 16);
    out.update(1).parallel(y).vectorize(x, 16);
    out.update(2).parallel(y).vectorize(x, 16);
    return out;
}

Func yuv2rgb(Func input) {

    Func out;

    Var x, y, c;

    Expr Y = input(0, x, y);
    Expr U = input(1, x, y);
    Expr V = input(2, x, y);

    out(c, x, y) = 0.f;

    out(0, x, y) = (Y + 1.403f * V);          // r
    out(1, x, y) = (Y - 0.344f * U - 0.714f * V);          // g
    out(2, x, y) = (Y + 1.770f * U);          // b

    out.compute_root().parallel(y).vectorize(x, 16);

    out.update(0).parallel(y).vectorize(x, 16);
    out.update(1).parallel(y).vectorize(x, 16);
    out.update(2).parallel(y).vectorize(x, 16);

    return out;
}

uint8_t *raw2rgbNr(DngImg &img, double gamma, double sat) {
    CC color(img);
    DngTemp temp = color.cameraNeutralToXy();
    Matriz camToPcs = color.cameraToPcs(temp);
    Matriz pcsToSrgb = CC::pcsToSrgb();
    Matriz sti = pcsToSrgb * camToPcs;

    Buffer<uint16_t> imgB(img.data, img.width, img.height);
    Func imgF(imgB);
    Var x, y, c;
    Var x0, y0, x1, y1;
    //-----------------------------par-----------------------------
    Func par("par");
    Expr x1_ = (x / 2) * 2;
    Expr y1_ = (y / 2) * 2;
    Expr ret = select(x == x1_ && y == y1_, 0,
                      x != x1_ && y == y1_, 1,
                      x == x1_ && y != y1_, 2, 3);
    par(x, y) = ret;

    //-----------------------------linearize-----------------------------
    Func linearize("linearize");
    Expr whitePoint = cast<float>((int) img.whiteLevel);
    Expr blackPoint = select(par(x, y) == 0, cast<float>((int) img.blackLevel[0]),
                             par(x, y) == 1, cast<float>((int) img.blackLevel[1]),
                             par(x, y) == 2, cast<float>((int) img.blackLevel[2]),
                             cast<float>((int) img.blackLevel[3]));
    Expr outColor = (cast<float>(imgF(x, y)) - blackPoint) / (whitePoint - blackPoint);// * gm(x, y, par(x, y));
    linearize(x, y) = clamp(outColor, 0.f, 1.f);
    linearize.compute_root()
            .parallel(y);
    //-----------------------------getMosaic-----------------------------
    Func getMosaic("getMosaic");
    Expr x0_ = x + x0;
    Expr y0_ = y + y0;
    x0_ = clamp(x0_, 0, img.width - 1);
    y0_ = clamp(y0_, 0, img.height - 1);
    getMosaic(x, y, x0, y0) = linearize(x0_, y0_);

    //-----------------------------demosaicBRGGB-----------------------------
    Func demoBBGGR("demoBBGGR");

    Expr p = par(x, y);

    Expr r = select(par(x, y) == 0, (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                                     getMosaic(x, y, 1, 1)) / 4.f,
                    par(x, y) == 1, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                    par(x, y) == 2, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                    getMosaic(x, y, 0, 0));

    Expr g = select(par(x, y) == 0,
                    (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) + getMosaic(x, y, 0, 1)) /
                    4.f,
                    par(x, y) == 1,
                    (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) + getMosaic(x, y, -1, 1) +
                     getMosaic(x, y, 1, 1)) / 5.f,
                    par(x, y) == 2,
                    (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, 0, 0) + getMosaic(x, y, -1, 1) +
                     getMosaic(x, y, 1, 1)) / 5.f,
                    (getMosaic(x, y, 0, -1) + getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0) + getMosaic(x, y, 0, 1)) /
                    4.f);

    Expr b = select(par(x, y) == 0, getMosaic(x, y, 0, 0),
                    par(x, y) == 1, (getMosaic(x, y, -1, 0) + getMosaic(x, y, 1, 0)) / 2.f,
                    par(x, y) == 2, (getMosaic(x, y, 0, -1) + getMosaic(x, y, 0, 1)) / 2.f,
                    (getMosaic(x, y, -1, -1) + getMosaic(x, y, 1, -1) + getMosaic(x, y, -1, 1) +
                     getMosaic(x, y, 1, 1)) / 4.f);
    demoBBGGR(x, y, c) = clamp(select(c == 0, r, c == 1, g, b), 0.f, 1.f);
    demoBBGGR.compute_root()
            .parallel(y);

//p == 0 par par
//    R G R
//    G B G
//    R G R
//p == 1 impar par
//    G R G
//    B G B
//    G R G
//p == 2 par impar
//    G B G
//    R G R
//    G B G
//p == 3 impar impar
//    B G B
//    G R G
//    B G B

//-----------------------------ColorSpace-----------------------------

    Expr cr = demoBBGGR(x, y, 0);
    Expr cg = demoBBGGR(x, y, 1);
    Expr cb = demoBBGGR(x, y, 2);

    cr = clamp(cr, 0.f, (float) img.camNeutral[0]);
    cg = clamp(cg, 0.f, (float) img.camNeutral[1]);
    cb = clamp(cb, 0.f, (float) img.camNeutral[2]);

    Expr cr1 = cr * (float) sti(0) + cg * (float) sti(1) + cb * (float) sti(2);
    Expr cg1 = cr * (float) sti(3) + cg * (float) sti(4) + cb * (float) sti(5);
    Expr cb1 = cr * (float) sti(6) + cg * (float) sti(7) + cb * (float) sti(8);

    cr1 = clamp(cr1, 0.f, 1.f);
    cg1 = clamp(cg1, 0.f, 1.f);
    cb1 = clamp(cb1, 0.f, 1.f);

    cr1 = pow(cr1, 1.f / (float) gamma);
    cg1 = pow(cg1, 1.f / (float) gamma);
    cb1 = pow(cb1, 1.f / (float) gamma);

    Func process;
    process(c, x, y) = select(c == 0, cr1,
                              c == 1, cg1, cb1);
    process.compute_root()
            .parallel(y, 8);

    Func yuvColor = rgb2yuv(process);
    Func deltaFactor;
    Func walkPx;
    RDom f(-10, 10, -10, 10);

    walkPx(x0, y0, c, x, y) = yuvColor(c, x + x0, y + y0);
    Expr dc = walkPx(x0, y0, c, x, y) - yuvColor(c, x, y);
    dc = dc * dc;

    deltaFactor(x0, y0, c, x, y) = (1.f - clamp(dc * 5.f, 0.f, 1.f));

    Expr total = sum(deltaFactor(f.x, f.y, c, x, y));
    Expr colorMerge = sum(walkPx(f.x, f.y, c, x, y) * deltaFactor(f.x, f.y, c, x, y));

    Func dnColor;
    dnColor(c, x, y) = clamp(colorMerge / total, 0.f, 1.f);

    Func rgbColor = yuv2rgb(dnColor);

    Func rawP("rawP");

    rawP(c, x, y) = select(c == 3, 255, cast<uint8_t>(rgbColor(c, x, y) * 255.f));
//    rawP(c, x, y) = select(c == 0, cast<uint8_t>(cr1 * 255.f),
//                           select(c == 1, cast<uint8_t>(cg1 * 255.f),
//                                  select(c == 2, cast<uint8_t>(cb1 * 255.f), 255)));

    rawP.compute_root()
            .parallel(y, 8);

    //1760.794000
    //1539.196700
    //1392.188700

    Buffer<uint8_t> out = rawP.realize({3, img.width, img.height});
    auto *rawOut = (uint8_t *) malloc(img.width * img.height * 3);
    memcpy(rawOut, out.data(), img.width * img.height * 3);

    cout << "processed" << endl;
    return rawOut;
}*/

