#include "imagemap.h"
#include "paramset.h"
#include "logging.h"

ImageTexture<Float, Float> *CreateImageFloatTexture(const Transform &tex2world,
	const TextureParams &tp) {
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
	else if (tp.FindString("mapping") == "cylindrical") {
		Float uscale = tp.FindFloat("uscale", 1);
		Float vscale = tp.FindFloat("vscale", 1);
		Float udelta = tp.FindFloat("udelta", 0);
		Float vdelta = tp.FindFloat("vdelta", 0);
		mapping.reset(new CylindricalMapping2D(Inverse(tex2world), uscale, vscale, udelta, vdelta));
	}
	else if (tp.FindString("mapping") == "planar") {
		Vector3f v1 = tp.FindVector3f("v1", Vector3f(1, 0, 0));
		Vector3f v2 = tp.FindVector3f("v2", Vector3f(0, 1, 0));
		Float uscale = tp.FindFloat("uscale", 1);
		Float vscale = tp.FindFloat("vscale", 1);
		Float udelta = tp.FindFloat("udelta", 0);
		Float vdelta = tp.FindFloat("vdelta", 0);
		mapping.reset(new PlanarMapping2D(v1, v2, uscale, vscale, udelta, vdelta));
	}
	else {
		LError << "2D texture mapping \"" << tp.FindString("mapping") << "\" unknown";
		mapping.reset(new UVMapping2D());
	}
	std::string fileName = tp.FindString("filename");

	Float maxAniso = tp.FindFloat("maxanisotropy", 8.0f);
	bool trilerp = tp.FindBool("trilinear", false);

	Float scale = tp.FindFloat("scale", 1);
	bool gamma = tp.FindBool("gamma", false);
	std::string wrapmodeStr = tp.FindString("wrapmode", "black");
	WrapMode wrapMode;
	if (wrapmodeStr == "repeat") {
		wrapMode = WrapMode::Repeat;
	}
	else if (wrapmodeStr == "clamp") {
		wrapMode = WrapMode::Clamp;
	}
	else if (wrapmodeStr == "black") {
		wrapMode = WrapMode::Black;
	}
	else {
		LWarning << "Invalid WrapMode " << wrapmodeStr << ". Use Black Mode.";
		wrapMode = WrapMode::Black;
	}
	return new ImageTexture<Float,Float>(std::move(mapping), fileName, wrapMode, trilerp, maxAniso, scale, gamma);
}


ImageTexture<RGBSpectrum,Spectrum> *CreateImageSpectrumTexture(const Transform &tex2world,
	const TextureParams &tp){
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
        else if(tp.FindString("mapping") == "cylindrical"){
            Float uscale = tp.FindFloat("uscale", 1);
            Float vscale = tp.FindFloat("vscale", 1);
            Float udelta = tp.FindFloat("udelta", 0);
            Float vdelta = tp.FindFloat("vdelta", 0);
            mapping.reset(new CylindricalMapping2D(Inverse(tex2world),uscale, vscale, udelta, vdelta));
        }
        else if(tp.FindString("mapping") == "planar"){
            Vector3f v1 = tp.FindVector3f("v1", Vector3f(1,0,0));
            Vector3f v2 = tp.FindVector3f("v2", Vector3f(0,1,0));
            Float uscale = tp.FindFloat("uscale", 1);
            Float vscale = tp.FindFloat("vscale", 1);
            Float udelta = tp.FindFloat("udelta", 0);
            Float vdelta = tp.FindFloat("vdelta", 0);
            mapping.reset(new PlanarMapping2D(v1,v2,uscale,vscale,udelta, vdelta));
        }
        else {
            LError<<"2D texture mapping \""<<tp.FindString("mapping")<<"\" unknown";
            mapping.reset(new UVMapping2D());
        }

       std::string fileName=tp.FindString("filename");

       Float maxAniso = tp.FindFloat("maxanisotropy", 8.0f);
       bool trilerp = tp.FindBool("trilinear", false);

       Float scale = tp.FindFloat("scale",1);
       bool gamma = tp.FindBool("gamma",true);
       std::string wrapmodeStr = tp.FindString("wrapmode","black");
       WrapMode wrapMode;
       if(wrapmodeStr=="repeat"){
          wrapMode=WrapMode::Repeat;
       }
       else if(wrapmodeStr=="clamp"){
          wrapMode=WrapMode::Clamp;
       }
       else if(wrapmodeStr=="black"){
          wrapMode=WrapMode::Black;
       }
       else{
           LWarning<<"Invalid WrapMode "<<wrapmodeStr<<". Use Black Mode.";
           wrapMode=WrapMode::Black;
       }
        return new ImageTexture<RGBSpectrum,Spectrum>(std::move(mapping),fileName,wrapMode,trilerp,maxAniso,scale,gamma);
    }