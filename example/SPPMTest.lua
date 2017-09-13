
	PixelFilter("box",{xwidth=1,ywidth=1})
	Film("image",{filename=("sppm.png"),xresolution=192*4,yresolution=108*4,cropwindow={type="float[]",value={0.0,1.0,0.0,1.0}}})
	Accelerator("bvh",{splitmethod="sah"});
	--Integrator("path",{})
	--Sampler("halton",{pixelsamples=128})
	Integrator("sppm",{photonsperiteration=1000,imagewritefrequency=4})--imagewritefrequency=8,numiterations=1
	Camera("perspective",{lensradius=0,frameaspectratio=16.0/9.0,focaldistance=2.0,fov=90.0})
	WorldBegin()
		Translate(0.0,0.0,3)
		TransformBegin()
			AreaLightSource("area",{L={type="rgb",value={10.0,9.0,8.0}}})
			Translate(4.0,2.0,0)
			Shape("sphere",{radius=0.5})
			AreaLightSource("",{})
			Translate(-0.0,-3.0,0.0)
			Material("glass",{eta=2.4})
			--Shape("sphere",{radius=1.0})
			AreaLightSource("",{})
		TransformEnd()
	
		
		Material("lambertian",{})
		ReverseOrientation()
		Translate(0.0,-3.0,0.0)
		--for i=0,#shapes do
			--Shape("trianglemesh",{P=shapes[i].vertices,indices=shapes[i].indices})
		--end
	WorldEnd()
