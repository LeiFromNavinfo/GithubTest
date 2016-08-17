#include "Geometry.h"


namespace EditorGeometry
{
    const char* GetWkbGeometryTypeName(int _wkbType)
    {
        switch (_wkbType)
        {
            case wkbPoint:
            {
                return "wkbPoint";
            }break;
            case wkbLineString:
            {
                return "wkbLineString";
            }break;
            case wkbPolygon:
            {
                return "wkbPolygon";
            }break;
            case wkbMultiPoint:
            {
                return "wkbMultiPoint";
            }break;
            case wkbMultiLineString:
            {
                return "wkbMultiLineString";
            }break;
            case wkbMultiPolygon:
            {
                return "wkbMultiPolygon";
            }break;
            case wkbGeometryCollection:
            {
                return "wkbGeometryCollection";
            }break;
            default:
            {
                return "Unknown";
            }break;
        }
    }
        
    void  Box2D::make_empty()
    {
        _minx = _miny = std::numeric_limits<double>::max();
            
        _maxx = _maxy = std::numeric_limits<double>::min();
    }
        
    void Box2D::make(double x1, double x2, double y1, double y2)
    {
        _minx = x1;
        
		_miny = y1;

        _maxx = x2;
            
        _maxy = y2;
            
        normalize();
    }
        
    bool Box2D::is_empty()
    {
        return _maxx < _minx;
    }
        
    void Box2D::normalize()
    {
        if (_minx > _maxx)
        {
            std::swap(_minx, _maxx);
        }
            
        if(_miny > _maxy)
        {
            std::swap(_miny, _maxy);
        }
    }
        
    bool Box2D::contains(Box2D& other)
    {
        if (is_empty() || other.is_empty()) return false;
        
        return
            other._minx >= _minx &&
            other._maxx <= _maxx &&
            other._miny >= _miny &&
            other._maxy <= _maxy;
    }
        
    bool Box2D::contains(Point2D& point)
    {
        if (is_empty()) return false;
            
        return
            point._x >= _minx &&
            point._x <= _maxx &&
            point._y >= _miny &&
            point._y <= _maxy;
    }
        
    bool Box2D::intersects(Box2D& other)
    {
        if (is_empty() || other.is_empty()) return false;
            
        return !(other._minx > _maxx ||
                    other._maxx < _minx ||
                    other._miny > _maxy ||
                    other._maxy < _miny);
    }
        
    bool Box2D::intersects(Point2D& point)
    {
        return (point._x <= _maxx && point._x >= _minx && point._y <= _maxy && point._y >= _miny);
    }
        
    unsigned int LinearRing2D::buffer_size()
    {
        return sizeof(unsigned int) + _numPoints*sizeof(Point2D);
    }
        
    unsigned int WkbPoint::buffer_size()
    {
        return sizeof(char) + sizeof(unsigned int) + sizeof(Point2D);
    }
        
    Box2D WkbPoint::query_box()
    {
        Box2D box;
            
        box._minx = box._maxx = _point._x;
            
        box._miny = box._maxy = _point._y;
            
        return box;
    }
        
    unsigned int WkbLineString::buffer_size()
    {
        return sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int) + _numPoints*sizeof(Point2D);
    }
        
    Box2D WkbLineString::query_box()
    {
        Box2D box;  box.make_empty();
            
        for (int i = 0; i < _numPoints; i ++)
        {
            box._minx = std::min(box._minx, _points[i]._x);
                
            box._maxx = std::max(box._maxx, _points[i]._x);
                
            box._miny = std::min(box._miny, _points[i]._y);
                
            box._maxy = std::max(box._maxy, _points[i]._y);
        }
            
        return box;
    }
        
    LinearRing2D* WkbPolygon::get_indexed_ring(unsigned int index)
    {
        unsigned char* firstAddress = (unsigned char*)&(this->_rings);
            
        int offset = 0;
            
        for(int i = 0; i < _numRings; i ++)
        {
            LinearRing2D* ring = (LinearRing2D*)(firstAddress+offset);
                
            if(i == index)
            {
                return ring;
            }
                
            offset += ring->buffer_size();
        }
            
        return NULL;
    }
        
    unsigned int WkbPolygon::buffer_size()
    {
        unsigned int size = sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int);
            
        for (int i = 0; i < _numRings; i ++)
        {
            size += get_indexed_ring(i)->buffer_size();
        }
            
        return size;
    }
        
    Box2D WkbPolygon::query_box()
    {
        Box2D box;  box.make_empty();
            
        for (int i = 0; i < _numRings; i ++)
        {
            LinearRing2D* ring = get_indexed_ring(i);
                
            for (int j = 0; j < ring->_numPoints; j ++)
            {
                box._minx = std::min(box._minx, ring->_points[j]._x);
                    
                box._maxx = std::max(box._maxx, ring->_points[j]._x);
                    
                box._miny = std::min(box._miny, ring->_points[j]._y);
                    
                box._maxy = std::max(box._maxy, ring->_points[j]._y);
            }
        }
            
        return box;
    }
        
    WkbPoint* WkbMultiPoint::get_indexed_point(unsigned int index)
    {
        return &_points[index];
    }
        
    unsigned int WkbMultiPoint::buffer_size()
    {
        unsigned int size = sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int);
            
        for (int i = 0; i < _numPoints; i ++)
        {
            size += get_indexed_point(i)->buffer_size();
        }
            
        return size;
    }
        
    Box2D WkbMultiPoint::query_box()
    {
        Box2D box;  box.make_empty();
            
        for (int i = 0; i < _numPoints; i ++)
        {
            WkbPoint* point = get_indexed_point(i);
                
            box._minx = std::min(box._minx, point->_point._x);
                
            box._maxx = std::max(box._maxx, point->_point._x);
                
            box._miny = std::min(box._miny, point->_point._y);
                
            box._maxy = std::max(box._maxy, point->_point._y);
        }
            
        return box;
    }
        
    WkbLineString* WkbMultiLineString::get_indexed_linestring(unsigned int index)
    {
        unsigned char* firstAddress = (unsigned char*)&(this->_lineStrings);
            
        int offset = 0;
            
        for(int i = 0; i < _numLineStrings; i ++)
        {
            WkbLineString* linestring = (WkbLineString*)(firstAddress+offset);
                
            if(i == index)
            {
                return linestring;
            }
                
            offset += linestring->buffer_size();
        }
            
        return NULL;
    }
        
    unsigned int WkbMultiLineString::buffer_size()
    {
        unsigned int size = sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int);
            
        for (int i = 0; i < _numLineStrings; i ++)
        {
            size += get_indexed_linestring(i)->buffer_size();
        }
            
        return size;
    }
        
    Box2D WkbMultiLineString::query_box()
    {
        Box2D box;  box.make_empty();
            
        for (int i = 0; i < _numLineStrings; i ++)
        {
            WkbLineString* ring = get_indexed_linestring(i);
                
            for (int j = 0; j < ring->_numPoints; j ++)
            {
                box._minx = std::min(box._minx, ring->_points[j]._x);
                    
                box._maxx = std::max(box._maxx, ring->_points[j]._x);
                    
                box._miny = std::min(box._miny, ring->_points[j]._y);
                    
                box._maxy = std::max(box._maxy, ring->_points[j]._y);
            }
        }
            
        return box;
    }
        
    WkbPolygon* WkbMultiPolygon::get_indexed_polygon(unsigned int index)
    {
        unsigned char* firstAddress = (unsigned char*)&(this->_polygons);
            
        int offset = 0;
            
        for(int i = 0; i < _numPolygons; i ++)
        {
            WkbPolygon* polygon = (WkbPolygon*)(firstAddress+offset);
                
            if(i == index)
            {
                return polygon;
            }
                
            offset += polygon->buffer_size();
        }
            
        return NULL;
    }
        
    unsigned int WkbMultiPolygon::buffer_size()
    {
        unsigned int size = sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int);
            
        for (int i = 0; i < _numPolygons; i ++)
        {
            size += get_indexed_polygon(i)->buffer_size();
        }
            
        return size;
    }
        
    Box2D WkbMultiPolygon::query_box()
    {
        Box2D box;  box.make_empty();
            
        for (int i = 0; i < _numPolygons; i ++)
        {
            WkbPolygon* polygon = get_indexed_polygon(i);
                
            for (int j = 0; j < polygon->_numRings; j ++)
            {
                LinearRing2D* ring = polygon->get_indexed_ring(j);
                    
                for (int k = 0; k < ring->_numPoints; k ++)
                {
                    box._minx = std::min(box._minx, ring->_points[k]._x);
                        
                    box._maxx = std::max(box._maxx, ring->_points[k]._x);
                        
                    box._miny = std::min(box._miny, ring->_points[k]._y);
                        
                    box._maxy = std::max(box._maxy, ring->_points[k]._y);
                }
            }
        }
            
        return box;
    }
        
    WkbGeometry* WkbGeometryCollection::get_indexed_geometry(unsigned int index)
    {
        unsigned char* firstAddress = (unsigned char*)&(this->_geometries);
            
        int offset = 0;
            
        for(int i = 0; i < _numGeometries; i ++)
        {
            WkbGeometry* geometry = (WkbGeometry*)(firstAddress+offset);
                
            if(i == index)
            {
                return geometry;
            }
                
            offset += geometry->buffer_size();
        }
            
        return NULL;
    }
        
    unsigned int WkbGeometryCollection::buffer_size()
    {
        unsigned int size = sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int);
            
        for (int i = 0; i < _numGeometries; i ++)
        {
            size += get_indexed_geometry(i)->buffer_size();
        }
            
        return size;
    }
        
    Box2D WkbGeometryCollection::query_box()
    {
        Box2D box;  box.make_empty();
            
        for (int i = 0; i < _numGeometries; i ++)
        {
            WkbGeometry* geometry = get_indexed_geometry(i);
                
            Box2D b = geometry->query_box();
                
            box._minx = std::min(box._minx, b._minx);
                
            box._maxx = std::max(box._maxx, b._maxx);
                
            box._miny = std::min(box._miny, b._miny);
                
            box._maxy = std::max(box._maxy, b._maxy);
        }
            
        return box;
    }
        
    unsigned int WkbGeometry::buffer_size()
    {
        switch (this->_wkbType)
        {
            case wkbPoint:
            {
                WkbPoint* geo = (WkbPoint*)this;
                    
                return geo->buffer_size();
            }break;
            case wkbLineString:
            {
                WkbLineString* geo = (WkbLineString*)this;
                    
                return geo->buffer_size();
            }break;
            case wkbPolygon:
            {
                WkbPolygon* geo = (WkbPolygon*)this;
                    
                return geo->buffer_size();
            }break;
            case wkbMultiPoint:
            {
                WkbMultiPoint* geo = (WkbMultiPoint*)this;
                    
                return geo->buffer_size();
            }break;
            case wkbMultiLineString:
            {
                WkbMultiLineString* geo = (WkbMultiLineString*)this;
                    
                return geo->buffer_size();
            }break;
            case wkbMultiPolygon:
            {
                WkbMultiPolygon* geo = (WkbMultiPolygon*)this;
                    
                return geo->buffer_size();
            }break;
            case wkbGeometryCollection:
            {
                WkbGeometryCollection* geo = (WkbGeometryCollection*)this;
                    
                return geo->buffer_size();
            }break;
            default:
            {
                return 0;
            }break;
        }
    }
        
    Box2D WkbGeometry::query_box()
    {
        switch (this->_wkbType)
        {
            case wkbPoint:
            {
                WkbPoint* geo = (WkbPoint*)this;
                    
                return geo->query_box();
            }break;
            case wkbLineString:
            {
                WkbLineString* geo = (WkbLineString*)this;
                    
                return geo->query_box();
            }break;
            case wkbPolygon:
            {
                WkbPolygon* geo = (WkbPolygon*)this;
                    
                return geo->query_box();
            }break;
            case wkbMultiPoint:
            {
                WkbMultiPoint* geo = (WkbMultiPoint*)this;
                    
                return geo->query_box();
            }break;
            case wkbMultiLineString:
            {
                WkbMultiLineString* geo = (WkbMultiLineString*)this;
                    
                return geo->query_box();
            }break;
            case wkbMultiPolygon:
            {
                WkbMultiPolygon* geo = (WkbMultiPolygon*)this;
                    
                return geo->query_box();
            }break;
            case wkbGeometryCollection:
            {
                WkbGeometryCollection* geo = (WkbGeometryCollection*)this;
                    
                return geo->query_box();
            }break;
            default:
            {
                Box2D box;  box.make_empty();
                    
                return box;
            }break;
        }
    }

	SpatialiteGeometry* SpatialiteGeometry::FromWKBGeometry(WkbGeometry* geo)
	{
        void* buff = malloc(geo->buffer_size()+39);

        SpatialiteGeometry* spGeo = (SpatialiteGeometry*)buff;

        spGeo->START=0x00;

        spGeo->ENDIAN=geo->_byteOrder;

        spGeo->MBR_END = 0x7C;

		Box2D box = geo->query_box();
        
        SetNumber<double>((WkbByteOrder)geo->_byteOrder, &spGeo->MBR_MAX_X, box._maxx);

        SetNumber<double>((WkbByteOrder)geo->_byteOrder, &spGeo->MBR_MAX_Y, box._maxy);

        SetNumber<double>((WkbByteOrder)geo->_byteOrder, &spGeo->MBR_MIN_X, box._minx);

        SetNumber<double>((WkbByteOrder)geo->_byteOrder, &spGeo->MBR_MIN_Y, box._miny);

        SetNumber<int>((WkbByteOrder)geo->_byteOrder, &spGeo->SRID, 4326);

        unsigned char* address = (unsigned char*)&(spGeo->GEOMETRY);

        memcpy(address, ((unsigned char*)geo)+1, geo->buffer_size()-1);

        *((unsigned char*)buff+geo->buffer_size()+39-1)=0xFE;

        return spGeo;
	}

	WkbGeometry* SpatialiteGeometry::ToWKBGeometry(SpatialiteGeometry* spGeo)
	{
        unsigned char* buff = (unsigned char*)(spGeo);

        unsigned char order = buff[1];

        buff[38] = order;

        WkbGeometry* geo = (WkbGeometry*)(buff+38);

        WkbGeometry* ret = (WkbGeometry*)malloc(geo->buffer_size());

        memcpy(ret, geo, geo->buffer_size());

        buff[38] = 0x7C;

        return ret;
	}
}
