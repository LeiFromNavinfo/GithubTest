//
//  Editor_DataFeatureTrackPoint.hpp
//  FMDataApi
//
//  Created by lidejun on 16/5/13.
//  Copyright © 2016年 Kevin Chou. All rights reserved.
//

#ifndef Editor_DataFeatureTrackPoint_hpp
#define Editor_DataFeatureTrackPoint_hpp

#include <stdio.h>
#include "Editor_Data.h"

namespace Editor
{
    class DataFeatureTrackPoint: public DataFeature
    {
    private:
        friend class DataFeatureFactory;
        
        DataFeatureTrackPoint();
        
        virtual ~DataFeatureTrackPoint();
        
    public:
        virtual int			    InnerCheck() {return 1;}
        
        //virtual JSON            GetSnapshotPart() {JSON json; return json;}
        
        virtual JSON            GetSnapshotPart() {return GetTotalPart();}
        
        virtual JSON            GetEditHistoryPart() { JSON json; return json; }
        
        //virtual JSON            GetTotalPart();
        
        //virtual int             SetTotalPart(JSON json);
    };
}

#endif /* Editor_DataFeatureTrackPoint_hpp */
