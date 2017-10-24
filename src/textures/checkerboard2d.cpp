#include "checkerboard2d.h"
#include "paramset.h"


Checkerboard2DTexture<Float> *CreateCheckerboard2DFloatTexture(const Transform &tex2world,
	const TextureParams &tp) {
	//确认mapping类型
	std::unique_ptr<TextureMapping2D> mapping;
	if (tp.FindString("mapping") == "uv") {
		Float uscale = tp.FindFloat("uscale",1);
		Float vscale = tp.FindFloat("vscale",1);
		Float udelta = tp.FindFloat("udelta", 0);
		Float vdelta = tp.FindFloat("vdelta", 0);
		//记住std::unique_ptr::reset用法!!!
		mapping.reset(new UVMapping2D(uscale, vscale, udelta, vdelta));
	}

	//纹理
	std::shared_ptr<Texture<Float>> tex0 = tp.GetFloatTexture("tex1", 0);
	std::shared_ptr<Texture<Float>> tex1 = tp.GetFloatTexture("tex2", 1);
	
	return new Checkerboard2DTexture<Float>(std::move(mapping),tex0,tex1);
}


Checkerboard2DTexture<Spectrum> *CreateCheckerboard2DSpectrumTexture(const Transform &tex2world,
	const TextureParams &tp) {
	//确认mapping类型
	std::unique_ptr<TextureMapping2D> mapping;
	if (tp.FindString("mapping") == "uv") {
		Float uscale = tp.FindFloat("uscale", 1);
		Float vscale = tp.FindFloat("vscale", 1);
		Float udelta = tp.FindFloat("udelta", 0);
		Float vdelta = tp.FindFloat("vdelta", 0);
		//记住std::unique_ptr::reset用法!!!
		mapping.reset(new UVMapping2D(uscale, vscale, udelta, vdelta));
	}
	else if (tp.FindString("mapping") == "spherical") {
		Float uscale = tp.FindFloat("uscale", 1);
		Float vscale = tp.FindFloat("vscale", 1);
		Float udelta = tp.FindFloat("udelta", 0);
		Float vdelta = tp.FindFloat("vdelta", 0);
		mapping.reset(new SphericalMapping2D(Inverse(tex2world), uscale, vscale, udelta, vdelta));
	}

	//纹理
	std::shared_ptr<Texture<Spectrum>> tex0 = tp.GetSpectrumTexture("tex1", 0);
	std::shared_ptr<Texture<Spectrum>> tex1 = tp.GetSpectrumTexture("tex2", 1);

	return new Checkerboard2DTexture<Spectrum>(std::move(mapping), tex0, tex1);
}