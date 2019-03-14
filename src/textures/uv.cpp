#include "uv.h"
#include "paramset.h"
#include "logging.h"
UVTexture *CreateUVTexture(const Transform &tex2world,
	const TextureParams &tp){
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
    
    return new UVTexture(std::move(mapping));
    }