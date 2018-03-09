	package.path = './?.lua;' 
	local sponza=require "sponza"
	local bunny=require "bunny"
	local diamond=require "eighthearteightarrow"
	local cube=require "cube"
	local cloud=require "cloud"

	PixelFilter("box",{xwidth=1,ywidth=1})
	Film("image",{filename=("test.png"),xresolution=1920*0.5,yresolution=1080*0.5,cropwindow={type="float[]",value={0.0,1.0,0.0,1.0}}})
	Accelerator("bvh",{splitmethod="sah"});
	Integrator("volpath",{})
	--Integrator("path",{})
	--Integrator("sppm",{})
	--Integrator("depth",{farpanel=100.0})
	--Sampler("halton",{pixelsamples=256})
	Sampler("stratified",{xsamples=8,ysamples=8})
	
	--Camera("environment",{})
	Camera("perspective",{lensradius=0,frameaspectratio=16.0/9.0,focaldistance=2.0,fov=70.0})
	WorldBegin()
	TransformBegin()
		Rotate(-90.0,1.0,0.0,0.0)
		LightSource("infinite",{mapname="skylight-morn.exr",scale=1.0})
	TransformEnd()
	
	TransformBegin()
		 Translate(-1,-1,2)
		 Scale(3,3,3)
		-- MakeNamedMedium("haze",{type="grid",density={type="float[]",value=cloud.data},sigma_a={type="rgb",value={90.0,90.0,90.0}},sigma_s={type="rgb",value={10.0,10.0,10.0}},nx=100,ny=100,nz=40})
		 MakeNamedMedium("haze",{type="grid",density={type="float[]",value=cloud.data},sigma_a={type="rgb",value={40.0,40.0,40.0}},sigma_s={type="rgb",value={10.0,10.0,10.0}},nx=100,ny=100,nz=40})
	TransformEnd()

	Texture("eta1","color","constant",{value={type="rgb",value={1,1.2,0.5}}})
	Texture("eta2","color","constant",{value={type="rgb",value={2.4,2.4,4.0}}})
	

		Translate(0.0,0.0,3)
		
		TransformBegin()
			AreaLightSource("area",{L={type="rgb",value={5.0,5.0,5.0}}})
			Translate(0.0,5.0,0.0)
			Shape("sphere",{radius=1.5})
		TransformEnd()
		 
		--  AreaLightSource("",{})
		--  --渲染除场景外的模型
		-- TransformBegin()
		-- 	--渲染玻璃材质
		-- 	Material("glass",{eta=2.4,roughness=0.0,Kt={type="rgb",value={1.0,1.0,1.0}},Kr={type="rgb",value={1.0,1.0,1.0}}})
		-- 	Translate(0.0,-1,-1.0)
		-- 	Scale(10,10,10)
		-- 	for i=0,#bunny.shapes do
		-- 		Shape("trianglemesh",{P=bunny.shapes[i].vertices,indices=bunny.shapes[i].indices})
		-- 	end
		-- TransformEnd()
		-- TransformBegin()
		-- 	--渲染金属材质
		-- 	Material("metal",{eta={type="texture",value="eta2"},absorb={type="rgb",value={0.9,0.9,0.9}}})
		-- 	Translate(1.5,-1,-1.0)
		-- 	Scale(10,10,10)
		-- 	for i=0,#bunny.shapes do
		-- 		Shape("trianglemesh",{P=bunny.shapes[i].vertices,indices=bunny.shapes[i].indices})
		-- 	end
		-- TransformEnd()
		-- TransformBegin()
		-- 	--渲染玻璃材质
		-- 	Material("mirror",{})
		-- 	Translate(-1.5,-1,-1.0)
		-- 	Scale(10,10,10)
		-- 	for i=0,#bunny.shapes do
		-- 		Shape("trianglemesh",{P=bunny.shapes[i].vertices,indices=bunny.shapes[i].indices})
		-- 	end
		-- TransformEnd()
		-- TransformBegin()
		-- 	--渲染体渲染材质
		-- 	MakeNamedMedium("smoke",{type="homogeneous",sigma_a={type="rgb",value={20.0,20.0,20.0}},sigma_s={type="rgb",value={10.0,10.0,10.0}}})
		--     MediumInterface("smoke","")
		-- 	Material("",{})
		-- 	Translate(-3.0,-1,-1.0)
		-- 	Scale(10,10,10)
		-- 	for i=0,#bunny.shapes do
		-- 		Shape("trianglemesh",{P=bunny.shapes[i].vertices,indices=bunny.shapes[i].indices})
		-- 	end
		-- TransformEnd()
		
		 --渲染场景
		 TransformBegin()
			AreaLightSource("",{})
			Material("matte",{})
			--Material("metal",{eta=1.5})
			Translate(0.0,-1.0,-2.0)
			for i=0,#sponza.shapes do
				Shape("trianglemesh",{P=sponza.shapes[i].vertices,indices=sponza.shapes[i].indices})
			end
		 TransformEnd()
		 MediumInterface("","haze")
	WorldEnd()
