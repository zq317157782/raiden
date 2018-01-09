#pragma once
#include "raiden.h"
#include "lodepng.h"
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

void WriteImageToFile(const char* fileName,Float* data,int w,int h){
    const char* fileExt;
    const char *ptr, c = '.'; 
    //最后一个出现c的位置
    ptr = strrchr(fileName,c); 
    //用指针相减 求得索引 
    int pos = ptr-fileName;
    //获取后缀
    fileExt=&fileName[pos+1];
    //判断后缀是否相同
    if (0==strcmp(fileExt,"png")||0==strcmp(fileExt,"PNG")){
        std::vector<uint8_t> image;
        for(int i=0;i<w*h;++i){
            Float rgb[3];
            rgb[0]=data[i*3];
            rgb[1]=data[i*3+1];
            rgb[2]=data[i*3+2];
            
            rgb[0]=GammaCorrect(rgb[0]);
			rgb[1]=GammaCorrect(rgb[1]);
			rgb[2]=GammaCorrect(rgb[2]);

            rgb[0] = Clamp(rgb[0], 0, 1);
			rgb[1] = Clamp(rgb[1], 0, 1);
			rgb[2] = Clamp(rgb[2], 0, 1);

            image.push_back(rgb[0]*255);//R
			image.push_back(rgb[1]*255);//G
			image.push_back(rgb[2]*255);//B
			image.push_back(255);		//A
        }

        unsigned error = lodepng::encode(fileName,image, w,h);
	    if (error) {
	         LError<<"encoder error " << error << ": "<< lodepng_error_text(error);
	    }
    }
    else if(0==strcmp(fileExt,"exr")||0==strcmp(fileExt,"EXR")){
        std::vector<IMF::Rgba> image;
        for(int i=0;i<w*h;++i){
            image.push_back(IMF::Rgba{ data[i*3] ,data[i*3+1] ,data[i*3+2] ,1});
        }
        WriteOpenEXR(fileName,&image[0],w,h);
    }
}

