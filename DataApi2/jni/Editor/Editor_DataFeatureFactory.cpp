#include "Editor.h"
#include "Editor_Data.h"
#include "Editor_DataFeatureTrackPoint.h"
#include "Editor_DataFeatureTrackSegment.h"

namespace Editor
{	
	DataFeatureFactory::DataFeatureFactory()
	{
	}

	DataFeatureFactory::~DataFeatureFactory()
	{
	}

	const DataFeatureFactory* DataFeatureFactory::getDefaultInstance()
	{
		static DataFeatureFactory* defInstance = new DataFeatureFactory();

		return defInstance;
	}

	DataFeature* DataFeatureFactory::CreateFeature(DataLayer* layer) const
	{
		int layerType = layer->GetDataLayerType();

        DataFeature* feature = NULL;

		switch (layerType)
		{
		case DATALAYER_POI:
            {
                feature = new DataFeaturePoi();
                break;
            }
		case DATALAYER_RDLINE:
            {
                feature = new DataFeatureRdLine();
                break;
            }
        case DATALAYER_TIPS:
            {
                feature = new DataFeatureTip();
                break;
            }
        case DATALAYER_TIPSPOINT:
        case DATALAYER_TIPSLINE:
        case DATALAYER_TIPSMULTILINE:
        case DATALAYER_GPSLINE:
        case DATALAYER_TIPSPOLYGON:
            {
                feature = new DataFeatureTipsGeo();
                break;
            }
        case DATALAYER_TIPSGEOCOMPONENT:
            {
                feature = new DataFeatureTipsGeoComponent();
                break;
            }
        case DATALAYER_RDNODE:
            {
                feature = new DataFetureRdNode();
                break;
            }
        case DATALAYER_INFOR:
            {
                feature = new DataFeatureInfor();
                break;
            }
        case DATALAYER_TRACKPOINT:
            {
                feature = new DataFeatureTrackPoint();
                break;
            }
        case DATALAYER_TRACKSEGMENT:
            {
                feature = new DataFeatureTrackSegment();
                break;
            }
        case DATALAYER_PROJECTUSER:
            {
                feature = new DataFeatureProjectUser();
                break;
            }
        case DATALAYER_PROJECTINFO:
            {
                feature = new DataFeatureProjectInfo();
                break;
            }
        case DATALAYER_TASKINFO:
            {
                feature = new DataFeatureTaskInfo();
                break;
            }
                
		default:
			break;
		}

        if(feature != NULL)
        {
            feature->Initialize(NULL, false , layer);
        }

		return feature;
	}

	void DataFeatureFactory::DestroyFeature(DataFeature* feature) const
	{
		delete feature;

		feature = NULL;
	}
}