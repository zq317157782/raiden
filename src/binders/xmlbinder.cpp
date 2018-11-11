#include "xmlbinder.h"
#include "raiden.h"
void XMLBinder::Init(){
}

void XMLBinder::Release(){
    
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
                if(strcmp(node.name(),"integrator")==0){
                    LInfo<<"-->integrator node";
                }
        }
    }else{
        LWarning<<"XML file must has a root node \"scene\" but \""<<root<<"\" now";
    }
}