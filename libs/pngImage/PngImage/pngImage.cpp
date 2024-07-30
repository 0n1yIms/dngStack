//
// Created by imsac on 18/1/2022.
//

#include "pngImage.hpp"
#include "png.h"
#include <cstdlib>
#include <cstdio>
#include "iostream"

png_byte color_type;
png_byte bit_depth;

uint8_t *pngRgb2rgb(png_bytepp data, int width, int height)
{
    uint8_t *image = (uint8_t*) malloc(3 * width * height * sizeof(uint8_t));
    for (int y = 0; y < height; ++y) {
        png_bytep row = data[y];
        for (int x = 0; x < width; ++x) {
            image[x * 3 + 0 + y * width * 3] = row[x * 3 + 0];
            image[x * 3 + 1 + y * width * 3] = row[x * 3 + 1];
            image[x * 3 + 2 + y * width * 3] = row[x * 3 + 2];
        }
    }
    return image;
}

uint8_t *pngRgba2rgb(png_bytepp data, int width, int height)
{
    uint8_t *image = (uint8_t*) malloc(3 * width * height * sizeof(uint8_t));
    for (int y = 0; y < height; ++y) {
        png_bytep row = data[y];
        for (int x = 0; x < width; ++x) {
            image[x * 3 + 0 + y * width * 3] = row[x * 4 + 0];
            image[x * 3 + 1 + y * width * 3] = row[x * 4 + 1];
            image[x * 3 + 2 + y * width * 3] = row[x * 4 + 2];
        }
    }
    return image;
}

void pngReadFile(const char *filename, int& width, int& height, uint8_t* &image) {
    FILE *fp = fopen(filename, "rb");
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) abort();
    png_infop info = png_create_info_struct(png);
    if(!info) abort();
    if(setjmp(png_jmpbuf(png))) abort();
    png_init_io(png, fp);
    png_read_info(png, info);

    width      = png_get_image_width(png, info);
    height     = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth  = png_get_bit_depth(png, info);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if(bit_depth == 16)
        png_set_strip_16(png);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);
    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    // These color_type don't have an alpha channel then fill it with 0xff.
    if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    png_bytepp rows = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        rows[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
    }
    png_read_image(png, rows);

    if(png_get_color_type(png, info) == PNG_COLOR_TYPE_RGB)
        image = pngRgb2rgb(rows, width, height);
        image = pngRgba2rgb(rows, width, height);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);
}


png_bytepp toPngFormat(uint8_t *data, int width, int height)
{
    auto rows = (png_bytepp) malloc(3 * width * height * sizeof(png_byte));

    for (int y = 0; y < height; ++y) {
//        auto row = (png_bytep) malloc(3 * width * sizeof(png_byte));
        rows[y] = &data[y * width * 3];
//        for (int x = 0; x < width; ++x) {
//            row[x * 3 + 0] = data[x * 3 + 0 + y * width * 3];
//            row[x * 3 + 1] = data[x * 3 + 1 + y * width * 3];
//            row[x * 3 + 2] = data[x * 3 + 2 + y * width * 3];
//        }
    }
    return rows;
}

png_bytepp RGBAtoPngFormat(uint8_t *data, int width, int height)
{
    auto rows = (png_bytepp) malloc(4 * width * height * sizeof(png_byte));

    for (int y = 0; y < height; ++y) {
        rows[y] = &data[y * width * 4];

    }
    return rows;
}

void pngWriteRgb(const char *filename, uint8_t *data, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if(!fp) abort();
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();
    png_infop info = png_create_info_struct(png);
    if (!info) abort();
    if (setjmp(png_jmpbuf(png))) abort();
    png_init_io(png, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
            png,
            info,
            width,height,
            8,
            PNG_COLOR_TYPE_RGB,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    //png_set_filler(png, 0, PNG_FILLER_AFTER);

    png_bytepp bytes = toPngFormat(data, width, height);
    png_write_image(png, bytes);
//    png_write_image(png, toPngFormat(data, width, height));
    png_write_end(png, NULL);

//    for(int y = 0; y < height; y++) {
//        free(bytes[y]);
//    }
//    free(bytes);

    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

void pngWriteRgba(const char *filename, uint8_t *data, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if(!fp) abort();
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();
    png_infop info = png_create_info_struct(png);
    if (!info) abort();
    if (setjmp(png_jmpbuf(png))) abort();
    png_init_io(png, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
            png,
            info,
            width,height,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    //png_set_filler(png, 0, PNG_FILLER_AFTER);

    png_bytepp bytes = RGBAtoPngFormat(data, width, height);
    png_write_image(png, bytes);
//    png_write_image(png, toPngFormat(data, width, height));
    png_write_end(png, NULL);

    for(int y = 0; y < height; y++) {
        free(bytes[y]);
    }
    free(bytes);

    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

