#include "imageio.h"
#include "lodepng.h"
#include <ImfNamespace.h>
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>
#include "spectrum.h"
#include "geometry.h"

namespace IMF = OPENEXR_IMF_NAMESPACE;

void WriteOpenEXR(const char fileName[], IMF::Rgba* pixels, int w, int h) {
	IMF::RgbaOutputFile file(fileName, w, h, IMF::WRITE_RGBA);
	file.setFrameBuffer(pixels, 1, w);
	file.writePixels(h);
}

void ReadOpenEXR(const char fileName[],std::vector<IMF::Rgba> &pixels,int &width,int &height)
{
	IMF::RgbaInputFile file(fileName);
	Imath::Box2i dw = file.dataWindow();
	width = dw.max.x - dw.min.x + 1;
	height = dw.max.y - dw.min.y + 1;
	pixels.reserve(height*width);
	file.setFrameBuffer(&pixels[0] - dw.min.x - dw.min.y * width, 1, width);
	file.readPixels(dw.min.y, dw.max.y);
}



void WriteImageToFile(const char* fileName, Float* data, int w, int h) {
	const char* fileExt;
	const char *ptr, c = '.';
	//���һ������c��λ��
	ptr = strrchr(fileName, c);
	//��ָ����� ������� 
	int pos = ptr - fileName;
	//��ȡ��׺
	fileExt = &fileName[pos + 1];
	//�жϺ�׺�Ƿ���ͬ
	if (0 == strcmp(fileExt, "png") || 0 == strcmp(fileExt, "PNG")) {
		std::vector<uint8_t> image;
		for (int i = 0; i<w*h; ++i) {
			Float rgb[3];
			rgb[0] = data[i * 3];
			rgb[1] = data[i * 3 + 1];
			rgb[2] = data[i * 3 + 2];

			rgb[0] = GammaCorrect(rgb[0]);
			rgb[1] = GammaCorrect(rgb[1]);
			rgb[2] = GammaCorrect(rgb[2]);

			rgb[0] = Clamp(rgb[0], 0, 1);
			rgb[1] = Clamp(rgb[1], 0, 1);
			rgb[2] = Clamp(rgb[2], 0, 1);

			image.push_back(rgb[0] * 255);//R
			image.push_back(rgb[1] * 255);//G
			image.push_back(rgb[2] * 255);//B
			image.push_back(255);		//A
		}

		unsigned error = lodepng::encode(fileName, image, w, h);
		if (error) {
			LError << "encoder error " << error << ": " << lodepng_error_text(error);
		}
	}
	else if (0 == strcmp(fileExt, "exr") || 0 == strcmp(fileExt, "EXR")) {
		std::vector<IMF::Rgba> image;
		for (int i = 0; i<w*h; ++i) {
			image.push_back(IMF::Rgba{ data[i * 3] ,data[i * 3 + 1] ,data[i * 3 + 2] ,1 });
		}
		WriteOpenEXR(fileName, &image[0], w, h);
	}
}

std::unique_ptr<RGBSpectrum[]> ReadImage(const char* fileName, Point2i* resolution) {
	const char* fileExt;
	const char *ptr, c = '.';
	//���һ������c��λ��
	ptr = strrchr(fileName, c);
	//��ָ����� ������� 
	int pos = ptr - fileName;
	//��ȡ��׺
	fileExt = &fileName[pos + 1];
	//�жϺ�׺�Ƿ���ͬ
	if (0 == strcmp(fileExt, "png") || 0 == strcmp(fileExt, "PNG")) {
		std::vector<unsigned char> rawData;
		uint32_t width, height;
		uint32_t error = lodepng::decode(rawData, width, height, fileName);
		if (error) {
			return nullptr;
		}

		std::unique_ptr<RGBSpectrum[]> rgbData(new RGBSpectrum[width*height]);
		uint32_t index = 0;
		Float invDiv = 1.0 / 255.0;
		for (int j = 0; j<height; ++j) {
			for (int i = 0; i<width; ++i) {
				int k = i*height+(height-1-j);
				//nt k=width*j+i;
				rgbData[k][0] = rawData[0 + index] * invDiv;
				rgbData[k][1] = rawData[1 + index] * invDiv;
				rgbData[k][2] = rawData[2 + index] * invDiv;
				index += 4;//����4���ֽڣ���ΪPNG�ǰ���RGBA 4Byte*8Bit�ķ�ʽ��֯��
			}
		}
		if (resolution) {
			*resolution = Point2i(width, height);
		}
		return rgbData;
	}
	else if (0 == strcmp(fileExt, "exr") || 0 == strcmp(fileExt, "EXR")) {
		std::vector<IMF::Rgba> rawData;
		int width, height;
		ReadOpenEXR(fileName,rawData,width,height);
		std::unique_ptr<RGBSpectrum[]> rgbData(new RGBSpectrum[width*height]);
		
		for (int j = 0; j<height; ++j) {
			for (int i = 0; i<width; ++i) {
				int k = i+j*width;
				rgbData[k][0] = rawData[k].r;
				rgbData[k][1] = rawData[k].g;
				rgbData[k][2] = rawData[k].b;
			}
		}
		if (resolution) {
			*resolution = Point2i(width, height);
		}
		return rgbData;
	}

	return nullptr;

}
