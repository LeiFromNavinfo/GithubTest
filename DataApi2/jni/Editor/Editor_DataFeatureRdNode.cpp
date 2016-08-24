#include "Editor.h"
#include "Editor_Data.h"
#include <stringbuffer.h>
#include <writer.h>
using namespace rapidjson;

namespace Editor
{
	DataFetureRdNode::DataFetureRdNode()
	{
	}

	DataFetureRdNode::~DataFetureRdNode()
	{
	}

	JSON DataFetureRdNode::GetSnapshotPart()
	{
		return GetTotalPart();
	}

    JSON DataFetureRdNode::GetTotalPart()
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

		document.AddMember("isMain",GetAsInteger(5),allocator);

        each_str_value = GetAsString(6);
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
		document.AddMember("op_date",each_json_value,allocator);

        document.AddMember("op_lifecycle",GetAsInteger(7),allocator);

        StringBuffer buffer;
	    Writer<StringBuffer> writer(buffer);  
		document.Accept(writer);  
		std::string result = buffer.GetString();

        JSON json;

        json.SetJsonString(result);

		return json;
    }
}
