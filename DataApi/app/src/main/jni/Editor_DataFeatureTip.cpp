#include "Editor.h"
#include "Editor_Data.h"
#include "DataTransfor.h"

namespace Editor
{
	DataFeatureTip::DataFeatureTip()
	{
	}

	DataFeatureTip::~DataFeatureTip()
	{
	}

    Model::TipsType DataFeatureTip::GetTipsType()
    {
        return this->m_pType;
    }

    void DataFeatureTip::SetTipsType(Model::TipsType type)
    {
        this->m_pType = type;
    }

    JSON DataFeatureTip::GetTotalPart()
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

        document.AddMember("s_featureKind",GetAsInteger(1),allocator);

        each_str_value = GetAsString(2);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("s_project",each_json_value,allocator);

        document.AddMember("s_sourceCode",GetAsInteger(3),allocator);

        each_str_value = GetAsString(4);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("s_sourceId",each_json_value,allocator);

        each_str_value = GetAsString(5);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("s_sourceType",each_json_value,allocator);

        document.AddMember("s_sourceProvider",GetAsInteger(6),allocator);
        document.AddMember("s_reliability",GetAsInteger(7),allocator);

        EditorGeometry::WkbGeometry* geoWkb = GetAsWkb(8);
        each_str_value = DataTransfor::Wkb2Wkt(geoWkb);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("g_location",each_json_value,allocator);

        each_str_value = GetAsString(9);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("g_guide",each_json_value,allocator);

        document.AddMember("t_lifecycle",GetAsInteger(10),allocator);
        document.AddMember("t_status",GetAsInteger(11),allocator);

        each_str_value = GetAsString(12);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("t_operateDate",each_json_value,allocator);

        document.AddMember("t_handler",GetAsInteger(13),allocator);
        
        unsigned char* blob;
        unsigned int length;

        blob = this->GetAsBlob(14, length);
        std::string temp_string(reinterpret_cast<char*>(blob),length);
        docTmp.Parse<0>(temp_string.c_str());
        document.AddMember("deep",docTmp,allocator);

        each_str_value = GetAsString(15);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("display_style",each_json_value,allocator);

        blob = this->GetAsBlob(16, length);
        std::string temp_string2(reinterpret_cast<char*>(blob),length);
        docTmp.Parse<0>(temp_string2.c_str());
        document.AddMember("attachments",docTmp,allocator);

        document.AddMember("angle",GetAsDouble(17),allocator);
        document.AddMember("t_command",GetAsInteger(18),allocator);
		document.AddMember("t_sync",GetAsInteger(19),allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string result = buffer.GetString();

        JSON json;

        json.SetJsonString(result);

        return json;
	}

    int DataFeatureTip::SetTotalPart(JSON json)
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
		    if(sKey == "s_featureKind")
		    {
			    SetAsInteger(1,(ptr->value).GetInt());
		    }
            if(sKey == "s_project")
		    {
			    SetAsString(2,(ptr->value).GetString());
		    }
		    if(sKey == "s_sourceCode")
		    {
			    SetAsInteger(3,(ptr->value).GetInt());
		    }
		    if(sKey == "s_sourceId")
		    {
			    SetAsString(4,(ptr->value).GetString());
		    }
            if(sKey == "s_sourceType")
		    {
			    SetAsString(5,(ptr->value).GetString());
		    }
            if(sKey == "s_sourceProvider")
		    {
			    SetAsInteger(6,(ptr->value).GetInt());
		    }
            if(sKey == "s_reliability")
		    {
			    SetAsInteger(7,(ptr->value).GetInt());
		    }
            if(sKey == "g_location")
            {
//                std::string wktGeo = (ptr->value).GetString();
//
//                EditorGeometry::WkbGeometry * wkb= DataTransfor::Wkt2Wkb(wktGeo);
//
//                SetAsWkb(8, wkb);
//
//                delete[] wkb;
                
                std::string wktGeo = "";
                if (!(ptr->value).IsString())
                {
                    //后台安装格式不是wkt，是geoJson，统一转为wkt
                    sValue= DataTransfor::GeoJson2Wkt(sValue, Model::Tips::DataSourceFromServer);
                    
                    //app 层是wkt
                    wktGeo = sValue;
                    if(sValue.size() >= 2 && (sValue[0] == '\"' && sValue[sValue.length() - 1] == '\"'))
                    {
                        wktGeo = sValue.substr(1, sValue.size() - 2);
                    }
                }
                else
                {
                    wktGeo = (ptr->value).GetString();
                }
                
                EditorGeometry::WkbGeometry * wkb= DataTransfor::Wkt2Wkb(wktGeo);
                if(wkb)
                {
                    SetAsWkb(8, wkb);
                    
                    delete[] wkb;
                }

            }
            if(sKey == "g_guide")
		    {
                std::string wktGeo = "";
                if (!(ptr->value).IsString())
                {
                    //后台安装格式不是wkt，是geoJson，统一转为wkt
                    sValue= DataTransfor::GeoJson2Wkt(sValue, Model::Tips::DataSourceFromServer);
                    
                    //app 层是wkt
                    wktGeo = sValue;
                    if(sValue.size() >= 2 && (sValue[0] == '\"' && sValue[sValue.length() - 1] == '\"'))
                    {
                        wktGeo = sValue.substr(1, sValue.size() - 2);
                    }
                }
                else
                {
                    wktGeo = (ptr->value).GetString();
                }
                
                SetAsString(9,wktGeo);
		    }
            if(sKey == "t_lifecycle")
		    {
			    SetAsInteger(10,(ptr->value).GetInt());
		    }
            if(sKey == "t_status")
		    {
			    SetAsInteger(11,(ptr->value).GetInt());
		    }
            if(sKey == "t_operateDate")
		    {
			    SetAsString(12,(ptr->value).GetString());
		    }
            if(sKey == "t_handler")
		    {
			    SetAsInteger(13,(ptr->value).GetInt());
		    }
            if(sKey == "deep")
		    {
                const char* deep = sValue.c_str();

                unsigned char* deep_str = (unsigned char*)deep;

                SetAsBlob(14,deep_str, strlen(deep));
		    }
            if(sKey == "display_style")
		    {
			    SetAsString(15,(ptr->value).GetString());
		    }
            if(sKey == "attachments")
		    {
                std::string type;

                if(doc.HasMember("s_sourceType"))
                {
                    type = doc["s_sourceType"].GetString();
                }
                
                if(strlen(type.c_str()) == 1)
                {
                    type = "0"+type;
                }

                std::string attachments_string = GetAttachments(sValue, type);

                const char* attachments = attachments_string.c_str();

                unsigned char* attachments_str = (unsigned char*)attachments;

                SetAsBlob(16,attachments_str, strlen(attachments));
		    }
            if(sKey == "angle")
		    {
			    SetAsDouble(17,(ptr->value).GetDouble());
		    }
            if(sKey == "t_command")
		    {
			    SetAsInteger(18,(ptr->value).GetInt());
		    }
			if(sKey == "t_sync")
			{
				SetAsInteger(19,(ptr->value).GetInt());
			}
        }
        return 0;
	}

    std::string DataFeatureTip::GetAttachments(const std::string& attachment, const std::string& type)
    {
        rapidjson::Document doc;

        std::string attachment_string = "[]";

        doc.Parse<0>(attachment.c_str());

        Document::AllocatorType& allocator = doc.GetAllocator();

        if(doc.HasParseError() || !doc.IsArray())
        {
            return attachment_string;
        }

         for(int i = 0; i< doc.Capacity(); ++i)
         {
             rapidjson::Value& arrayValue = doc[i];
             
             double lon =0.0, lat=0.0;

             if(arrayValue.HasMember("extContent"))
             {
                 rapidjson::Value& extContent = arrayValue["extContent"];

                 if(extContent.IsNull())
                 {
                     continue;
                 }

                 if(extContent.HasMember("longitude"))
                 {
                     lon = extContent["longitude"].GetDouble();
                 }

                 if(extContent.HasMember("latitude"))
                 {
                     lat = extContent["latitude"].GetDouble();
                 }
             }

             if(!arrayValue.HasMember("id"))
             {
                 std::string rowkey =Tools::GetTipsRowkey(type);
                 
                 Value jsonValue(kStringType);

                 jsonValue.SetString(rowkey.c_str(),rowkey.size(),allocator);

                 arrayValue.AddMember("id",jsonValue,allocator);
             }
         }

         StringBuffer buffer;
         Writer<StringBuffer> writer(buffer);  
         doc.Accept(writer);  
         attachment_string = buffer.GetString();

         return attachment_string;
    }

    FEATURE_GEOTYPE DataFeatureTip::GetGeoType(int type)
    {
        switch(type)
        {
        case Model::OrdinaryTurnRestriction:
        case Model::RoadKind:
		case Model::LaneConnexity:
		case Model::RoadDirect:
        case Model::CrossName:
        case Model::HighwayBranch:
        case Model::Gsc:
        case Model::Hitching:
            {
                return GEO_POINT;
            }
        case Model::RoadName:
		case Model::SurveyLine:
            {
                return GEO_LINE;
            }
        case Model::MultiDigitized:
        case Model::UnderConstruction:
        case Model::Bridge:
            {
                return GEO_MIXED;
            }
        case Model::RegionalRoad:
                return GEO_POLYGON;
        default:
            return GEO_POINT;
        }
    }
}
