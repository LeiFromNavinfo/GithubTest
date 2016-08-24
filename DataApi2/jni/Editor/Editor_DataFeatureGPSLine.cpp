//
//  DataFeatureGPSLine.cpp
//  EditorSDK
//
//  Created by lidejun on 15/11/4.
//  Copyright (c) 2015骞� navinfo. All rights reserved.
//

#include <stdio.h>
#include "Editor.h"
#include "Editor_Data.h"
#include "DataTransfor.h"
#include "Logger.h"

namespace Editor
{
    DataFeatureGPSLine::DataFeatureGPSLine()
    {
    }
    
    DataFeatureGPSLine::~DataFeatureGPSLine()
    {
    }
    
    JSON DataFeatureGPSLine::GetTotalPart()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        
        Value each_json_value(kStringType);
        std::string each_str_value ;
        
        document.AddMember("rowid",GetRowId(),allocator);
        
        each_str_value = GetAsString(0);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("rowkey",each_json_value,allocator);
        
        EditorGeometry::WkbGeometry* geoWkb = GetAsWkb(1);
        each_str_value = DataTransfor::Wkb2Wkt(geoWkb);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("geometry",each_json_value,allocator);
        
        
        each_str_value = GetAsString(2);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("name",each_json_value,allocator);
        
        document.AddMember("kind",GetAsInteger(3),allocator);
        document.AddMember("source",GetAsInteger(4),allocator);
       
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string result = buffer.GetString();
        
        JSON json;
        
        json.SetJsonString(result);
        
        return json;
    }
    
    int DataFeatureGPSLine::SetTotalPart(JSON json)
    {
        rapidjson::Document doc;
        
        doc.Parse<0>(json.GetJsonString().c_str());
        
        if(doc.HasParseError())
        {
            return -1;
        }
        
        for(rapidjson::Document::MemberIterator ptr = doc.MemberBegin(); ptr != doc.MemberEnd(); ++ptr)
        {
            std::string  sKey = (ptr->name).GetString();
            
            rapidjson::Value &valueKey = (ptr->value);
            
            std::string sValue = "";
            
            if((ptr->value).IsNull())
            {
                continue;
            }
            if(!(ptr->value).IsString() || !(ptr->value).IsNumber())
            {
                rapidjson::StringBuffer buffer;
                
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                
                valueKey.Accept(writer);
                
                sValue = buffer.GetString();
            }
            if(sKey == "rowid")
            {
                SetRowId((ptr->value).GetInt());
            }
            if(sKey == "rowkey")
            {
                SetAsString(0,(ptr->value).GetString());
            }
            if(sKey == "geometry")
            {
                std::string wktGeo = (ptr->value).GetString();
                //Logger::LogD("//////-----------------\\\\\\wktGeo:%s",wktGeo.c_str());
                
                EditorGeometry::WkbGeometry * wkb= DataTransfor::Wkt2Wkb(wktGeo);
                
                int iflag = SetAsWkb(1, wkb);
                //Logger::LogD("///////-------------------\\\\\\iflag of SetAsWkb:%d",iflag);
                
                delete[] wkb;
            }
            if(sKey == "name")
            {
                SetAsString(2,(ptr->value).GetString());
            }
            if(sKey == "kind")
            {
                SetAsInteger(3,(ptr->value).GetInt());
            }
            if(sKey == "source")
            {
                SetAsInteger(4,(ptr->value).GetInt());
            }
            
        }
        return 0;
    }
}

