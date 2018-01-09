#pragma once
#include "raiden.h"
#include <ImfNamespace.h>
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>

namespace IMF = OPENEXR_IMF_NAMESPACE;

void WriteOpenEXR(const char fileName[], IMF::Rgba* pixels,int w,int h) {
	IMF::RgbaOutputFile file(fileName, w, h, IMF::WRITE_RGBA);
    file.setFrameBuffer(pixels, 1, w);
    file.writePixels(h);
}