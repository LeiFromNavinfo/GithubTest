//
//  Editor_DataFeatureTrackSegment.hpp
//  FMDataApi
//
//  Created by lidejun on 16/5/13.
//  Copyright © 2016年 Kevin Chou. All rights reserved.
//

#ifndef Editor_DataFeatureTrackSegment_hpp
#define Editor_DataFeatureTrackSegment_hpp

#include <stdio.h>
#include "Editor_Data.h"

namespace Editor
{
    class DataFeatureTrackSegment: public DataFeature
    {
    private:
        friend class DataFeatureFactory;
        
        DataFeatureTrackSegment();
        
        virtual ~DataFeatureTrackSegment();
        
    public:
        virtual int			    InnerCheck() {return 1;}
        
        //virtual JSON            GetSnapshotPart() {JSON json; return json;}
        
        virtual JSON            GetSnapshotPart() {return GetTotalPart();}
        
        virtual JSON            GetEditHistoryPart() { JSON json; return json; }
        
        //virtual JSON            GetTotalPart();
        
        //virtual int             SetTotalPart(JSON json);
    };
}


#endif /* Editor_DataFeatureTrackSegment_hpp */
