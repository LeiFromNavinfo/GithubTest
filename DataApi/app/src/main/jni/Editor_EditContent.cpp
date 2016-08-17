#include "Editor_ResumeProcessor.h"

namespace Editor
{
    EditContent::EditContent()
    {
	    validationMethod = 1;
    }

    EditContent::EditContent(int oldLifecycle, int newLifecycle)
    {
        validationMethod = 1;
        
        StringBuffer s;
        Writer<StringBuffer> writer(s);
        writer.StartObject();
        writer.Key("lifecycle");
        writer.Int(oldLifecycle);
        writer.EndObject();
        oldValue = s.GetString();
        
        writer.Reset(s);
        s.Clear();
        writer.Reset(s);
        writer.StartObject();
        writer.Key("lifecycle");
        writer.Int(newLifecycle);
        writer.EndObject();
        newValue = s.GetString();
    }
    
    EditContent::EditContent(std::string fieldName, std::string oldVal, std::string newVal)
    {
        validationMethod = 1;
        
        StringBuffer s;
        Writer<StringBuffer> writer(s);
        writer.StartObject();
        writer.Key(fieldName.c_str());
        writer.String(oldVal.c_str());
        writer.EndObject();
        oldValue = s.GetString();
        
        writer.Reset(s);
        s.Clear();
        writer.Reset(s);
        writer.StartObject();
        writer.Key(fieldName.c_str());
        writer.String(newVal.c_str());
        writer.EndObject();
        newValue = s.GetString();
    }
    
    bool EditContent::Parsejson(rapidjson::Value& json)
    {
        if(json.IsObject())
        {
            if(json.HasMember("newValue"))
            {
                newValue = json["newValue"].GetString();
            }
            
            if(json.HasMember("oldValue"))
            {
                oldValue = json["oldValue"].GetString();
            }
            
            if(json.HasMember("validationMethod"))
            {
                validationMethod = json["validationMethod"].GetInt();
            }
        }
        
        return true;
    }
    
    void EditContent::Serialize(rapidjson::Writer<StringBuffer>& writer) const
    {
        writer.StartObject();
        
        writer.Key("newValue");
        
        writer.String(newValue.c_str());
        
        writer.Key("oldValue");
        
        writer.String(oldValue.c_str());
        
        writer.EndObject();
    }
}