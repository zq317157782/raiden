
/*
 * xmlparser.cpp
 *
 *  Created on: 2018年11月11日
 *      Author: zhuqian
 */
#pragma once

#include "parser.h"
#include "pugixml.hpp"


class XMLParser : public Parser{
private:
    pugi::xml_document _doc;
    const std::string _postFix=".xml";

    bool EndWithXML(const std::string& fileName) const{
        if(fileName.compare(fileName.size()-_postFix.size(),_postFix.size(),_postFix)==0){
            return true;
        }
        return false;
    }

    void PharseChildNodeParamSet(ParamSet& set,const pugi::xml_node& node) const;
public:
     void Init() override;
     void Release() override;
    
     bool IsSupported(const char*  fileName) const override{ 
          assert(fileName!=nullptr);
          return EndWithXML(std::string(fileName));
     }

     //执行XML解析
     void Parse(const char*  fileName) override;
};