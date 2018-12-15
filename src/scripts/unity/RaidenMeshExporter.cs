using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEngine;

[ExecuteInEditMode]
[RequireComponent(typeof(MeshFilter))]
public class RaidenMeshExporter : MonoBehaviour
{
    [MenuItem("Raiden/ExportAllAsLuaFile")]
    static void ExportAllAsLua()
    {
       var meshExporters=GameObject.FindObjectsOfType<RaidenMeshExporter>() as RaidenMeshExporter[];
        foreach (var expoter in meshExporters)
        {
            expoter.ExportAsLua();
        }
    }

    public bool normal=true;
    public bool tangent = true;
    public bool uv1 = true;

    private MeshFilter _meshFilter;
    private float[] _vertices;
    private float[] _normals;
    private float[] _tangents;
    private float[] _uvs;
    private List<int[]> _indices;
    // Start is called before the first frame update
    void Start()
    {
        _meshFilter = GetComponent<MeshFilter>();
        _vertices=VectorArrayToFloatArray(_meshFilter.sharedMesh.vertices);
        _normals = VectorArrayToFloatArray(_meshFilter.sharedMesh.normals);
        _uvs = VectorArrayToFloatArray(_meshFilter.sharedMesh.uv);
        _tangents= VectorArrayToFloatArrayWithoutW(_meshFilter.sharedMesh.tangents);
        _indices = new List<int[]>();
        for(int i = 0; i < _meshFilter.sharedMesh.subMeshCount; ++i)
        {
            _indices.Add(_meshFilter.sharedMesh.GetIndices(i));
        }
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    private float[] VectorArrayToFloatArrayWithoutW(Vector4[] array)
    {
        float[] ret = new float[array.Length * 3];
        for (int i = 0; i < array.Length; ++i)
        {
            ret[i * 3 + 0] = array[i].x;
            ret[i * 3 + 1] = array[i].y;
            ret[i * 3 + 2] = array[i].z;
        }
        return ret;
    }

    private float[] VectorArrayToFloatArray(Vector3[] array)
    {
        float[] ret = new float[array.Length * 3];
        for (int i = 0; i < array.Length; ++i)
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


    [ContextMenu("Export as Lua file")]
    void ExportAsLua()
    {
        var dir = "raiden_output";

        if (!Directory.Exists(dir))
        {
            Directory.CreateDirectory(dir);
        }

        if (File.Exists("./" + dir + "/" + _meshFilter.sharedMesh.name + ".lua"))
        {
            Debug.Log(_meshFilter.sharedMesh.name+".lua already exists.");
            Debug.Log("overwrite " + _meshFilter.sharedMesh.name + ".lua.");
        }
        var _out = File.CreateText("./" + dir + "/" + _meshFilter.sharedMesh.name + ".lua");
        _out.WriteLine("local meshs={}");
        _out.WriteLine("meshs.vertices={'point3f',{"+FloatArrayToString(_vertices) +"}}");
        if (normal)
        {
            _out.WriteLine("meshs.normals={'normal3f',{" + FloatArrayToString(_normals) + "}}"); 
        }
        if (tangent)
        {
            _out.WriteLine("meshs.tangents={'vector3f',{" + FloatArrayToString(_tangents) + "}}");
        }
        if (uv1)
        {
            _out.WriteLine("meshs.uv1s={'point2f',{" + FloatArrayToString(_uvs) + "}}");
        }
        _out.WriteLine("meshs.indices={}");
        for(int i = 0; i < _meshFilter.sharedMesh.subMeshCount; ++i)
        {
            _out.WriteLine("meshs.indices["+i+"]={"+IntArrayToString(_indices[i])+"}");
        }
        _out.WriteLine("return meshs");
        _out.Flush();
        _out.Close();
    }


}
