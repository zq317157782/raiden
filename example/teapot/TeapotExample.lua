	require "teapot"
	
	PixelFilter("box",{xwidth=1,ywidth=1})
	Film("image",{filename=("sphere.png"),xresolution=192*4,yresolution=108*4,cropwindow={type="float[]",value={0.0,1.0,0.0,1.0}}})
	Accelerator("bvh",{splitmethod="sah"});
	Integrator("path",{})
	Sampler("halton",{pixelsamples=12})
	Camera("perspective",{lensradius=0,frameaspectratio=16.0/9.0,focaldistance=2.0,fov=90.0})
	WorldBegin()
		Translate(0.0,0.0,3)
		TransformBegin()
			AreaLightSource("area",{L={type="rgb",value={10.0,9.0,8.0}}})
			Translate(0.0,5.0,0.0)
			Shape("sphere",{radius=1.5})
		TransformEnd()
		TransformBegin()
			AreaLightSource("",{})
			Material("lambertian",{})
			Translate(0.0,0.0,0.0)
			for i=0,#shapes do
				Shape("trianglemesh",{P=shapes[i].vertices,indices=shapes[i].indices})
			end
		TransformEnd()
	WorldEnd()
