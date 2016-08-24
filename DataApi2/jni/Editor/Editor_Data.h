#ifndef FM_SDK_Editor_Data_h
#define FM_SDK_Editor_Data_h

#include <string>
#include "Editor.h"
#include <document.h>
#include "model.h"
using namespace rapidjson;

namespace Editor 
{   
    class EDITOR_API DataFeaturePoi : DataFeature
    {
    private:
        friend class DataFeatureFactory;

        DataFeaturePoi();

        virtual ~DataFeaturePoi();

    public:
        virtual int			    InnerCheck() {return 1;}

        virtual JSON            GetSnapshotPart();

        virtual JSON            GetEditHistoryPart() { JSON json; return json; }

        //virtual JSON            GetTotalPart();

        //virtual int             SetTotalPart(JSON json);
        
        std::string             ToUploadJsonStr();
    };

    class DataFeatureInfor : DataFeature
    {
    private:
        friend class DataFeatureFactory;

        DataFeatureInfor();

        ~DataFeatureInfor();

    public:
        virtual int			    InnerCheck() {return 1;}

        virtual JSON            GetEditHistoryPart() { JSON json; return json;}
        
        virtual JSON            GetSnapshotPart() {JSON json; return json;}
        
        //virtual JSON            GetTotalPart();
        
        //virtual int             SetTotalPart(JSON json);

        std::string             ToUploadJsonStr();
    };

    class DataFeatureTip : DataFeature
    {
    private:
        friend class DataFeatureFactory;

        DataFeatureTip();

        ~DataFeatureTip();

        Model::TipsType m_pType;

        std::string         GetAttachments(const std::string& attachment, const std::string& type);

    public:
        virtual int			InnerCheck() {return 1;}

        std::string         GetBasicPart();

        void                SetBasicPart(std::string part);

        std::string         GetRestrictionPart();

        void		        SetRestrictionPart(std::string part);

        virtual JSON        GetSnapshotPart() {JSON json; return json;}

        virtual JSON        GetEditHistoryPart() { JSON json; return json; }

        Model::TipsType     GetTipsType();

        void                SetTipsType(Model::TipsType type);

        virtual JSON        GetTotalPart();

        virtual int         SetTotalPart(JSON json);

        FEATURE_GEOTYPE     GetGeoType(int type);

        //...other part
    };

    class DataFeatureTipsGeo : DataFeature
    {
    private:
        friend class DataFeatureFactory;

        DataFeatureTipsGeo()
        {
            
        }

        ~DataFeatureTipsGeo()
        {
            
        }

    public:
        virtual int			InnerCheck() {return 1;}

        virtual JSON        GetSnapshotPart() {JSON json; return json;}

        virtual JSON        GetEditHistoryPart() { JSON json; return json; }

        virtual JSON        GetTotalPart(){JSON json; return json;}

        virtual int         SetTotalPart(JSON json){return 1;}
    };

    class DataFeatureTipsGeoComponent : DataFeature
    {
    private:
        friend class DataFeatureFactory;

        DataFeatureTipsGeoComponent()
        {
            
        }

        ~DataFeatureTipsGeoComponent()
        {
            
        }

    public:

        virtual int			InnerCheck() {return 1;}

        virtual JSON        GetSnapshotPart() {JSON json; return json;}

        virtual JSON        GetEditHistoryPart() { JSON json; return json; }

        virtual JSON        GetTotalPart(){JSON json; return json;}

        virtual int         SetTotalPart(JSON json){return 1;}
    };

    class DataFeatureRdLine : DataFeature
    {
    private:
        friend class DataFeatureFactory;

        DataFeatureRdLine();

        ~DataFeatureRdLine();

    public:
        virtual int			InnerCheck() {return 1;}

        virtual JSON        GetSnapshotPart();

        virtual JSON        GetEditHistoryPart() { JSON json; return json; }

        virtual JSON        GetTotalPart();

        virtual int         SetTotalPart(JSON json) {return 0;}
    };
    
    ///feature GpsLine
    class DataFeatureGPSLine : DataFeature
    {
    private:
        friend class DataFeatureFactory;
        
        DataFeatureGPSLine();
        
        ~DataFeatureGPSLine();
        
    public:
        virtual int			InnerCheck() {return 1;}
        
        virtual JSON        GetSnapshotPart() {JSON json; return json;}
        
        virtual JSON        GetEditHistoryPart() { JSON json; return json; }
        
        virtual JSON        GetTotalPart();
        
        virtual int         SetTotalPart(JSON json);
        
        //...other part

    };

    class DataFetureRdNode : DataFeature
    {
    private:
        friend class DataFeatureFactory;

        DataFetureRdNode();

        ~DataFetureRdNode();

    public:

        virtual int			InnerCheck() {return 1;}

        virtual JSON        GetSnapshotPart();

        virtual JSON        GetEditHistoryPart() { JSON json; return json; }

        virtual JSON        GetTotalPart();

        virtual int         SetTotalPart(JSON json){return 1;}
    };
    
    class EDITOR_API DataFeatureProjectUser : DataFeature
    {
    private:
        friend class DataFeatureFactory;
        
        DataFeatureProjectUser(){}
        
        virtual ~DataFeatureProjectUser(){}
        
    public:
        virtual int			    InnerCheck() {return 1;}
        
        virtual JSON            GetEditHistoryPart() { JSON json; return json; }
    };
    
    class EDITOR_API DataFeatureProjectInfo : DataFeature
    {
    private:
        friend class DataFeatureFactory;
        
        DataFeatureProjectInfo(){}
        
        virtual ~DataFeatureProjectInfo(){}
        
    public:
        virtual int			    InnerCheck() {return 1;}
        
        virtual JSON            GetEditHistoryPart() { JSON json; return json; }
    };
    
    class EDITOR_API DataFeatureTaskInfo : DataFeature
    {
    private:
        friend class DataFeatureFactory;
        
        DataFeatureTaskInfo(){}
        
        virtual ~DataFeatureTaskInfo(){}
        
    public:
        virtual int			    InnerCheck() {return 1;}
        
        virtual JSON            GetEditHistoryPart() { JSON json; return json; }
    };
}

#endif
