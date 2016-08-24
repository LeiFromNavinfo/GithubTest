#ifndef FM_SDK_Geometry_h
#define FM_SDK_Geometry_h

#include "EndianHelper.h"
#include "Editor_Define.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <numeric>
#include <float.h>
#include <assert.h>
#include <stdlib.h>

namespace EditorGeometry
{
    enum WkbByteOrder
    {
		wkbXDR = 0, // Big Endian
		wkbNDR = 1  // Little Endian
    };
        
    enum WkbGeometryType
    {
        wkbPoint = 1,
        wkbLineString = 2,
        wkbPolygon = 3,
        wkbMultiPoint = 4,
        wkbMultiLineString = 5,
        wkbMultiPolygon = 6,
		wkbGeometryCollection = 7
    };

#if (defined(__APPLE__)) || (defined(ANDROID))
    inline void *__memcpy(void * dst, void const * src, size_t len)
    {
        char * pDst = (char *) dst;
        char const * pSrc = (char const *) src;
            
        while (len--)
        {
            *pDst++ = *pSrc++;
        }
            
        return (dst);
    }
#endif    
	template<class T>
	inline T GetNumber(WkbByteOrder byteOrder,T num)
	{
		if((int)EndianHelper::HostEdian()==(int)byteOrder)
		{
			return num;
		}
		else
		{
			return swap_endian<T>(num);
		}
	}
        
	template<class T>
	inline void SetNumber(WkbByteOrder byteOrder,T* num,T value)
	{
		if((int)EndianHelper::HostEdian()==(int)byteOrder)
		{

#if (defined(__APPLE__)) || (defined(ANDROID))
            __memcpy(num,&value,sizeof(T));
#else
			*num = value;
#endif
		}
		else
		{
			*num=swap_endian<T>(value);
		}
	}
 
    EDITOR_API const char* GetWkbGeometryTypeName(int _wkbtype);
        
    #pragma pack(push,1)
        
    struct EDITOR_API Point2D
	{
		double _x;
            
		double _y;
	};
        
    struct EDITOR_API LinearRing2D
	{
		unsigned int _numPoints;

		Point2D		 _points[1];
            
        unsigned int buffer_size();
	};
    
    struct EDITOR_API Box2D
    {
        double _minx;
            
        double _miny;
            
        double _maxx;
            
        double _maxy;
            
        void make(double x1, double x2, double y1, double y2);
            
        void make_empty();
            
        bool is_empty();
            
        void normalize();
            
        bool contains(Box2D& other);
            
        bool contains(Point2D& point);
            
        bool intersects(Box2D& other);
            
        bool intersects(Point2D& point);
    };
        
    struct EDITOR_API WkbGeometry
	{
		char _byteOrder;

		unsigned int _wkbType;
            
        unsigned int buffer_size();
            
        Box2D query_box();
	};
        
    struct EDITOR_API WkbPoint:public WkbGeometry
	{
		Point2D _point;
            
        unsigned int buffer_size();
            
        Box2D query_box();
	};
        
    struct EDITOR_API WkbLineString :public WkbGeometry
	{
		unsigned int _numPoints;

		Point2D _points[1];
            
        unsigned int buffer_size();
            
        Box2D query_box();
	};
        
    struct EDITOR_API WkbPolygon :public WkbGeometry
	{
		unsigned int _numRings;
            
		LinearRing2D _rings[1];
            
        LinearRing2D* get_indexed_ring(unsigned int index);
            
        unsigned int buffer_size();
            
        Box2D query_box();
	};
        
    struct EDITOR_API WkbMultiPoint :public WkbGeometry
	{
		unsigned int _numPoints;

		WkbPoint _points[1];
            
        WkbPoint* get_indexed_point(unsigned int index);
            
        unsigned int buffer_size();
            
        Box2D query_box();
	};
        
    struct EDITOR_API WkbMultiLineString :public WkbGeometry
	{
		unsigned int _numLineStrings;

		WkbLineString _lineStrings[1];
            
        WkbLineString* get_indexed_linestring(unsigned int index);
            
        unsigned int buffer_size();
            
        Box2D query_box();
	};
        
    struct EDITOR_API WkbMultiPolygon :public WkbGeometry
	{
		unsigned int _numPolygons;
            
		WkbPolygon _polygons[1];
            
        WkbPolygon* get_indexed_polygon(unsigned int index);
            
        unsigned int buffer_size();
            
        Box2D query_box();
	};
        
    struct EDITOR_API WkbGeometryCollection :public WkbGeometry
	{
		unsigned int _numGeometries;
            
		WkbGeometry _geometries[1];
            
        WkbGeometry* get_indexed_geometry(unsigned int index);
            
        unsigned int buffer_size();
            
        Box2D query_box();
	};

    struct EDITOR_API SpatialiteGeometry
	{
		unsigned char   START;
		unsigned char   ENDIAN;
		int             SRID;
		double          MBR_MIN_X;
		double          MBR_MIN_Y;
		double          MBR_MAX_X;
		double          MBR_MAX_Y;
		unsigned char   MBR_END;
		void*           GEOMETRY;

	public:
		static SpatialiteGeometry* FromWKBGeometry(WkbGeometry* geo);

		static WkbGeometry* ToWKBGeometry(SpatialiteGeometry* spGeo);
	};

	#pragma pack(pop)
}

#endif
