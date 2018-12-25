#ifndef SRC_CORE_IMAGEIO_H_
#define SRC_CORE_IMAGEIO_H_
#pragma once
#include "raiden.h"
#include "geometry.h"
void WriteImageToFile(const char* fileName, Float* data, int w, int h);

std::unique_ptr<RGBSpectrum[]> ReadImage(const char* fileName, Point2i* resolution);


bool IsImageFormatSupported(const char* fileName);
#endif