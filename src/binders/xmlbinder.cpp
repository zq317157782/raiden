#include "xmlbinder.h"
#include "raiden.h"
#include "paramset.h"
void XMLBinder::Init(){
}

void XMLBinder::Release(){
    
}

ParamSet XMLBinder::PharseParamSet(const pugi::xml_node& root) const{
    ParamSet set;
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {       
        const char* name=node.name();
       
        if(strcmp(name,"integer")==0){
            //单个整数
            const char* v_name=node.attribute("name").as_string();
            auto v=node.attribute("value").as_int();
            set.AddInt(v_name, std::unique_ptr<int[]>(new int[1] {v}),1);
            LInfo<<"--->integer name:"<<v_name<<" value:"<<v;
        }
        else if(strcmp(name,"float")==0){
            //单个浮点数
            const char* v_name=node.attribute("name").as_string();
            auto v=node.attribute("value").as_float();
            set.AddFloat(v_name, std::unique_ptr<float[]>(new float[1] {v}),1);
            LInfo<<"--->float name:"<<v_name<<" value:"<<v;
        }
        else if(strcmp(name,"bool")==0){
            //单个布尔值
            const char* v_name=node.attribute("name").as_string();
            auto v=node.attribute("value").as_bool();
            set.AddBool(v_name, std::unique_ptr<bool[]>(new bool[1] {v}),1);
            LInfo<<"--->bool name:"<<v_name<<" value:"<<v;
        }
        else if(strcmp(name,"string")==0){
            //字符串
        }
    }
    return set;
}

void XMLBinder::ExecScript(const char*  fileName){
    pugi::xml_parse_result result=_doc.load_file(fileName);
    LInfo<<"Loading XML state:"<<result.description();
    auto  root=_doc.first_child();
    if(strcmp(root.name(),"scene")==0){
        //开始解析scene层
        LInfo<<"->scene node";
        for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
        {       
                const char* name=node.name();
                if(strcmp(name,"integrator")==0){
                    LInfo<<"-->integrator node";
                    PharseParamSet(node);
                    //raidenIntegrator(name,);
                }
        }
    }else{
        LWarning<<"XML file must has a root node \"scene\" but \""<<root<<"\" now";
    }
}