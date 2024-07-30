//
// Created by imsac on 7/1/2022.
//
#include <string>
#include "dngImage.hpp"
#include "dng_host.h"
#include "dng_stream.h"
#include "dng_file_stream.h"
#include "dng_info.h"
#include "dng_simple_image.h"
#include "dng_camera_profile.h"
#include "dng_image.h"
#include "dng_image_writer.h"
#include "dng_gain_map.h"
#include "dng_memory_stream.h"


GainMap::GainMap() {

}
GainMap::~GainMap() {
    if(gm != nullptr)
        free(gm);
}

GainMap::GainMap(int w, int h) {
    width = w;
    height = h;
    gm = new float[w * h * 4];
}
GainMap::GainMap(const GainMap& gainMap) {
    if(gainMap.gm == nullptr){
        if(gm != nullptr)
            delete[] gm;
        gm = nullptr;
    }
    else{
        if(gainMap.width != width || gainMap.height != height) {
            delete[] gm;
            gm = new float[gainMap.width * gainMap.height * 4];
        }
        memcpy(gm, gainMap.gm, gainMap.width * gainMap.height * 4 * sizeof(float));
    }
    width = gainMap.width;
    height = gainMap.height;
}

GainMap &GainMap::operator=(const GainMap &gainMap) {
    if(gainMap.gm == nullptr){
        if(gm != nullptr)
            delete[] gm;
        gm = nullptr;
    }
    else{
        if(gainMap.width != width || gainMap.height != height) {
            delete[] gm;
            gm = new float[gainMap.width * gainMap.height * 4];
        }
        memcpy(gm, gainMap.gm, gainMap.width * gainMap.height * 4 * sizeof(float));
    }
    width = gainMap.width;
    height = gainMap.height;
    return *this;
}

void GainMap::release() {
    if(gm != nullptr)
        free(gm);
}

bool GainMap::empty() {
    return gm == nullptr;
}

float &GainMap::operator()(int x, int y, int c) {
    if(empty())
        return *new float(1.f);
    else
        return gm[x + y * width + (width * height * c)];
}




void dngWrite(DngImg& dngImg, dng_stream& stream);
DngImg &dngRead(dng_stream& inStream);

static double* dngMatrix2double(dng_matrix_3by3& dngMatrix)
{
    return new double[9] {
            dngMatrix[0][0], dngMatrix[0][1], dngMatrix[0][2],
            dngMatrix[1][0], dngMatrix[1][1], dngMatrix[1][2],
            dngMatrix[2][0], dngMatrix[2][1], dngMatrix[2][2]
    };
}
static dng_matrix_3by3 double2dngMatrix(double *matrix)
{
    return {matrix[0], matrix[1], matrix[2],
            matrix[3], matrix[4], matrix[5],
            matrix[6], matrix[7], matrix[8]};
}

static int dngOrientation(dng_orientation o)
{
    if(o == dng_orientation::Normal())
        return DngImg::ORIENTATION_NORMAL;
    if(o == dng_orientation::Rotate90CW())
        return DngImg::ORIENTATION_ROTATE90;
    if(o == dng_orientation::Rotate180())
        return DngImg::ORIENTATION_ROTATE180;
    if(o == dng_orientation::Rotate90CCW())
        return DngImg::ORIENTATION_ROTATE270;
    else
        return 0;
}
static dng_orientation dngOrientation(int orientation)
{
    if(orientation == DngImg::ORIENTATION_NORMAL)
        return dng_orientation::Normal();
    else if(orientation == DngImg::ORIENTATION_ROTATE90)
        return dng_orientation::Rotate90CW();
    else if(orientation == DngImg::ORIENTATION_ROTATE180)
        return dng_orientation::Rotate180();
    else if(orientation == DngImg::ORIENTATION_ROTATE270)
        return dng_orientation::Rotate90CCW();
    else
        return dng_orientation::Normal();
}

static int getCfa(uint8_t cfa[2][2])
{
    if(cfa[0][0] == colorKeyRed && cfa[0][1] == colorKeyGreen && cfa[1][0] == colorKeyGreen && cfa[1][1] == colorKeyBlue)
        return DngImg::CFA_RGGB;
    else if(cfa[0][0] == colorKeyBlue   && cfa[0][1] == colorKeyGreen   && cfa[1][0] == colorKeyGreen   && cfa[1][1] == colorKeyRed)
        return DngImg::CFA_BGGR;
    else if(cfa[0][0] == colorKeyGreen  && cfa[0][1] == colorKeyRed     && cfa[1][0] == colorKeyBlue    && cfa[1][1] == colorKeyGreen)
        return DngImg::CFA_GRBG;
    else if(cfa[0][0] == colorKeyGreen  && cfa[0][1] == colorKeyBlue    && cfa[1][0] == colorKeyRed     && cfa[1][1] == colorKeyGreen)
        return DngImg::CFA_GBRG;
    else
        return -1;
}

DngImg &dngRead(void* data, int size)
{
    dng_stream stream(data, size);
    return dngRead(stream);
}
DngImg &dngRead(const string& in)
{
    dng_stream &stream = *new dng_file_stream(in.c_str(), false);
    return dngRead(stream);
}

DngImg &dngRead(dng_stream &inStream) {
    auto *host = new dng_host;
    AutoPtr<dng_stream> stream(&inStream);
    AutoPtr<dng_negative> negative;
    negative.Reset(host->Make_dng_negative());
    dng_info info;
    info.Parse(*host, *stream);
    info.PostParse(*host);
    if (!info.IsValidDNG()) 
        throw runtime_error("DngImage::dngRead: cant open image");

    negative->Parse(*host, *stream, info);
    negative->PostParse(*host, *stream, info);
    negative->ReadStage1Image(*host, *stream, info);

    auto *stage2 = (dng_simple_image *) negative->Stage1Image();

    uint32 pplanes = stage2->Planes();
    uint32 ptype = stage2->PixelType();

    dng_pixel_buffer buffer;
    stage2->GetPixelBuffer(buffer);

    uint32 pixels = stage2->Bounds().H() * stage2->Bounds().W() * pplanes;
    uint16_t *rawData = nullptr;
    if (ptype == ttShort) {
        rawData = new uint16_t[pixels * TagTypeSize(ttShort)];
        memmove(rawData, buffer.fData, pixels * TagTypeSize(ttShort));
    } else if (ptype == ttByte) {
        rawData = (uint16_t *) malloc(pixels * sizeof(uint64_t));
        auto *src = (uint8_t *) buffer.fData;
        auto *dst = (uint16_t *) rawData;
        for (uint32_t i = 0; i < pixels; i++)
            dst[i] = src[i];
    }

    auto *dngImg = new DngImg;
    dngImg->data = (uint16_t *) rawData;
    dngImg->whiteLevel = negative->WhiteLevel(0);
    auto *linearizationInfo = (dng_linearization_info *) negative->GetLinearizationInfo();
    dngImg->blackLevel = new uint32_t[4]{
            (uint32_t) linearizationInfo->BlackLevel(0, 0, 0).As_real64(),
            (uint32_t) linearizationInfo->BlackLevel(0, 1, 0).As_real64(),
            (uint32_t) linearizationInfo->BlackLevel(1, 0, 0).As_real64(),
            (uint32_t) linearizationInfo->BlackLevel(1, 1, 0).As_real64()
    };
    dng_vector_3 camNeutral = negative->CameraNeutral();
    dngImg->camNeutral = new double[3]
            {
                    camNeutral[0],
                    camNeutral[1],
                    camNeutral[2],
            };

    dngImg->orientation = dngOrientation(negative->BaseOrientation());

    auto exif = negative->Metadata().GetExif();

    dngImg->aperture = exif->fApertureValue.As_real64();
    dngImg->exposureTime = exif->fExposureTime.As_real64();
    dngImg->focalLength = exif->fFocalLength.As_real64();
    dngImg->iso = exif->fISOSpeedRatings[0];

    uint8 cfa[2][2];
    cfa[0][0] = negative->GetMosaicInfo()->fCFAPattern[0][0];
    cfa[0][1] = negative->GetMosaicInfo()->fCFAPattern[0][1];
    cfa[1][0] = negative->GetMosaicInfo()->fCFAPattern[1][0];
    cfa[1][1] = negative->GetMosaicInfo()->fCFAPattern[1][1];
    dngImg->cfa = getCfa(cfa);

    dng_opcode_GainMap *gainMaps[4];
    int count = 0;
    for (uint32_t i = 0; i < negative->OpcodeList2().Count(); ++i) {
        dng_opcode *code = &negative->OpcodeList2().Entry(i);
        if(code->OpcodeID() == dngOpcode_GainMap)
        {
            gainMaps[count] = (dng_opcode_GainMap*) code;
            count++;
        }
    }
    if(count == 4) {
        int width = gainMaps[0]->fGainMap->Points().h;
        int height = gainMaps[0]->fGainMap->Points().v;
        dngImg->gm = GainMap(width, height);
        GainMap& gm = dngImg->gm;
        for (int i = 0; i < 4; ++i) {
            dng_gain_map *gainMap = gainMaps[i]->fGainMap.Get();
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    gm(x, y, i) = gainMap->Entry(y, x, 0);
                }
            }
        }
    }

    dng_camera_profile profile = negative->ProfileByIndex(0);
    uint32_t illuminant1 = profile.CalibrationIlluminant1();
    uint32_t illuminant2 = profile.CalibrationIlluminant2();
    switch (illuminant1) {
        case lsStandardLightA:
            dngImg->illuminant1 = DngImg::ILLUMINANT_SA;
            break;
        case lsStandardLightB:
            dngImg->illuminant1 = DngImg::ILLUMINANT_SB;
            break;
        case lsStandardLightC:
            dngImg->illuminant1 = DngImg::ILLUMINANT_SC;
            break;
        case lsD50:
            dngImg->illuminant1 = DngImg::ILLUMINANT_D_50;
            break;
        case lsD55:
            dngImg->illuminant1 = DngImg::ILLUMINANT_D_55;
            break;
        case lsD65:
            dngImg->illuminant1 = DngImg::ILLUMINANT_D_65;
            break;
        case lsD75:
            dngImg->illuminant1 = DngImg::ILLUMINANT_D_75;
            break;
        default:
            dngImg->illuminant1 = DngImg::ILLUMINANT_D_50;
            break;
    }
    switch (illuminant2) {
        case lsStandardLightA:
            dngImg->illuminant2 = DngImg::ILLUMINANT_SA;
            break;
        case lsStandardLightB:
            dngImg->illuminant2 = DngImg::ILLUMINANT_SB;
            break;
        case lsStandardLightC:
            dngImg->illuminant2 = DngImg::ILLUMINANT_SC;
            break;
        case lsD50:
            dngImg->illuminant2 = DngImg::ILLUMINANT_D_50;
            break;
        case lsD55:
            dngImg->illuminant2 = DngImg::ILLUMINANT_D_55;
            break;
        case lsD65:
            dngImg->illuminant2 = DngImg::ILLUMINANT_D_65;
            break;
        case lsD75:
            dngImg->illuminant2 = DngImg::ILLUMINANT_D_75;
            break;
        default:
            dngImg->illuminant2 = DngImg::ILLUMINANT_D_50;
            break;
    }

    if (!negative->CameraCalibration1().IsEmpty()) {
        dng_matrix_3by3 calibration1 = negative->CameraCalibration1();
        dngImg->calibration1 = dngMatrix2double(calibration1);
    }
    if (!negative->CameraCalibration2().IsEmpty()) {
        dng_matrix_3by3 calibration2 = negative->CameraCalibration2();
        dngImg->calibration2 = dngMatrix2double(calibration2);
    }

    dng_matrix_3by3 color1 = profile.ColorMatrix1();
    dng_matrix_3by3 color2 = profile.ColorMatrix2();
    dngImg->color1 = dngMatrix2double(color1);
    dngImg->color2 = dngMatrix2double(color2);

    if (!profile.ForwardMatrix1().IsEmpty()) {
        dng_matrix_3by3 forward1 = profile.ForwardMatrix1();
        dngImg->forward1 = dngMatrix2double(forward1);
    }
    if (!profile.ForwardMatrix2().IsEmpty()) {
        dng_matrix_3by3 forward2 = profile.ForwardMatrix2();
        dngImg->forward2 = dngMatrix2double(forward2);
    }

    dngImg->width = stage2->Width();
    dngImg->rowStride = dngImg->width;
    dngImg->height = stage2->Height();
    dngImg->dataLength = dngImg->rowStride * dngImg->height;
    dngImg->gmW = 0;
    dngImg->gmH = 0;
    return *dngImg;
}

void dngWrite(DngImg& dngImg, const string& out)
{
    dng_file_stream stream(out.c_str(), true);
    dngWrite(dngImg, stream);
}
void dngWrite(DngImg& dngImg, uint8_t *& data, uint64_t& len)
{
    dng_memory_allocator allocator;
    dng_memory_stream stream(allocator);
    dngWrite(dngImg, stream);
    len = stream.Length();
    data = (uint8_t*)malloc(len);
    stream.SetReadPosition(0);
    stream.Get(data, len);
}

void dngWrite(DngImg& dngImg, dng_stream& stream) {
    const bool enableCompression = true;
    const int width = dngImg.width;
    const int height = dngImg.height;

    dng_host host;

    host.SetSaveLinearDNG(false);
    host.SetSaveDNGVersion(dngVersion_SaveDefault);

    AutoPtr<dng_negative> negative(host.Make_dng_negative());

    // Create lens shading map for each channel
    if (!dngImg.gm.empty()) {
        // Rearrange the shading map channels to match the sensor layout
//        const auto &rggbShadingMap = imageMetadata.shadingMap();

        for (int c = 0; c < 4; c++) {
            dng_point channelGainMapPoints(dngImg.gm.height, dngImg.gm.height);

            AutoPtr<dng_gain_map> gainMap(new dng_gain_map(host.Allocator(),
                                                           channelGainMapPoints,
                                                           dng_point_real64(1.0 / (dngImg.gm.height),
                                                                            1.0 / (dngImg.gm.width)),
                                                           dng_point_real64(0, 0),
                                                           1));

            for (int y = 0; y < dngImg.gm.height; y++) {
                for (int x = 0; x < dngImg.gm.width; x++) {
                    gainMap->Entry(y, x, 0) = dngImg.gm(y, x, c);//.at<float>(y, x);
                }
            }

            int left = 0;
            int top = 0;

            if (c == 0) {
                left = 0;
                top = 0;
            } else if (c == 1) {
                left = 1;
                top = 0;
            } else if (c == 2) {
                left = 0;
                top = 1;
            } else if (c == 3) {
                left = 1;
                top = 1;
            }

            dng_rect gainMapArea(top, left, height, width);
            AutoPtr<dng_opcode> gainMapOpCode(new dng_opcode_GainMap(dng_area_spec(gainMapArea, 0, 1, 2, 2), gainMap));

            negative->OpcodeList2().Append(gainMapOpCode);
        }
    }

    negative->SetModelName("ModelName");
    negative->SetLocalName("LocalName");

    negative->SetColorKeys(colorKeyRed, colorKeyGreen, colorKeyBlue);

    uint32_t phase;

    switch (dngImg.cfa) {
        case DngImg::CFA_GRBG:
            phase = 0;
            break;

        default:
        case DngImg::CFA_RGGB:
            phase = 1;
            break;

        case DngImg::CFA_BGGR:
            phase = 2;
            break;

        case DngImg::CFA_GBRG:
            phase = 3;
            break;
    }

    negative->SetBayerMosaic(phase);
    negative->SetColorChannels(3);

    negative->SetQuadBlacks(dngImg.blackLevel[0],
                            dngImg.blackLevel[1],
                            dngImg.blackLevel[2],
                            dngImg.blackLevel[3]);

    negative->SetWhiteLevel( dngImg.whiteLevel);

    // Square pixels
    negative->SetDefaultScale(dng_urational(1, 1), dng_urational(1, 1));

    negative->SetDefaultCropSize(width, height);
    negative->SetCameraNeutral(dng_vector_3(dngImg.camNeutral[0], dngImg.camNeutral[1], dngImg.camNeutral[2]));

    // Set metadata
    auto exif = negative->Metadata().GetExif();
    exif->SetApertureValue(dngImg.aperture);
    exif->SetExposureTime(dngImg.exposureTime);
    exif->fFocalLength = dng_urational((int)(dngImg.focalLength * 10000.), 10000);
    exif->fISOSpeed = dngImg.iso;
    exif->fISOSpeedRatings[0] = dngImg.iso;
    exif->fISOSpeedRatings[1] = dngImg.iso;
    exif->fISOSpeedRatings[2] = dngImg.iso;

    // Set orientation
    dng_orientation dngOrientation;
    switch (dngImg.orientation) {
        default:
        case DngImg::ORIENTATION_NORMAL: //PORTRAIT
            dngOrientation = dng_orientation::Normal();
            break;

        case DngImg::ORIENTATION_ROTATE90: //LANDSCAPE
            dngOrientation = dng_orientation::Rotate90CW();
            break;

        case DngImg::ORIENTATION_ROTATE180: //REVERSE_PORTRAIT
            dngOrientation = dng_orientation::Rotate180();
            break;

        case DngImg::ORIENTATION_ROTATE270: //REVERSE_LANDSCAPE
            dngOrientation = dng_orientation::Rotate90CCW();
            break;
    }
    negative->SetBaseOrientation(dngOrientation);

    // Set up camera profile
    AutoPtr<dng_camera_profile> cameraProfile(new dng_camera_profile());
    // Color matrices
    cameraProfile->SetColorMatrix1(double2dngMatrix(dngImg.color1));
    cameraProfile->SetColorMatrix2(double2dngMatrix(dngImg.color2));

    // Forward matrices
    if (dngImg.forward1 != nullptr && dngImg.forward2 != nullptr) {
        cameraProfile->SetForwardMatrix1(double2dngMatrix(dngImg.forward1));
        cameraProfile->SetForwardMatrix2(double2dngMatrix(dngImg.forward2));
    }

    // Set camera calibration matrix
    if (dngImg.calibration1 != nullptr) {
        negative->SetCameraCalibration1(double2dngMatrix(dngImg.calibration1));
    }
    if (dngImg.calibration2 != nullptr) {
        negative->SetCameraCalibration2(double2dngMatrix(dngImg.calibration2));
    }


    uint32_t illuminant1 = 0;
    uint32_t illuminant2 = 0;

    // Convert to DNG format

    switch(dngImg.illuminant1) {
        case DngImg::ILLUMINANT_SA:
            illuminant1 = lsStandardLightA;
            break;
        case DngImg::ILLUMINANT_SB:
            illuminant1 = lsStandardLightB;
            break;
        case DngImg::ILLUMINANT_SC:
            illuminant1 = lsStandardLightC;
            break;
        case DngImg::ILLUMINANT_D_50:
            illuminant1 = lsD50;
            break;
        case DngImg::ILLUMINANT_D_55:
            illuminant1 = lsD55;
            break;
        case DngImg::ILLUMINANT_D_65:
            illuminant1 = lsD65;
            break;
        case DngImg::ILLUMINANT_D_75:
            illuminant1 = lsD75;
            break;
    }
    switch(dngImg.illuminant2) {
        case DngImg::ILLUMINANT_SA:
            illuminant2 = lsStandardLightA;
            break;
        case DngImg::ILLUMINANT_SB:
            illuminant2 = lsStandardLightB;
            break;
        case DngImg::ILLUMINANT_SC:
            illuminant2 = lsStandardLightC;
            break;
        case DngImg::ILLUMINANT_D_50:
            illuminant2 = lsD50;
            break;
        case DngImg::ILLUMINANT_D_55:
            illuminant2 = lsD55;
            break;
        case DngImg::ILLUMINANT_D_65:
            illuminant2 = lsD65;
            break;
        case DngImg::ILLUMINANT_D_75:
            illuminant2 = lsD75;
            break;
    }

    cameraProfile->SetCalibrationIlluminant1(illuminant1);
    cameraProfile->SetCalibrationIlluminant2(illuminant2);

    cameraProfile->SetName("I");
    cameraProfile->SetEmbedPolicy(pepAllowCopying);

    // This ensures profile is saved
    cameraProfile->SetWasReadFromDNG();

    negative->AddProfile(cameraProfile);

    // Finally add the raw data to the negative
    dng_rect dngArea(height, width);
    dng_pixel_buffer dngBuffer;

    AutoPtr<dng_image> dngImage(host.Make_dng_image(dngArea, 1, ttShort));

    dngBuffer.fArea = dngArea;
    dngBuffer.fPlane = 0;
    dngBuffer.fPlanes = 1;
    dngBuffer.fRowStep = dngBuffer.fPlanes * width;
    dngBuffer.fColStep = dngBuffer.fPlanes;
    dngBuffer.fPixelType = ttShort;
    dngBuffer.fPixelSize = TagTypeSize(ttShort);
    dngBuffer.fData = dngImg.data;

    dngImage->Put(dngBuffer);

    // Build the DNG images
    negative->SetStage1Image(dngImage);
    negative->BuildStage2Image(host);
    negative->BuildStage3Image(host);

    negative->SynchronizeMetadata();

    // Write DNG file to disk
    AutoPtr<dng_image_writer> dngWriter(new dng_image_writer());

    dngWriter->WriteDNG(host, stream, *negative.Get(), nullptr, dngVersion_SaveDefault, !enableCompression);
    stream.Flush();
}


DngImg::DngImg(){

}
DngImg::DngImg(const DngImg &dng){
    width = dng.width;
    height = dng.height;
    rowStride = dng.rowStride;

    cfa = dng.cfa;
    gm = dng.gm;
    gmW = dng.gmW;
    gmH = dng.gmH;
    whiteLevel = dng.whiteLevel;
    if(blackLevel == nullptr)
        blackLevel = new uint32_t[4];
    blackLevel[0] = dng.blackLevel[0];
    blackLevel[1] = dng.blackLevel[1];
    blackLevel[2] = dng.blackLevel[2];
    blackLevel[3] = dng.blackLevel[3];

    aperture = dng.aperture;
    exposureTime = dng.exposureTime;
    focalLength = dng.focalLength;
    iso = dng.iso;                 

    orientation = dng.orientation;

    if(dng.calibration1 != nullptr){
        if(calibration1 == nullptr)
            calibration1 = new double[9];
        memcpy(calibration1, dng.calibration1, 9 * sizeof(double));
    }
    if(dng.calibration2 != nullptr){
        if(calibration2 == nullptr)
            calibration2 = new double[9];
        memcpy(calibration2, dng.calibration2, 9 * sizeof(double));
    }
    if(dng.color1 != nullptr){
        if(color1 == nullptr)
            color1 = new double[9];
        memcpy(color1, dng.color1, 9 * sizeof(double));
    }
    if(dng.color2 != nullptr){
        if(color2 == nullptr)
            color2 = new double[9];
        memcpy(color2, dng.color2, 9 * sizeof(double));
    }
    if(dng.forward1 != nullptr){
        if(forward1 == nullptr)
            forward1 = new double[9];
        memcpy(forward1, dng.forward1, 9 * sizeof(double));
    }
    if(dng.forward2 != nullptr){
        if(forward2 == nullptr)
            forward2 = new double[9];
        memcpy(forward2, dng.forward2, 9 * sizeof(double));
    }
    if(dng.camNeutral != nullptr){
        if(camNeutral == nullptr)
            camNeutral = new double[3];
        memcpy(camNeutral, dng.camNeutral, 3 * sizeof(double));
    }

    illuminant1 = dng.illuminant1;
    illuminant2 = dng.illuminant2;

    if(dng.dataLength != dataLength){
        if(data != nullptr)
            delete[] data;
        data = new uint16_t[dng.dataLength];
        dataLength = dng.dataLength;
    }
    memcpy(data, dng.data, dataLength * sizeof(uint16_t));
    dataLength = dng.dataLength;
}

DngImg::~DngImg(){
    if(blackLevel != nullptr)
        delete[] blackLevel;
    if(calibration1 != nullptr)
        delete[] calibration1;
    if(calibration2 != nullptr)
        delete[] calibration2;
    if(color1 != nullptr)
        delete[] color1;
    if(color2 != nullptr)
        delete[] color2;
    if(forward1 != nullptr)
        delete[] forward1;
    if(forward2 != nullptr)
        delete[] forward2;
    if(camNeutral != nullptr)
        delete[] camNeutral;
    if(data != nullptr)
        delete[] data;
}

DngImg &DngImg::operator=(const DngImg &dng){
    width = dng.width;
    height = dng.height;
    rowStride = dng.rowStride;

    cfa = dng.cfa;
    gm = dng.gm;
    gmW = dng.gmW;
    gmH = dng.gmH;
    whiteLevel = dng.whiteLevel;
    if(blackLevel == nullptr)
        blackLevel = new uint32_t[4];
    blackLevel[0] = dng.blackLevel[0];
    blackLevel[1] = dng.blackLevel[1];
    blackLevel[2] = dng.blackLevel[2];
    blackLevel[3] = dng.blackLevel[3];

    aperture = dng.aperture;
    exposureTime = dng.exposureTime;
    focalLength = dng.focalLength;
    iso = dng.iso;                 

    orientation = dng.orientation;

    if(dng.calibration1 != nullptr){
        if(calibration1 == nullptr)
            calibration1 = new double[9];
        memcpy(calibration1, dng.calibration1, 9 * sizeof(double));
    }
    if(dng.calibration2 != nullptr){
        if(calibration2 == nullptr)
            calibration2 = new double[9];
        memcpy(calibration2, dng.calibration2, 9 * sizeof(double));
    }
    if(dng.color1 != nullptr){
        if(color1 == nullptr)
            color1 = new double[9];
        memcpy(color1, dng.color1, 9 * sizeof(double));
    }
    if(dng.color2 != nullptr){
        if(color2 == nullptr)
            color2 = new double[9];
        memcpy(color2, dng.color2, 9 * sizeof(double));
    }
    if(dng.forward1 != nullptr){
        if(forward1 == nullptr)
            forward1 = new double[9];
        memcpy(forward1, dng.forward1, 9 * sizeof(double));
    }
    if(dng.forward2 != nullptr){
        if(forward2 == nullptr)
            forward2 = new double[9];
        memcpy(forward2, dng.forward2, 9 * sizeof(double));
    }
    if(dng.camNeutral != nullptr){
        if(camNeutral == nullptr)
            camNeutral = new double[3];
        memcpy(camNeutral, dng.camNeutral, 3 * sizeof(double));
    }

    illuminant1 = dng.illuminant1;
    illuminant2 = dng.illuminant2;

    if(dng.dataLength != dataLength){
        if(data != nullptr)
            delete[] data;
        data = new uint16_t[dng.dataLength];
        dataLength = dng.dataLength;
    }
    memcpy(data, dng.data, dataLength * sizeof(uint16_t));
    dataLength = dng.dataLength;
    return *this;
}


void DngImg::log() {
    string str = "";
    str += "dngImage data:\n";
    str += "width: " + to_string(width) + "\n";
    str += "height: " + to_string(height) + "\n";
    str += "rowStride: " + to_string(rowStride) + "\n";
    str += "dataLength: " + to_string(dataLength) + "\n";

    if(cfa == DngImg::CFA_RGGB)
        str += "cfa: RGGB\n";
    else if(cfa == DngImg::CFA_BGGR)
        str += "cfa: BGGR\n";
    else if(cfa == DngImg::CFA_GRBG)
        str += "cfa: GRBG\n";
    else if(cfa == DngImg::CFA_GBRG)
        str += "cfa: GBRG\n";
    else
        str += "cfa: null\n";
    if (gm.empty())
        str += "gainMap: null\n";
    else
        str += "gainMap: " + to_string(gm.width) + "x" + to_string(gm.height) + "\n";

    str += "whiteLevel: " + to_string(whiteLevel) + "\n";
    str += "blackLevel: " + to_string(blackLevel[0]);
    str += " " + to_string(blackLevel[1]);
    str += " " + to_string(blackLevel[2]);
    str += " " + to_string(blackLevel[3]) + "\n";

    str += "aperture: " + to_string(aperture) + "\n";
    str += "exposureTime: " + to_string(exposureTime) + "\n";
    str += "focalLength: " + to_string(focalLength) + "\n";
    str += "iso: " + to_string(iso) + "\n";
    if(orientation == ORIENTATION_NORMAL)
        str += "orientation: 0\n";
    else if(orientation == ORIENTATION_ROTATE90)
        str += "orientation: 90\n";
    else if(orientation == ORIENTATION_ROTATE180)
        str += "orientation: 180\n";
    else if(orientation == ORIENTATION_ROTATE270)
        str += "orientation: 270\n";
    else
        str += "orientation: null\n";

    if (calibration1 == nullptr)
        str += "calibration Matrix 1: null\n";
    else {
        str += "calibration Matrix 1: ok\n";
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                str += to_string(calibration1[x + y * 3]) + " ";
            }
            str += "\n";
        }
    }
    if (calibration2 == nullptr)
        str += "calibration Matrix 2: null\n";
    else {
        str += "calibration Matrix 2: ok\n";
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                str += to_string(calibration2[x + y * 3]) + " ";
            }
            str += "\n";
        }
    }
    if (color1 == nullptr)
        str += "color Matrix 1: null\n";
    else {
        str += "color Matrix 1: ok\n";
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                str += to_string(color1[x + y * 3]) + " ";
            }
            str += "\n";
        }
    }
    if (color2 == nullptr)
        str += "color Matrix 2: null\n";
    else {
        str += "color Matrix 2: ok\n";
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                str += to_string(color2[x + y * 3]) + " ";
            }
            str += "\n";
        }
    }
    if (forward1 == nullptr)
        str += "forward Matrix 1: null\n";
    else {
        str += "forward Matrix 1: ok\n";
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                str += to_string(forward1[x + y * 3]) + " ";
            }
            str += "\n";
        }
    }
    if (forward2 == nullptr)
        str += "forward Matrix 2: null\n";
    else {
        str += "forward Matrix 2: ok\n";
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                str += to_string(forward2[x + y * 3]) + " ";
            }
            str += "\n";
        }
    }

    if(illuminant1 == DngImg::ILLUMINANT_SA)
        str += "illuminant1: StandardA\n";
    else if(illuminant1 == DngImg::ILLUMINANT_SB)
        str += "illuminant1: StandardB\n";
    else if(illuminant1 == DngImg::ILLUMINANT_SC)
        str += "illuminant1: StandardC\n";
    else if(illuminant1 == DngImg::ILLUMINANT_D_50)
        str += "illuminant1: D50\n";
    else if(illuminant1 == DngImg::ILLUMINANT_D_55)
        str += "illuminant1: D55\n";
    else if(illuminant1 == DngImg::ILLUMINANT_D_65)
        str += "illuminant1: D65\n";
    else if(illuminant1 == DngImg::ILLUMINANT_D_75)
        str += "illuminant1: D75\n";
    else
        str += "illuminant1: null\n";

    if(illuminant2 == DngImg::ILLUMINANT_SA)
        str += "illuminant2: StandardA\n";
    else if(illuminant2 == DngImg::ILLUMINANT_SB)
        str += "illuminant2: StandardB\n";
    else if(illuminant2 == DngImg::ILLUMINANT_SC)
        str += "illuminant2: StandardC\n";
    else if(illuminant2 == DngImg::ILLUMINANT_D_50)
        str += "illuminant2: D50\n";
    else if(illuminant2 == DngImg::ILLUMINANT_D_55)
        str += "illuminant2: D55\n";
    else if(illuminant2 == DngImg::ILLUMINANT_D_65)
        str += "illuminant2: D65\n";
    else if(illuminant2 == DngImg::ILLUMINANT_D_75)
        str += "illuminant2: D75\n";
    else
        str += "illuminant2: null\n";
    cout << str << endl;
}
