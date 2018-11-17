#include "xmlbinder.h"
#include "raiden.h"
#include "paramset.h"
#include <string>
void XMLBinder::Init()
{
}

void XMLBinder::Release()
{
}

void PushInteger(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    auto v = node.attribute("value").as_int();
    set.AddInt(v_name, std::unique_ptr<int[]>(new int[1]{v}), 1);
    LInfo << "--->integer name:" << v_name << " value:" << v;
}

void PushFloat(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    auto v = node.attribute("value").as_float();
    set.AddFloat(v_name, std::unique_ptr<float[]>(new float[1]{v}), 1);
    LInfo << "--->float name:" << v_name << " value:" << v;
}

void PushBool(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    auto v = node.attribute("value").as_bool();
    set.AddBool(v_name, std::unique_ptr<bool[]>(new bool[1]{v}), 1);
    LInfo << "--->bool name:" << v_name << " value:" << v;
}

void PushRGB(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    float r = node.attribute("r").as_float();
    float g = node.attribute("g").as_float();
    float b = node.attribute("b").as_float();
    set.AddRGBSpectrum(v_name, std::unique_ptr<float[]>(new float[3]{r, g, b}), 3);
    LInfo << "--->rgb name:" << v_name << " value:[" << r << "," << g << "," << b << "]";
}

void PushPoint2f(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    float x = node.attribute("x").as_float();
    float y = node.attribute("y").as_float();
    set.AddPoint2f(v_name, std::unique_ptr<Point2f[]>(new Point2f[1]{Point2f(x, y)}), 1);
    LInfo << "--->point2f name:" << v_name << " value:[" << x << "," << y << "]";
}

void PushPoint3f(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    float x = node.attribute("x").as_float();
    float y = node.attribute("y").as_float();
    float z = node.attribute("z").as_float();
    set.AddPoint3f(v_name, std::unique_ptr<Point3f[]>(new Point3f[1]{Point3f(x, y, z)}), 1);
    LInfo << "--->point3f name:" << v_name << " value:[" << x << "," << y << "," << z << "]";
}

void PushVector2f(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    float x = node.attribute("x").as_float();
    float y = node.attribute("y").as_float();
    set.AddVector2f(v_name, std::unique_ptr<Vector2f[]>(new Vector2f[1]{Vector2f(x, y)}), 1);
    LInfo << "--->vector2f name:" << v_name << " value:[" << x << "," << y << "]";
}

void PushVector3f(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    float x = node.attribute("x").as_float();
    float y = node.attribute("y").as_float();
    float z = node.attribute("z").as_float();
    set.AddVector3f(v_name, std::unique_ptr<Vector3f[]>(new Vector3f[1]{Vector3f(x, y, z)}), 1);
    LInfo << "--->vector3f name:" << v_name << " value:[" << x << "," << y << "," << z << "]";
}

void PushNormal3f(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    float x = node.attribute("x").as_float();
    float y = node.attribute("y").as_float();
    float z = node.attribute("z").as_float();
    set.AddNormal3f(v_name, std::unique_ptr<Normal3f[]>(new Normal3f[1]{Normal3f(x, y, z)}), 1);
    LInfo << "--->normal3f name:" << v_name << " value:[" << x << "," << y << "," << z << "]";
}

void PushIntegetArray(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    const char *v_value = node.child_value();
    char *t_value = (char *)malloc(strlen(v_value));
    memcpy(t_value, v_value, strlen(v_value));
    std::vector<int> splits;
    char *p = strtok(t_value, ",");
    while (p != nullptr)
    {
        splits.push_back(atoi(p));
        p = strtok(nullptr, ",");
    }
    std::unique_ptr<int[]> ints(new int[splits.size()]);
    for (uint32_t i = 0; i < splits.size(); ++i)
    {
        ints[i] = splits[i];
    }
    set.AddInt(v_value, std::move(ints), splits.size());
    LInfo << "--->int[] name:" << v_name << " value:[" << v_value << "]";
}

void PushFloatArray(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    const char *v_value = node.child_value();
    char *t_value = (char *)malloc(strlen(v_value));
    memcpy(t_value, v_value, strlen(v_value));
    std::vector<float> splits;
    char *p = strtok(t_value, ",");
    while (p != nullptr)
    {
        splits.push_back(atof(p));
        p = strtok(nullptr, ",");
    }
    std::unique_ptr<float[]> floats(new float[splits.size()]);
    for (uint32_t i = 0; i < splits.size(); ++i)
    {
        floats[i] = splits[i];
    }
    set.AddFloat(v_value, std::move(floats), splits.size());
    LInfo << "--->float[] name:" << v_name << " value:[" << v_value << "]";
}

void PushTexture(ParamSet &set, const pugi::xml_node &node, const char *name = "name")
{
    const char *v_name = node.attribute(name).as_string();
    set.AddTexture(name, v_name);
    LInfo << "--->texture name:" << v_name;
}

void PushString(ParamSet &set, const pugi::xml_node &node, const char *name = "name"){
    const char *v_name = node.attribute(name).as_string();
    set.AddString(name, std::unique_ptr<std::string[]>(new std::string[1]{v_name}), 1);
    LInfo << "--->string name:" << v_name;
}

void XMLBinder::PharseChildNodeParamSet(ParamSet &set, const pugi::xml_node &root) const
{

    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        const char *name = node.name();
        if (strcmp(name, "integer") == 0)
        {
            //单个整数
            PushInteger(set, node);
        }
        else if (strcmp(name, "float") == 0)
        {
            //单个浮点数
            PushFloat(set, node);
        }
        else if (strcmp(name, "bool") == 0)
        {
            //单个布尔值
            PushBool(set, node);
        }
        else if (strcmp(name, "rgb") == 0)
        {
            PushRGB(set, node);
        }
        else if (strcmp(name, "point2f") == 0)
        {
            PushPoint2f(set, node);
        }
        else if (strcmp(name, "point3f") == 0)
        {
            PushPoint3f(set, node);
        }
        else if (strcmp(name, "vector2f") == 0)
        {
            PushVector2f(set, node);
        }
        else if (strcmp(name, "vector3f") == 0)
        {
            PushVector3f(set, node);
        }
        else if (strcmp(name, "normal3f") == 0)
        {
            PushNormal3f(set, node);
        }
        else if (strcmp(name, "integer_array") == 0)
        {
            PushIntegetArray(set, node);
        }
        else if (strcmp(name, "float_array") == 0)
        {
            PushFloatArray(set, node);
        }
        else if (strcmp(name, "texture") == 0)
        {
            PushTexture(set,node);
        }
        else if (strcmp(name, "string") == 0 )
        {
            //字符串
            PushString(set,node);
        }
    }
}

void XMLBinder::ExecScript(const char *fileName)
{
    pugi::xml_parse_result result = _doc.load_file(fileName);
    LInfo << "Loading XML state:" << result.description();
    auto root = _doc.first_child();
    if (strcmp(root.name(), "scene") == 0)
    {
        //开始解析scene层
        LInfo << "->scene node";
        for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
        {
            const char *name = node.name();
            if (strcmp(name, "begin") == 0)
            {
                LInfo << "-->world begin node";
                raidenWorldBegin();
            }
            else if (strcmp(name, "end") == 0)
            {
                LInfo << "-->world end node";
                raidenWorldEnd();
            }
            else if (strcmp(name, "translate") == 0)
            {
                float dx = node.attribute("x").as_float();
                float dy = node.attribute("y").as_float();
                float dz = node.attribute("z").as_float();
                LInfo << "-->translate node:[" << dx << " ," << dy << " ," << dz << "]";
                raidenTranslate(dx, dy, dz);
            }
            else if (strcmp(name, "rotate") == 0)
            {
                float angle = node.attribute("angle").as_float();
                float dx = node.attribute("x").as_float();
                float dy = node.attribute("y").as_float();
                float dz = node.attribute("z").as_float();
                LInfo << "-->rotate node:[" << angle << "|" << dx << " ," << dy << " ," << dz << "]";
                raidenRotate(angle, dx, dy, dz);
            }
            else if (strcmp(name, "integrator") == 0)
            {
                LInfo << "-->integrator node";
                ParamSet params;
                PushString(params,node,"type");
                auto type=params.FindOneString("type","path");
                params.EraseString("type");
                PharseChildNodeParamSet(params, node);
                raidenIntegrator(type,params);
                
            }
            else if (strcmp(name, "camera") == 0)
            {
                LInfo << "-->camera node";
                ParamSet params;
                PushString(params,node,"type");
                auto type=params.FindOneString("type","pinhole");
                params.EraseString("type");
                PharseChildNodeParamSet(params, node);
                raidenCamera(type, params);
            }
            else if (strcmp(name, "film") == 0)
            {
                LInfo << "-->film node";
                ParamSet params;
                PushString(params,node,"type");
                auto type=params.FindOneString("type","image");
                params.EraseString("type");
                PharseChildNodeParamSet(params, node);
                raidenFilm(type, params);
            }
            else if (strcmp(name, "sampler") == 0)
            {
                LInfo << "-->sampler node";
                ParamSet params;
                PushString(params,node,"type");
                auto type=params.FindOneString("type","stratified");
                params.EraseString("type");
                PharseChildNodeParamSet(params, node);
                raidenSampler(type, params);
            }
        }
    }
    else
    {
        LWarning << "XML file must has a root node \"scene\" but \"" << root << "\" now";
    }
}