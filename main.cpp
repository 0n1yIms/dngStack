#define DEBUG

#include <dngImage.hpp>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace std;

bool checkFile(string fileDir)
{
    ifstream file;
    file.open(fileDir.c_str());
    return file.good();
}

bool checkPath(string path)
{
    return filesystem::exists(path);
}

void merge(string dir, int cant)
{
    DngImg *imgs = new DngImg[cant];

    cout << "opening images" << endl;
    cout << dir + "/f _.dng" << endl;

    int format = -1;
    if (checkFile(dir + "f0.dng"))
        format = 0;
    else if (checkFile(dir + "f (1).dng"))
        format = 1;
    else
    {
        cerr << "dngs not found" << endl;
        throw new runtime_error("dngs not found");
    }
    if (format == 0)
        cout << "format: fx.dng" << endl;
    else
        cout << "format: fx (1).dng" << endl;

    for (int i = 0; i < cant; ++i)
    {
        string fil;
        if (format == 0)
            fil = dir + "f" + to_string(i) + ".dng";
        else if (format == 1)
            fil = dir + "f (" + to_string(i + 1) + ").dng";
        if (!checkFile(fil))
        {
            cerr << fil << " not found" << endl;
            throw new runtime_error("dng not found");
        }

        imgs[i] = dngRead(fil);
    }

    cout << "merging" << endl;

    uint16_t *outDngd = new uint16_t[imgs[0].width * imgs[0].height * 4];

    int len = imgs[0].width * imgs[0].height;

    double factor = 65536. / imgs[0].whiteLevel;

#pragma omp parallel for
    for (int x = 0; x < len; x++)
    {
        double v = 0;
        for (int idx = 0; idx < cant; idx++)
            v += ((double)imgs[idx].data[x]) * factor;

        outDngd[x] = (uint16_t)(v / cant);
    }

    imgs[0].whiteLevel = 65535;
    imgs[0].blackLevel[0] = imgs[0].blackLevel[0] * (uint32_t)factor;
    imgs[0].blackLevel[1] = imgs[0].blackLevel[1] * (uint32_t)factor;
    imgs[0].blackLevel[2] = imgs[0].blackLevel[2] * (uint32_t)factor;
    imgs[0].blackLevel[3] = imgs[0].blackLevel[3] * (uint32_t)factor;


    imgs[0].data = outDngd;
    cout << "writing dng" << endl;
    dngWrite(imgs[0], dir + "/dngout.dng");

    // F:\stacks\stack78
    // F:/stacks/stack78/

    // uint8_t *process = raw2rgb(img);
    // string outDir = dir + "/out.png";
    // pngWriteRgb(outDir.c_str(), process, imgs2[0].width, imgs2[0].height);

    // DngImg motionCamDng = dngRead(dir + "/motioncam.dng");
    // process = raw2rgb(motionCamDng);
    // outDir = dir + "/motioncam.png";
    // pngWriteRgb(outDir.c_str(), process, motionCamDng.width, motionCamDng.height);

    delete[] imgs;
}

int main()
{
    cout << "syntaxis (F:/photos/dngs/ 10)" << endl;
    string dir;
    int num;
    cin >> dir;
    cin >> num;

    cout << "merge " << dir << " " << num << endl;

    merge(dir, num);

    return 0;
}
