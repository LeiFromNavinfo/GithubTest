#include "Editor_ResumeProcessor.h"

namespace Editor
{
    EditAttrs::EditAttrs()
    {
    }
    
    void EditAttrs::Serialize(rapidjson::Writer<StringBuffer>& writer) const
    {
        writer.StartArray();
        
        for (int i=0; i<EditHistories.size(); i++)
        {
            EditHistories[i].Serialize(writer);
        }
        
        writer.EndArray();
    }

    std::string EditAttrs::ToJsonString()
    {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        Serialize(writer);
        std::string sResult = buffer.GetString();
        
        return sResult;
    }
    
	bool EditAttrs::ParseJsonForExtractionPoi(std::string json, std::string& lastMergeDate)
    {
        rapidjson::Document doc;

        doc.Parse<0>(json.c_str());

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        if(doc.HasParseError())
        {
            return false;
        }

        if(!doc.IsArray())
        {
            return false;
        }
        if(doc.Size() > 0)
        {
            for(rapidjson::SizeType i = doc.Size(); i > 0; i--)
            {
                rapidjson::Value& p = doc[i - 1];

                Editor::EditHistory eh;

                eh.ParseJson(p);

                if(eh.sourceName == "Android" && ((eh.operation == LIFECYCLE_UPDATE && eh.contents.size() > 0) ||
					    (eh.operation != LIFECYCLE_UPDATE)))
                {
                	lastMergeDate = eh.mergeDate;

				    break;
                }
            }
        }
        
        return true;
    }

    bool EditAttrs::ParseJson(std::string json)
    {
        rapidjson::Document doc;

        doc.Parse<0>(json.c_str());

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        if(doc.HasParseError())
        {
            return false;
        }

        if(!doc.IsArray())
        {
            return false;
        }

        if(doc.Size() > 0)
        {
            for(rapidjson::SizeType i = 0; i < doc.Size(); i++)
            {
                Editor::EditHistory eh;

                eh.ParseJson(doc[i]);

                EditHistories.push_back(eh);
            }
        }

        return true;
    }

    std::string EditAttrs::AddEditHistory(const std::string json, EditHistory history)
    {
        if(ParseJson(json) == false)
        {
            return "";
        }

        EditHistories.push_back(history);

        return ToJsonString();
    }

    std::string EditAttrs::UpdateEditHistory(const std::string json, int& lifecycle, std::string& mergeDate, std::string& project, std::string user)
    {
        return "";
    }

    bool EditAttrs::GenerateUpdateEditHistory(Editor::EditHistory& eh, std::string& oldJson, const std::string& newJson)
    {
        rapidjson::Document oldDoc, newDoc;
        
        newDoc.Parse<0>(newJson.c_str());
        
        oldDoc.Parse<0>(oldJson.c_str());
        
        if(newDoc.HasParseError() || oldDoc.HasParseError())
        {
            Logger::LogD("EditAttrs::GenerateUpdateEditHistory HasParseError");
            Logger::LogO("EditAttrs::GenerateUpdateEditHistory HasParseError");
            
            return false;
        }
        
        if (!newDoc.HasMember("type"))
        {
            Logger::LogD("doc no has type Member!");
            Logger::LogO("doc no has type Member!");
            return false;
        }
        
        int typeValue = newDoc["type"].GetInt();
        if(typeValue != 0)
        {
            Logger::LogD("update failed! [%d] connot update", typeValue);
            Logger::LogO("update failed! [%d] connot update", typeValue);
            
            return false;
        }
        
        if(!oldDoc.HasMember("lifecycle")||oldDoc["lifecycle"].GetInt() == LIFECYCLE_DELETE)
        {
            Logger::LogD("old lifecycle is [%d], connot update", oldDoc["lifecycle"].GetInt());
            Logger::LogO("old lifecycle is [%d], connot update", oldDoc["lifecycle"].GetInt());
            
            return false;
        }
        
        eh.mergeDate = Tools::GetCurrentDateTime();
        
        eh.operation = LIFECYCLE_UPDATE;
        
        for(rapidjson::Document::MemberIterator ptr = newDoc.MemberBegin(); ptr != newDoc.MemberEnd(); ptr++)
        {
            std::string sKey = (ptr->name).GetString();
            
            rapidjson::Value &valueKey = (ptr->value);
            
            std::string oldText = "";
            
            std::string newText = "";
            
            if(sKey == "edits")
            {
                continue;
            }
            
            if(sKey == "lifecycle")
            {
                if(oldDoc["lifecycle"].GetInt() == LIFECYCLE_ORIGINAL)
                {
                    Editor::EditContent ec(LIFECYCLE_ORIGINAL, LIFECYCLE_UPDATE);
                    
                    eh.contents.push_back(ec);
                }
                
                continue;
            }
            
            if(sKey == "qtStatus")
            {
                if(oldDoc.HasMember("qtStatus") && oldDoc["qtStatus"].GetInt() == 1)
                {
                    Editor::EditContent ec("qtStatus", "1", "2");
                    
                    eh.contents.push_back(ec);
                }
                
                continue;
            }
            
            if(valueKey.IsArray() || valueKey.IsObject())
            {
                if(oldDoc.HasMember(sKey.c_str()) && !oldDoc[sKey.c_str()].IsNull())
                {
                    oldText = GetValueString(oldDoc[sKey.c_str()]);
                }
                newText = GetValueString(valueKey);
            }
            else if(valueKey.IsNumber())
            {
                oldText = Tools::NumberToString<int>(oldDoc[sKey.c_str()].GetInt());
                
                newText = Tools::NumberToString<int>(valueKey.GetInt());
            }
            else if(valueKey.IsString())
            {
                if (oldDoc.HasMember(sKey.c_str()) && (!oldDoc[sKey.c_str()].IsNull()||oldDoc[sKey.c_str()].GetString()!=NULL))
                {
                    oldText = oldDoc[sKey.c_str()].GetString();
                }
                
                newText = valueKey.GetString();
            }
            
            if(oldText != newText)
            {
                Editor::EditContent ec(sKey, oldText, newText);
                
                eh.contents.push_back(ec);
            }
        }  
        
        return true;
    }

    std::string EditAttrs::GetValueString(rapidjson::Value& value)
    {
        rapidjson::StringBuffer buffer;

        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        value.Accept(writer);

        std::string sResult = buffer.GetString();

        return sResult;
    }
}
