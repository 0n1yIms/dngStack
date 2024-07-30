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



bool checkFile(string fileDir){
    ifstream file;
    file.open(fileDir.c_str());
    return file.good();
}

bool checkPath(string path){
    return filesystem::exists(path);
}

void merge(string dir, int cant)
{
    DngImg *imgs = new DngImg[cant];

    cout << "opening images" << endl;

    int format = -1;
    if (checkFile(dir + "/f0.dng"))
        format = 0;
    else if (checkFile(dir + "/f (1).dng"))
        format = 1;
    else{
        cerr << "dngs not found" << endl;
        throw new runtime_error("dngs not found");
    }

    for (int i = 0; i < cant; ++i){
        if(format == 0){

            imgs[i] = dngRead(dir + "/f" + to_string(i) + ".dng");
        }
        else if (format == 1){

            imgs[i] = dngRead(dir + "/f (" + to_string(i + 1) + ").dng");
        }
    }

    

    uint16_t *outDngd = new uint16_t[imgs[0].width * imgs[0].height * 4];

    
    for (int x = 0; x < imgs[0].width * imgs[0].height * 4; x++)
    {
        double v = 0;
        for (int idx = 0; idx < cant; idx++)
            v += (double)imgs[idx].data[x];
        outDngd[x] = (uint16_t)(v / cant);
    }
    
    imgs[0].data = outDngd;
    cout << "writing dng" << endl;
    dngWrite(imgs[0], dir + "/dngout.dng");

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

    string dir;
    int num;
    cin >> dir;
    cin >> num;

    merge(dir, num);
    
    return 0;
}
