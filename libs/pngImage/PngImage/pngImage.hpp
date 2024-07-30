//
// Created by imsac on 18/1/2022.
//
#ifndef DNGPROC_PNGIMAGE_H
#define DNGPROC_PNGIMAGE_H

#include <cstdint>


static const int PNG_CTYPE_RGBA = 0;
static const int PNG_CTYPE_RGB = 1;
static const int PNG_CTYPE_GRAY = 2;

void pngReadFile(const char *filename, int& width, int& height, uint8_t* &image);

void pngWriteRgb(const char *filename, uint8_t *data, int width, int height);

void pngWriteRgba(const char *filename, uint8_t *data, int width, int height);


#endif //DNGPROC_PNGIMAGE_H