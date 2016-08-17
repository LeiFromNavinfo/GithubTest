#include "Editor.h"
#include "Editor_Data.h"
#include "DataTransfor.h"
#include <stringbuffer.h>
#include <writer.h>
using namespace rapidjson;

namespace Editor
{
	DataFeatureRdLine::DataFeatureRdLine()
	{
	}

	DataFeatureRdLine::~DataFeatureRdLine()
	{
	}

	JSON DataFeatureRdLine::GetSnapshotPart()
	{
		return GetTotalPart();
	}

    JSON DataFeatureRdLine::GetTotalPart()
    {
        Document document;
        document.SetObject();
		Document::AllocatorType& allocator = document.GetAllocator();

		document.AddMember("rowid",GetRowId(),allocator);
		document.AddMember("pid",GetAsInteger(0),allocator);

        EditorGeometry::WkbGeometry* geoWkb = GetAsWkb(1);
        Value each_json_value(kStringType);
        std::string each_str_value = DataTransfor::Wkb2Wkt(geoWkb);
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);  
		document.AddMember("geometry",each_json_value,allocator);

		each_str_value = GetAsString(2);
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
		document.AddMember("display_style",each_json_value,allocator);

        each_str_value = GetAsString(3);
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
		document.AddMember("display_text",each_json_value,allocator);

        each_str_value = GetAsString(4);
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
		document.AddMember("meshid",each_json_value,allocator);

		document.AddMember("kind",GetAsInteger(5),allocator);
		document.AddMember("direct",GetAsInteger(6),allocator);
		document.AddMember("appInfo",GetAsInteger(7),allocator);
        document.AddMember("tollInfo",GetAsInteger(8),allocator);
		document.AddMember("multiDigitized",GetAsInteger(9),allocator);
		document.AddMember("specialTraffic",GetAsInteger(10),allocator);
        document.AddMember("fc",GetAsInteger(11),allocator);
        document.AddMember("laneNum",GetAsInteger(12),allocator);
        document.AddMember("laneLeft",GetAsInteger(13),allocator);
        document.AddMember("laneRight",GetAsInteger(14),allocator);
        document.AddMember("isViaduct",GetAsInteger(15),allocator);
        document.AddMember("paveStatus",GetAsInteger(16),allocator);

        unsigned char* blob;
        unsigned int length;

        blob = this->GetAsBlob(17, length);
        std::string temp_string(reinterpret_cast<char*>(blob),length);
        Document docTmp;
        docTmp.Parse<0>(temp_string.c_str());
        if(!docTmp.Parse<0>(temp_string.c_str()).HasParseError())
        {
            document.AddMember("forms",docTmp,allocator);
        }
        
        blob = this->GetAsBlob(18, length);
        std::string temp_string2(reinterpret_cast<char*>(blob),length);
        docTmp.Parse<0>(temp_string2.c_str());
        if(!docTmp.Parse<0>(temp_string2.c_str()).HasParseError())
        {
            document.AddMember("styleFactors",docTmp,allocator);
        }

        blob = this->GetAsBlob(19, length);
        std::string temp_string3(reinterpret_cast<char*>(blob),length);
        docTmp.Parse<0>(temp_string3.c_str());
        if(!docTmp.Parse<0>(temp_string3.c_str()).HasParseError())
        {
            document.AddMember("speedLimit",docTmp,allocator);
        }

        each_str_value = GetAsString(20);
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
		document.AddMember("op_date",each_json_value,allocator);

        document.AddMember("op_lifecycle",GetAsInteger(21),allocator);

        blob = this->GetAsBlob(22, length);
        std::string temp_string4(reinterpret_cast<char*>(blob),length);
        if(!docTmp.Parse<0>(temp_string4.c_str()).HasParseError())
        {
            document.AddMember("names",docTmp,allocator);
        }

        StringBuffer buffer;
	    Writer<StringBuffer> writer(buffer);  
		document.Accept(writer);  
		std::string result = buffer.GetString();

        JSON json;

        json.SetJsonString(result);

		return json;
    }
}
