using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEngine;

[ExecuteInEditMode]
public class raiden : MonoBehaviour {
  
    public enum SceneFileFormat{Lua};
    [Header("Scene")]
    public string SceneFileName = "raiden";
    public SceneFileFormat SceneFormat= SceneFileFormat.Lua;
    [Header("OutputImage")]
    public string OutputImageName = "raiden";
    public enum OutputImageFormat { PNG,EXR}
    public OutputImageFormat ImageFormat = OutputImageFormat.PNG;
    public float ImageHeight=1080;
    
    // Use this for initialization
    void Start () {
       // Export();
    }
	
	// Update is called once per frame
	void Update () {
		
	}

    [ContextMenu("ExportSceneFile")]
    private void Export()
    {
        Camera  camera = GameObject.FindObjectOfType<Camera>() as Camera;
        Light[] lights = GameObject.FindObjectsOfType<Light>() as Light[];
        MeshFilter[] primatives = GameObject.FindObjectsOfType<MeshFilter>() as MeshFilter[];
        WriteLuaFile(camera, lights, primatives);
    }

    struct MeshData
    {
        Vector3[] vertices;
        Vector3[] normal;
        Vector4[] tangent;
        
    }

    private float[] VectorArrayToFloatArray(Vector3[] array)
    {
        float[] ret = new float[array.Length * 3];
        for(int i=0;i< array.Length; ++i)
        {
            ret[i * 3 + 0] = array[i].x;
            ret[i * 3 + 1] = array[i].y;
            ret[i * 3 + 2] = array[i].z;
        }
        return ret;

    }
    private float[] VectorArrayToFloatArray(Vector2[] array)
    {
        float[] ret = new float[array.Length * 2];
        for (int i = 0; i < array.Length; ++i)
        {
            ret[i * 2 + 0] = array[i].x;
            ret[i * 2 + 1] = array[i].y;
        }
        return ret;

    }


    private string FloatArrayToString(float[] array)
    {
        string array_s = "";
        for (int j = 0; j < array.Length; ++j)
        {
            array_s += array[j].ToString();
            if (j != (array.Length - 1))
            {
                array_s += ",";
            }
        }
        return array_s;
    }

    private string IntArrayToString(int[] array)
    {
        string array_s = "";
        for (int j = 0; j < array.Length; ++j)
        {
            array_s += array[j].ToString();
            if (j != (array.Length - 1))
            {
                array_s += ",";
            }
        }
        return array_s;
    }

    private void WriteLuaFile(Camera camera,Light[] lights, MeshFilter[] primatives)
    {

        List<string> namedTextures = new List<string>();

        var dir = "raiden_output";

        if (!Directory.Exists(dir))
        {
            Directory.CreateDirectory(dir);
        }
       
       
        if (File.Exists("./"+ dir+"/" + SceneFileName +".lua"))
        {
            Debug.Log(SceneFileName + ".lua already exists.");
            Debug.Log("overwrite "+SceneFileName + ".lua.");
        }

        var sr = File.CreateText("./" + dir + "/" + SceneFileName + ".lua");


        sr.WriteLine("local meshs={}");
        for (int i = 0; i < primatives.Length; ++i)
        {
            sr.WriteLine($"meshs[{i}]={{}}");
            var vertices=VectorArrayToFloatArray(primatives[i].sharedMesh.vertices);
            sr.WriteLine($"meshs[{i}].vertices={{ type='point3f',value={{{FloatArrayToString(vertices)}}} }}");

            var normals = VectorArrayToFloatArray(primatives[i].sharedMesh.normals);
            sr.WriteLine($"meshs[{i}].normals={{ type='normal3f',value={{{FloatArrayToString(normals)}}} }}");

            var uvs = VectorArrayToFloatArray(primatives[i].sharedMesh.uv);
            sr.WriteLine($"meshs[{i}].uvs={{ type='point2f',value={{{FloatArrayToString(uvs)}}} }}");

            sr.WriteLine($"meshs[{i}].indices={{}}");
            for (int j=0;j< primatives[i].sharedMesh.subMeshCount; ++j)
            {
                var indices=primatives[i].sharedMesh.GetIndices(j);
                sr.WriteLine($"meshs[{i}].indices[{j}]={{ type='int[]',value={{{IntArrayToString(indices)}}} }}");
            }
        }

        sr.WriteLine("require 'raiden'");
        sr.WriteLine("--#############################defalut##################################");
        //加速结构
        sr.WriteLine("Accelerator('bvh',{splitmethod='sah'})");
        //积分器
        sr.WriteLine("Integrator('path',{ depth = 5})");
        //采样器
        sr.WriteLine($"Sampler('stratified',{{ xsamples = 1,ysamples = 1}})");
        //PixelFilter
        sr.WriteLine("PixelFilter(\"box\",{xwidth=1.0,ywidth=1.0})");
        sr.WriteLine("--#####################################################################");
        sr.WriteLine("--****************************form Unity*************************************");
        //Film
        sr.WriteLine($"Film('image',{{filename=(\"{OutputImageName + ".png"}\"),xresolution={Screen.currentResolution.width},yresolution={Screen.currentResolution.height},cropwindow={{type=\"float[]\",value={{0.0,1.0,0.0,1.0}}}}}})");

        var camera_pos=-camera.transform.position;
        float camera_rotateAngle;
        Vector3 camera_rotateAxis;
        camera.transform.rotation.ToAngleAxis(out camera_rotateAngle, out camera_rotateAxis);
        sr.WriteLine($"Translate({camera_pos.x},{camera_pos.y},{camera_pos.z})");
        sr.WriteLine($"Rotate({-camera_rotateAngle},{camera_rotateAxis.x},{camera_rotateAxis.y},{camera_rotateAxis.z})");
        sr.WriteLine($"Camera('perspective',{{ lensradius = 0,frameaspectratio = {camera.aspect},focaldistance = 2.0,fov = {camera.fieldOfView.ToString("0.0000")}}})");

        sr.WriteLine("WorldBegin()");
        //光源
        for(int i=0;i< lights.Length; ++i)
        {
            var light = lights[i];
            var pos=light.transform.position;
            float rotateAngle;
            Vector3 rotateAxis;
            light.transform.rotation.ToAngleAxis( out rotateAngle, out rotateAxis);
            var scale=light.transform.localScale;
            var to= pos+light.transform.forward;

            sr.WriteLine("AttributeBegin()");
            //sr.WriteLine($"Translate({pos.x},{pos.y},{pos.z})");
            //sr.WriteLine($"Rotate({rotateAngle},{rotateAxis.x},{rotateAxis.y},{rotateAxis.z})");
            //sr.WriteLine($"Scale({scale.x},{scale.y},{scale.z})");
            if (light.type == LightType.Directional)
            {
                sr.WriteLine($"LightSource(\"distant\",{{  L=rgb({light.color.r},{light.color.g},{light.color.b}),scale=rgb({light.intensity},{light.intensity},{light.intensity}),from=point3f({pos.x},{pos.y},{pos.z}),to=point3f({to.x},{to.y},{to.z}) }})");
            }else if (light.type == LightType.Point)
            {
                sr.WriteLine($"LightSource(\"point\",{{  I=rgb({light.color.r},{light.color.g},{light.color.b}),scale=rgb({light.intensity},{light.intensity},{light.intensity}),from=point3f({pos.x},{pos.y},{pos.z}) }})");
            }
            sr.WriteLine("AttributeEnd()");
        }

        //天空
        var texture=RenderSettings.skybox.GetTexture("_MainTex");
        Debug.Log(AssetDatabase.GetAssetPath(texture));
        if(!File.Exists("./" + dir + "/" + texture.name + ".exr"))
        {
            FileUtil.CopyFileOrDirectory(AssetDatabase.GetAssetPath(texture), "./" + dir + "/" + texture.name + ".exr");
        }
        
        sr.WriteLine("AttributeBegin()");
        sr.WriteLine($"LightSource('infinite',{{mapname='{texture.name}.exr',scale=1.0}})");
        sr.WriteLine("AttributeEnd()");

        /*TransformBegin()
		Rotate(-90.0,1.0,0.0,0.0)
		LightSource("infinite",{mapname="skylight-day.exr",scale=1.0})
	TransformEnd()*/

        //shape
        for (int i=0;i< primatives.Length; ++i)
        {
            var shape = primatives[i];
            var pos = shape.transform.position;
            float rotateAngle;
            Vector3 rotateAxis;
            shape.transform.rotation.ToAngleAxis(out rotateAngle, out rotateAxis);
            var scale = shape.transform.localScale;

            var mesh = shape.sharedMesh;


            var material = shape.GetComponent<Renderer>().sharedMaterial;

            var albedo = material.GetTexture("_MainTex");
            if (albedo != null)
            {

                var path = AssetDatabase.GetAssetPath(albedo);
                Debug.Log(path);
                var type=path.Substring(path.Length - 4);
                if (!File.Exists("./" + dir + "/" + albedo.name + type))
                {
                    FileUtil.CopyFileOrDirectory(AssetDatabase.GetAssetPath(albedo), "./" + dir + "/" + albedo.name + type);
                }
                sr.WriteLine($"Texture('{albedo.name}', 'color', 'imagemap',{{ mapping = 'uv',filename = \"{ albedo.name + type}\",trilinear = true}})");
               
            }

            var metallicGlossMap = material.GetTexture("_MetallicGlossMap");
            if (metallicGlossMap != null)
            {
                var path = AssetDatabase.GetAssetPath(metallicGlossMap);
                Debug.Log(path);
                var type = path.Substring(path.Length - 4);
                if (!File.Exists("./" + dir + "/" + metallicGlossMap.name + type))
                {
                    FileUtil.CopyFileOrDirectory(AssetDatabase.GetAssetPath(metallicGlossMap), "./" + dir + "/" + metallicGlossMap.name + type);
                }
                sr.WriteLine($"Texture('{metallicGlossMap.name}', 'float', 'imagemap',{{ mapping = 'uv',filename = \"{ metallicGlossMap.name + type}\",trilinear = true}})");

            }

            //var to = pos + shape.transform.forward;
            sr.WriteLine("AttributeBegin()");
            // 
            if (albedo != null)
            {
                sr.WriteLine($"Material('unity',{{ albedo ={{ type = 'texture',value = '{albedo.name}'}},metallic ={{ type = 'texture',value = '{metallicGlossMap.name}'}} }})");
            }
            else
            {
                sr.WriteLine("Material('unity',{})");
            }
            sr.WriteLine($"Translate({pos.x},{pos.y},{pos.z})");
            sr.WriteLine($"Rotate({rotateAngle},{rotateAxis.x},{rotateAxis.y},{rotateAxis.z})");
            sr.WriteLine($"Scale({scale.x},{scale.y},{scale.z})");
            for(int j=0; j<mesh.subMeshCount; ++j)
            {
               
                sr.WriteLine($"Shape(\"trianglemesh\",{{P=meshs[{i}].vertices,N=meshs[{i}].normals,indices=meshs[{i}].indices[{j}],uv=meshs[{i}].uvs}})");
            }
            sr.WriteLine("AttributeEnd()");
        }

        sr.WriteLine("WorldEnd()");

        sr.WriteLine("--***************************************************************************");
        sr.Close();
    }
    
}
