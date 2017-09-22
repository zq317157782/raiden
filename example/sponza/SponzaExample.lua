	package.path = './?.lua;' 
	local sponza=require "sponza"
	
	PixelFilter("box",{xwidth=1,ywidth=1})
	Film("image",{filename=("sponza.png"),xresolution=192*5,yresolution=108*5,cropwindow={type="float[]",value={0.0,1.0,0.0,1.0}}})
	Accelerator("bvh",{splitmethod="sah"});
	Integrator("path",{})
	Sampler("halton",{pixelsamples=128})
	Camera("perspective",{lensradius=0,frameaspectratio=16.0/9.0,focaldistance=2.0,fov=90.0})
	WorldBegin()
		Translate(0.0,0.0,3)
		TransformBegin()
			AreaLightSource("area",{L={type="rgb",value={6.0,6.0,6.0}}})
			Translate(0.0,5.0,0.0)
			Shape("sphere",{radius=1.5})
		TransformEnd()
		
		TransformBegin()
			AreaLightSource("",{})
			Material("metal",{eta={type="rgb",value={9,2,4}},absorb={type="rgb",value={0.99,0.99,0.99}},roughnessX=0.1,roughnessY=0.1})
			Translate(0.0,0.5,0.0)
			Shape("sphere",{radius=1})
		TransformEnd()
		
		TransformBegin()
			AreaLightSource("",{})
			Material("matte",{sigma=90.0})
			--Material("metal",{eta=1.5})
			Translate(0.0,-1.0,-2.0)
			for i=0,#sponza.shapes do
				Shape("trianglemesh",{P=sponza.shapes[i].vertices,indices=sponza.shapes[i].indices})
			end
		TransformEnd()
	WorldEnd()
