#include "Editor.h"
#include "Editor_Data.h"
#include "BussinessConstants.h"

namespace Editor
{
	DataFeatureInfor::DataFeatureInfor()
	{
	}

	DataFeatureInfor::~DataFeatureInfor()
	{
	}
    std::string DataFeatureInfor::ToUploadJsonStr()
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
        std::string each_str_value;

        for(int i=0; i< fd->GetColumnCount();i++)
        {
            std::string columnName = fd->GetColumnName(i);
            
            int columntype = fd->GetColumnType(i);
            
            switch (columntype) {
                case FT_INTEGER:
                {
                    if(strcmp(columnName.c_str(),Info_bfeatureKind.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_bsourceCode.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_breliability.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_isAdopted.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_cfeatureKind.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_cpid.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_userId.c_str()) ==0                 
                       )
                    {
                        Value key_name(kStringType);

                        key_name.SetString(columnName.c_str(), columnName.size(), allocator);

                        document.AddMember(key_name,GetAsInteger(i),allocator);
                    }
                    
                    break;
                }
                case FT_TEXT:
                {
                    if(strcmp(columnName.c_str(),Info_id.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_location.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_hprojectId.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_denyReason.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_cfid.c_str()) ==0
                        )
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
                    if(strcmp(columnName.c_str(),Info_f_array.c_str()) ==0)
                    {
                        unsigned char* blob = NULL;
                        unsigned int length;
                        
                        blob = this->GetAsBlob(i, length);
                        
                        if(NULL==blob || length==0)
                        {
                            each_str_value = "null";
                        }
                        else
                        {
                            std::string temp_string(reinterpret_cast<char*>(blob),length);

                            Document docTmp;
                                
                            docTmp.Parse<0>(temp_string.c_str());

                            if(docTmp.HasParseError())
                            {
                                docTmp.SetNull();

                                Logger::LogO("generate json error!!!");
                            }

                            document.AddMember("f_array", docTmp,allocator);
                        }
                    }
                    break;
                }
                default:
                    break;
            }       
        }

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();
        
        return sResult;
    }
    
    /**
     *  道路情报组装json
     *
     *  @return string
     */
    std::string DataFeatureInfor::ToUploadRoadJsonStr()
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
        std::string each_str_value;
        
        for(int i=0; i< fd->GetColumnCount();i++)
        {
            std::string columnName = fd->GetColumnName(i);
            
            int columntype = fd->GetColumnType(i);
            
            switch (columntype) {
                case FT_INTEGER:
                {
                    if(strcmp(columnName.c_str(),Info_bfeatureKind.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_bsourceCode.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_breliability.c_str()) ==0 
                       )
                    {
                        Value key_name(kStringType);
                        
                        key_name.SetString(columnName.c_str(), columnName.size(), allocator);
                        
                        document.AddMember(key_name,GetAsInteger(i),allocator);
                    }
                    
                    break;
                }
                case FT_TEXT:
                {
                    if(strcmp(columnName.c_str(),Info_id.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_location.c_str()) ==0 ||
                       strcmp(columnName.c_str(),Info_hprojectId.c_str()) ==0
                       )
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
                    if(strcmp(columnName.c_str(),Info_f_array.c_str()) ==0)
                    {
                        unsigned char* blob = NULL;
                        unsigned int length;
                        
                        blob = this->GetAsBlob(i, length);
                        
                        if(NULL==blob || length==0)
                        {
                            each_str_value = "null";
                        }
                        else
                        {
                            std::string temp_string(reinterpret_cast<char*>(blob),length);
                            
                            Document docTmp;
                            
                            docTmp.Parse<0>(temp_string.c_str());
                            
                            if(docTmp.HasParseError())
                            {
                                docTmp.SetNull();
                                
                                Logger::LogO("generate json error!!!");
                            }
                            
                            document.AddMember("f_array", docTmp,allocator);
                        }
                    }
                    break;
                }
                default:
                    break;
            }       
        }
        
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();
        
        return sResult;
    }

    
}
