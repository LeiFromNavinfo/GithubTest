#include "Editor_ResumeProcessor.h"

namespace Editor
{
    EditOperator::EditOperator()
    {
        role = 0;
        
        user = 0;
    }
    
    bool EditOperator::ParseJson(rapidjson::Value& json)
    {
        if(json.IsObject())
        {
            if(json.HasMember("role") && json["role"].IsInt())
            {
                role = json["role"].GetInt();
            }
            
            if(json.HasMember("user") && json["user"].IsInt())
            {
                user = json["user"].GetInt();
            }
        }
        
        return true;
    }
    
    void EditOperator::Serialize(rapidjson::Writer<StringBuffer>& writer) const
    {
        writer.StartObject();
        
        writer.Key("user");
        writer.Int(user);
        
        writer.Key("role");
        writer.Int(role);
        
        writer.EndObject();
    }
}
