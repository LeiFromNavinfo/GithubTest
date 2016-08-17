#include "Editor.h"
#include "Editor_Data.h"
#include "DataTransfor.h"
#include <stringbuffer.h>
#include <writer.h>
#include "BussinessConstants.h"

using namespace rapidjson;

namespace Editor
{
	DataFeaturePoi::DataFeaturePoi()
	{
	}

	DataFeaturePoi::~DataFeaturePoi()
	{
	}

	JSON DataFeaturePoi::GetSnapshotPart()
	{
        JSON json;
        
        FieldDefines* fd = this->GetFieldDefines();
        
        if(NULL == fd || fd->GetColumnCount() == 0)
        {
            return json;
        }
        
		Document document;
        document.SetObject();
		Document::AllocatorType& allocator = document.GetAllocator();

        Value each_json_value(kStringType);
        std::string each_str_value ;
        
		document.AddMember("rowid",GetRowId(),allocator);
        
        int index = fd->GetColumnIndex("fid");
        if(index != -1)
        {
            each_str_value = GetAsString(index);
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("fid",each_json_value,allocator);
        }
        
        index = fd->GetColumnIndex("pid");
        if(index != -1)
        {
            document.AddMember("pid",GetAsInteger(index),allocator);
        }
        
        index = fd->GetColumnIndex("geometry");
        if(index != -1)
        {
            EditorGeometry::WkbGeometry* geoWkb = GetAsWkb(index);
            each_str_value = DataTransfor::Wkb2Wkt(geoWkb);
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("geometry",each_json_value,allocator);
        }
        
        index = fd->GetColumnIndex("name");
        if(index != -1)
        {
            each_str_value = GetAsString(index);
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("name",each_json_value,allocator);
        }
        
        index = fd->GetColumnIndex("kindCode");
        if(index != -1)
        {
            each_str_value = GetAsString(index);
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("kindCode",each_json_value,allocator);
        }
        
        index = fd->GetColumnIndex("address");
        if(index != -1)
        {
            each_str_value = GetAsString(index);
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("address",each_json_value,allocator);
        }
        
        index = fd->GetColumnIndex("level");
        if(index != -1)
        {
            each_str_value = GetAsString(index);
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("level",each_json_value,allocator);
        }
        
        index = fd->GetColumnIndex("lifecycle");
        if(index != -1)
        {
            document.AddMember("lifecycle",GetAsInteger(index),allocator);
        }

        StringBuffer buffer;
	    Writer<StringBuffer> writer(buffer);  
		document.Accept(writer);  
		std::string result = buffer.GetString();

        json.SetJsonString(result);

		return json;
	}
    
    std::string DataFeaturePoi::ToUploadJsonStr()
    {
        FieldDefines* fd = this->GetFieldDefines();
        
        if(fd==NULL)
        {
            return "";
        }
        
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        
        Value each_json_value(kStringType);
        std::string each_str_value ;
        
        for(int i=0; i< fd->GetColumnCount();i++)
        {
            std::string columnName = fd->GetColumnName(i);
            
            int columntype = fd->GetColumnType(i);
            
            switch (columntype) {
                case FT_INTEGER:
                {
                    if(strcmp(columnName.c_str(),Poi_pid.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_evaluatePlanning.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_qtStatus.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_open24H.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_submitStatus.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_lifecycle.c_str()) ==0)
                    {
                        each_str_value = Tools::NumberToString(GetAsInteger(i));
                        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                        Value key_name(kStringType);

                        key_name.SetString(columnName.c_str(), columnName.size(), allocator);

                        document.AddMember(key_name,each_json_value,allocator);
                    }
                    
                    break;
                }
                case FT_TEXT:
                {
                    if(strcmp(columnName.c_str(),Poi_fid.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_meshid.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_name.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_kindCode.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_vipFlag.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_level.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_address.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_postCode.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_adminCode.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_rawFields.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_sourceCollection.c_str()) ==0)
                    {
                        each_str_value = GetAsString(i);
                        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                        Value key_name(kStringType);

                        key_name.SetString(columnName.c_str(), columnName.size(), allocator);

                        document.AddMember(key_name,each_json_value,allocator);
                    }
                    
                    break;
                }
                case FT_BLOB:
                {
                    if(strcmp(columnName.c_str(),Poi_guide.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_names.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_relateParent.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_relateChildren.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_addresses.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_contacts.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_foodtypes.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_parkings.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_hotel.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_sportsVenues.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_chargingStation.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_chargingPole.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_gasStation.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_indoor.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_attachments.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_brands.c_str()) ==0)
                    {
                        unsigned char* blob = NULL;
                        unsigned int length;
                        
                        blob = this->GetAsBlob(i, length);
                        
                        if(NULL==blob || length==0)
                        {
                            each_str_value = "null";//为空时服务端要求这样传值
                        }
                        else
                        {
                            std::string temp_string(reinterpret_cast<char*>(blob),length);
                            
                            each_str_value = temp_string;
                        }
                        
                        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                        Value key_name(kStringType);

                        key_name.SetString(columnName.c_str(), columnName.size(), allocator);

                        document.AddMember(key_name,each_json_value,allocator);
                    }
                    break;
                }
                case FT_GEOMETRY:
                {
                    EditorGeometry::WkbGeometry* geoWkb = GetAsWkb(i);
                    each_str_value = DataTransfor::Wkb2Wkt(geoWkb);
                    each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                    Value key_name(kStringType);

                    key_name.SetString(columnName.c_str(), columnName.size(), allocator);

                    document.AddMember(key_name,each_json_value,allocator);
                    break;
                }
                default:
                    break;
            }
            
        }
        
        int index = fd->GetColumnIndex(Poi_latestMergeDate);
        if(index != -1)
        {
            each_str_value = GetAsString(index);
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("mergeDate",each_json_value,allocator);
        }
        
        each_str_value = "Android";
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("sourceName",each_json_value,allocator);
        
        each_str_value = "";
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("_id",each_json_value,allocator);
        
        each_str_value = "null";
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("rowkey", each_json_value, allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();
        
        return sResult;
    }
    
    
    std::string DataFeaturePoi::ToUploadChargingJsonStr()
    {
        FieldDefines* fd = this->GetFieldDefines();
        
        if(fd==NULL)
        {
            return "";
        }
        
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        
        Value each_json_value(kStringType);
        std::string each_str_value ;
        
        for(int i=0; i< fd->GetColumnCount();i++)
        {
            std::string columnName = fd->GetColumnName(i);
            
            int columntype = fd->GetColumnType(i);
            
            switch (columntype) {
                case FT_INTEGER:
                {
                    if(strcmp(columnName.c_str(),Poi_pid.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_lifecycle.c_str()) ==0)
                    {
                        each_str_value = Tools::NumberToString(GetAsInteger(i));
                        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                        Value key_name(kStringType);
                        
                        key_name.SetString(columnName.c_str(), columnName.size(), allocator);
                        
                        document.AddMember(key_name,each_json_value,allocator);
                    }
                    
                    break;
                }
                case FT_TEXT:
                {
                    if(strcmp(columnName.c_str(),Poi_fid.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_meshid.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_name.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_kindCode.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_address.c_str()) ==0)
                    {
                        each_str_value = GetAsString(i);
                        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                        Value key_name(kStringType);
                        
                        key_name.SetString(columnName.c_str(), columnName.size(), allocator);
                        
                        document.AddMember(key_name,each_json_value,allocator);
                    }
                    
                    break;
                }
                case FT_BLOB:
                {
                    if(strcmp(columnName.c_str(),Poi_guide.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_relateParent.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_relateChildren.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_contacts.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_chargingStation.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_chargingPole.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Poi_attachments.c_str()) ==0)
                    {
                        unsigned char* blob = NULL;
                        unsigned int length;
                        
                        blob = this->GetAsBlob(i, length);
                        
                        if(NULL==blob || length==0)
                        {
                            each_str_value = "null";//为空时服务端要求这样传值
                        }
                        else
                        {
                            std::string temp_string(reinterpret_cast<char*>(blob),length);
                            
                            each_str_value = temp_string;
                        }
                        
                        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                        Value key_name(kStringType);
                        
                        key_name.SetString(columnName.c_str(), columnName.size(), allocator);
                        
                        document.AddMember(key_name,each_json_value,allocator);
                    }
                    break;
                }
                case FT_GEOMETRY:
                {
                    EditorGeometry::WkbGeometry* geoWkb = GetAsWkb(i);
                    each_str_value = DataTransfor::Wkb2Wkt(geoWkb);
                    each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                    Value key_name(kStringType);
                    
                    key_name.SetString(columnName.c_str(), columnName.size(), allocator);
                    
                    document.AddMember(key_name,each_json_value,allocator);
                    break;
                }
                default:
                    break;
            }
            
        }
        
        int index = fd->GetColumnIndex(Poi_latestMergeDate);
        if(index != -1)
        {
            each_str_value = GetAsString(index);
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("mergeDate",each_json_value,allocator);
        }
        
        each_str_value = "Android";
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("sourceName",each_json_value,allocator);
        
        each_str_value = "";
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("_id",each_json_value,allocator);
        
        each_str_value = "null";
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("rowkey", each_json_value, allocator);
        
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();
        
        return sResult;
    }
}
