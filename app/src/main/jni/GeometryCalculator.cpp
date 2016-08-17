#include "GeometryCalculator.h"
#include "BussinessConstants.h"

namespace Editor
{
    GeometryCalculator* GeometryCalculator::m_pInstance = NULL;

    GeometryCalculator::GeometryCalculator()
    {
        
    }

    GeometryCalculator* GeometryCalculator::getInstance()
    {
        if(NULL == m_pInstance)
        {
            m_pInstance = new GeometryCalculator();
        }

        return m_pInstance;
    }

    bool GeometryCalculator::IsPointEquals(double x1, double y1, double x2, double y2)
    {
        if(fabs(x1-x2)>PrecisionMap)
        {
            return false;
        }

        if(fabs(y1-y2)>PrecisionMap)
        {
            return false;
        }
        
        return true;
    }

    bool GeometryCalculator::IsPointEquals(const geos::geom::Coordinate& point1, const geos::geom::Coordinate& point2)
    {
        return IsPointEquals(point1.x,point1.y,point2.x,point2.y);
    }

    bool GeometryCalculator::IsCollinear(const geos::geom::Coordinate& a, const geos::geom::Coordinate& b, const geos::geom::Coordinate& c,double max_tolerance)
    {
        double x1=a.x* PrecisionGeo;
        double y1=a.y* PrecisionGeo;

        double x2=b.x* PrecisionGeo;
        double y2=b.y* PrecisionGeo;

        double x3=c.x* PrecisionGeo;
        double y3=c.y* PrecisionGeo;

        double Z = (x2- x1) * (y3 - y2) - 
            (x3 - x2) * (y2 - y1);
        
        return fabs((double)Z) <= max_tolerance;
    }

    bool GeometryCalculator::IsPointAtLineInter(const geos::geom::Coordinate& start, const geos::geom::Coordinate& end, const geos::geom::Coordinate& target,double max_tolerance)
    {
        std::pair<double, std::pair<double, double> > distance = MeasurePointToSegLineDistanceByHeron(target.x, target.y, start.x, start.y, end.x, end.y);
        
        if(distance.first > 1.0)
        {
            return false;
        }
        
        if(std::abs(end.x - start.x) >= std::abs(end.y - start.y))
        {
            return ((start.x < target.x) && (target.x < end.x)) ||
            ((start.x > target.x) && (target.x > end.x));
        }
        else
        {
            return ((start.y < target.y) && (target.y < end.y)) ||
            ((start.y > target.y) && (target.y > end.y));
        }
    }

    bool GeometryCalculator::LocatePointInPath(geos::geom::Point* target, geos::geom::LineString* line, int& index)
    {
        if(target == NULL || line == NULL)
        {
            return false;
        }

        geos::geom::CoordinateSequence* line_coordinates = line->getCoordinates();

        if(line_coordinates == NULL)
        {
            return false;
        }
        
        geos::geom::Coordinate target_point; 
        target_point.x = target->getX();
        target_point.y = target->getY();

        index = -1;
        bool onVertex =false;

        geos::geom::Coordinate currentPoint, nextPoint;

        for (int i = 0; i < line_coordinates->size(); i++)
        {
            currentPoint = line_coordinates->getAt(i);

            if(IsPointEquals(currentPoint, target_point))
            {
                index = i;
                onVertex = true;
                break;
            }

            if(i != line_coordinates->size()-1)
            {
                nextPoint = line_coordinates->getAt(i+1);

                if(IsPointAtLineInter(currentPoint, nextPoint,target_point))
                {
                    index = i;
                    break;
                }
            }
        }
        return onVertex;
    }
    double GeometryCalculator::CalcAngle(EditorGeometry::WkbGeometry* wkb, EditorGeometry::Point2D point, int type)
	{
		std::pair<double, double> result = GetVector(wkb, point, type);
		double EastAngle = (180/PI) * acos(result.first / sqrt(pow(result.first, 2) + pow(result.second, 2)));
		double WestAngle = (180/PI) * acos(-result.first / sqrt(pow(result.first, 2) + pow(result.second, 2)));
		double angle = -1;
		if(WestAngle <= EastAngle)
		{
			angle = 360 - (180/PI) * acos(result.second / sqrt(pow(result.first, 2) + pow(result.second, 2)));
		}
		else
		{
			angle = (180/PI) * acos(result.second / sqrt(pow(result.first, 2) + pow(result.second, 2)));
		}
		return angle;
	}
	std::pair<double, double> GeometryCalculator::GetVector(EditorGeometry::WkbGeometry* wkb, EditorGeometry::Point2D point, int type)
	{
		std::pair<double, double> result;
		geos::geom::Geometry* geos = DataTransfor::Wkb2Geo(wkb);
		geos::geom::CoordinateSequence* coords = geos->getCoordinates();
		double dist = DBL_MAX;
		double leadx = -1;
		double leady = -1;
		double leadDirX = -1;
		double leadDirY = -1;
		int nCount = coords->size();
		for(int i = 0; i < nCount - 1; i++)
		{
			std::pair<double, std::pair<double, double> > result = MeasurePointToSegLineDistanceByHeron(point._x, point._y,coords->getAt(i).x, coords->getAt(i).y,coords->getAt(i+1).x, coords->getAt(i+1).y);
			if(result.first - dist < 0)
			{
				dist = result.first;
				leadx = result.second.first;
				leady = result.second.second;

//				if(type == 2)
//				{
//					leadDirX = coords->getAt(i+1).x;
//					leadDirY = coords->getAt(i+1).y;
//				}
//				else if(type == 3)
//				{
//					leadDirX = coords->getAt(i).x;
//					leadDirY = coords->getAt(i).y;
//				}
//				else
//				{
				leadDirX = leadx - point._x;
				leadDirY = leady - point._y;
//				}
			}
		}
//		if(type != 1)
//		{
//			result.first = leadDirX - leadx;
//			result.second = leadDirY - leady;
//		}
//		else
//		{
		result.first = leadDirY;
		result.second = -leadDirX;
//		}
		return result;
	}
	double  GeometryCalculator::MeasurePointToPointDistance(double x1, double y1, double x2, double y2)
	{
		return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
	}
	std::pair<double, std::pair<double, double> >    GeometryCalculator::MeasurePointToSegLineDistanceByHeron(double px, double py, double ax, double ay, double bx, double by)
	{
		std::pair<double, std::pair<double, double> > result;
		double pa = MeasurePointToPointDistance(px, py, ax, ay);
		if(pa <= 0.00001)
		{
            result.second.first = ax;
            result.second.second = ay;
			return result;
		}
		double pb = MeasurePointToPointDistance(px, py, bx, by);
		if(pb <= 0.00001)
		{
            result.second.first = bx;
            result.second.second = by;
			return result;
		}
		double ab = MeasurePointToPointDistance(ax, ay, bx, by);
		if(ab <= 0.00001)
		{
            result.second.first = ax;
            result.second.second = ay;
			return result;
		}
		if(pa * pa >= pb * pb + ab * ab)
		{
			result.second.first = bx;
			result.second.second = by;
			result.first = pb;
            return result;
		}
		if(pb * pb >= pa * pa + ab * ab)
		{
			result.second.first = ax;
			result.second.second = ay;
			result.first = pa;
            return result;
		}
		double L = (pa + pb + ab) / 2;
		double S = sqrt(L * (L - pa) * (L - pb) * (L - ab));
		double H = 2 * S / ab;
		double A = by - ay;//A=y2-y1
		double B = -1 * (bx - ax);//B=-(x2-x1)
		double C = ay * (bx - ax) - ax * (by - ay); //c=y1(x2-x1)-x1(y2-y1)
		double Hx = px - A * (A * px + B * py + C) / (A * A + B * B);
		double Hy = py - B * (A * px + B * py + C) / (A * A + B * B);
		result.second.first = Hx;
		result.second.second = Hy;
		result.first = H;
        
		return result;
	}

    geos::geom::LineString* GeometryCalculator::GetLineSegment(geos::geom::Point* sPoint, geos::geom::Point* ePoint, geos::geom::LineString* line)
    {
        if(NULL == sPoint || NULL == ePoint || NULL == line)
        {
            return NULL;
        }

        int sIndex, eIndex;

        bool sIsOnVertex, eIsOnVertex;

        sIsOnVertex = LocatePointInPath(sPoint, line, sIndex);
        eIsOnVertex = LocatePointInPath(ePoint, line, eIndex);

        if(sIndex == -1 || eIndex == -1)
        {
            Logger::LogD("GetLineSegment, 起点或终点不在线上！起点:%s, 终点:%s", sPoint->toText().c_str(), ePoint->toText().c_str());

			Logger::LogO("GetLineSegment, 起点或终点不在线上！起点:%s, 终点:%s", sPoint->toText().c_str(), ePoint->toText().c_str());
            return NULL;
        }

        geos::geom::CoordinateSequence* lineCoords = line->getCoordinates();

        if(NULL == lineCoords)
        {
            return NULL;
        }

        geos::geom::Coordinate sCoordinate(sPoint->getCoordinate()->x,sPoint->getCoordinate()->y);
        geos::geom::Coordinate eCoordinate(ePoint->getCoordinate()->x,ePoint->getCoordinate()->y);

        geos::geom::CoordinateArraySequence coords;

        if(sIsOnVertex)
        {
            if(sIndex<eIndex)
            {
                for(int i=sIndex;i<=eIndex;i++)
                {
                    geos::geom::Coordinate coord = lineCoords->getAt(i);

                    coords.add(coord);
                }
            }
            else
            {
                for(int i=sIndex;i>eIndex;i--)
                {
                    geos::geom::Coordinate coord = lineCoords->getAt(i);

                    coords.add(coord);
                }
                
                if(eIsOnVertex)
                {
                    coords.add(eCoordinate);
                }
            }
        }
        else
        {
            coords.add(sCoordinate);

            if(sIndex<eIndex)
            {
                for(int i=sIndex+1;i<=eIndex;i++)
                {
                    geos::geom::Coordinate coord = lineCoords->getAt(i);

                    coords.add(coord);
                }
            }
            else
            {
                for(int i=sIndex;i>eIndex;i--)
                {
                    geos::geom::Coordinate coord = lineCoords->getAt(i);

                    coords.add(coord);
                }
                
                if(eIsOnVertex)
                {
                    coords.add(eCoordinate);
                }
            }
        }
        
        if(!eIsOnVertex)
        {
            coords.add(eCoordinate);
        }

        geos::geom::LineString* result = geos::geom::GeometryFactory::getDefaultInstance()->createLineString(coords);

        return result;
    }

    int GeometryCalculator::GetCrossPointType(geos::geom::Geometry* source, geos::geom::Geometry* target)
    {
        if(NULL == source || NULL == target)
        {
            return -1;
        }

        geos::geom::CoordinateSequence* sourceCoords = source->getCoordinates();
        geos::geom::CoordinateSequence* targetCoords = target->getCoordinates();

        if(NULL == sourceCoords || NULL == targetCoords)
        {
            return -1;
        }

        if(IsPointEquals(sourceCoords->getAt(0),targetCoords->getAt(0)))
        {
            return 0;
        }
        
        if(IsPointEquals(sourceCoords->getAt(0),targetCoords->getAt(targetCoords->size()-1)))
        {
            return 1;
        }
        
        if(IsPointEquals(sourceCoords->getAt(sourceCoords->size()-1),targetCoords->getAt(0)))
        {
            return 2;
        }
        
        if(IsPointEquals(sourceCoords->getAt(sourceCoords->size()-1),targetCoords->getAt(targetCoords->size()-1)))
        {
            return 3;
        }
        
        return -1;
    }
    
     int GeometryCalculator::GetCrossPointTypeEx(std::list<geos::geom::Coordinate>& source, geos::geom::Geometry* target)
    {
        if(source.size()==0 || NULL == target)
        {
            return -1;
        }
        
        geos::geom::CoordinateSequence* targetCoords = target->getCoordinates();
        
        if(NULL == targetCoords)
        {
            return -1;
        }
        
        if(IsPointEquals(source.front(),targetCoords->getAt(0)))
        {
            return 0;
        }
        else if(IsPointEquals(source.back(),targetCoords->getAt(0)))
        {
            return 1;
        }
        else if (IsPointEquals(source.front(),targetCoords->getAt(targetCoords->size()-1)))
        {
            return 2;
        }
        else if (IsPointEquals(source.back(),targetCoords->getAt(targetCoords->size()-1)))
        {
            return 3;
        }
        
        return -1;
    }
   
    std::pair<double, double> GeometryCalculator::GetVector(const std::string& point, EditorGeometry::WkbGeometry* line, int dir)
    {
        std::pair<double, double> result;
		
        try
        {
            geos::geom::Geometry* pointGeo = DataTransfor::Wkt2Geo(point);
			
            geos::geom::Geometry* lineGeo = DataTransfor::Wkb2Geo(line);
			
            geos::geom::Point* p = dynamic_cast<geos::geom::Point*>(pointGeo);
			
            geos::geom::LineString* lineStr = dynamic_cast<geos::geom::LineString*>(lineGeo);
            
			int index = -1;
			
            bool flag = Editor::GeometryCalculator::getInstance()->LocatePointInPath(p, lineStr, index);
            
			if(index == -1)
            {
                Logger::LogD("location point in path unsuccessfully!!!");
                Logger::LogO("location point in path unsuccessfully!!!");
				
                return result;
            }
			
            geos::geom::CoordinateSequence* coords = lineGeo->getCoordinates();
			
            if(coords->getSize() == index)
            {
                if(dir != 3)
                {
                    result.first = coords->getAt(index).x - coords->getAt(index - 1).x;
                    result.second = coords->getAt(index).y - coords->getAt(index - 1).y;
                }
                else
                {
                    result.first = coords->getAt(index - 1).x - coords->getAt(index).x;
                    result.second = coords->getAt(index - 1).y - coords->getAt(index).y;
                }
            }
            else if(0 == index)
            {
                if(dir != 3)
                {
                    result.first = coords->getAt(index + 1).x - coords->getAt(index).x;
                    result.second = coords->getAt(index + 1).y - coords->getAt(index).y;
                }
                else
                {
                    result.first = coords->getAt(index).x - coords->getAt(index + 1).x;
                    result.second = coords->getAt(index).y - coords->getAt(index + 1).y;
                }
            }
            else
            {
                if(dir != 3)
                {
                    result.first = coords->getAt(index + 1).x - coords->getAt(index).x;
                    result.second = coords->getAt(index + 1).y - coords->getAt(index).y;
                }
                else
                {
                    result.first = coords->getAt(index - 1).x - coords->getAt(index).x;
                    result.second = coords->getAt(index - 1).y - coords->getAt(index).y;
                }
            }
			
            delete coords;
            delete lineStr;
            delete p;
			
            return result;
        }
        catch(...)
        {
            Logger::LogD("that convert wkb to geos is unsuccessfully!!!");
            Logger::LogO("that convert wkb to geos is unsuccessfully!!!");
        }
		
        return result;
    }
std::pair<double, double> GeometryCalculator::GetPedalPoint(EditorGeometry::WkbGeometry* wkb, EditorGeometry::Point2D point, double& distance)
    {
        std::pair<double, double> returnResult;

		geos::geom::Geometry* geos = DataTransfor::Wkb2Geo(wkb);

		geos::geom::CoordinateSequence* coords = geos->getCoordinates();

		double dist = DBL_MAX;

		int nCount = coords->size();

		for(int i = 0; i < nCount - 1; i++)
		{
			std::pair<double, std::pair<double, double> > result = MeasurePointToSegLineDistanceByHeron(point._x, point._y,coords->getAt(i).x, coords->getAt(i).y,coords->getAt(i+1).x, coords->getAt(i+1).y);
			if(result.first < dist)
			{
				dist = result.first;
                distance = dist;
				returnResult.first = result.second.first;
                returnResult.second = result.second.second;
			}
		}

		return returnResult;
    }

    double GeometryCalculator::CaclDistFromPointToEndOfLIne(geos::geom::Geometry* geo, geos::geom::Point* point_1, int index_1, geos::geom::Point* point_2, int index_2)
    {
        double dist = 0.0;

        try
        {
            geos::geom::CoordinateSequence* coords = geo->getCoordinates();

            if(coords->getSize() > index_1  && coords->getSize() > index_2)
            {
                int nStart = -1;

                int nEnd = -1;

                if(index_1 - index_2 < 0)
                {
                    nStart = 0;

                    nEnd = index_1;
                }
                else if(index_1 - index_2 > 0)
                {
                    nStart = index_1 + 1;

                    nEnd = coords->getSize() -1;
                }
                else
                {
                    geos::geom::Point* tpmPoint = geos::geom::GeometryFactory::getDefaultInstance()->createPoint(coords->getAt(index_1));

                    double dist_1 = point_1->distance(tpmPoint);

                    delete tpmPoint;

                    tpmPoint = geos::geom::GeometryFactory::getDefaultInstance()->createPoint(coords->getAt(index_2));

                    double dist_2 = point_2->distance(tpmPoint);

                    delete tpmPoint;

                    if(dist_1 - dist_2 < 0)
                    {
                        nStart = 0;

                        nEnd = index_1;
                    }
                    else
                    {
                        nStart = index_1;

                        nEnd = coords->getSize() -1;
                    }
                }

                geos::geom::CoordinateArraySequence coordSeq;          

                for(int i = nStart; i < nEnd && i < coords->getAt(i); i++)
                {
                    coordSeq.add(coords->getAt(i));
                }

                geos::geom::LineString* lineStr = geos::geom::GeometryFactory::getDefaultInstance()->createLineString(coordSeq);

                dist = lineStr->getLength();

                delete lineStr;
            }          
        }
        catch(...)
        {
            Logger::LogD("CaclDistFromPointToEndOfLIne is error!!!");
            Logger::LogO("CaclDistFromPointToEndOfLIne is error!!!");

            return -1;
        }

        return dist;
    }
    
    geos::geom::Geometry* GeometryCalculator::GetSETipsStartEndLineGeo(const std::vector<geos::geom::Geometry*>& lines, geos::geom::Geometry* point, int type)
    {
        if(lines.size()==0 || point == NULL)
        {
            return NULL;
        }
        
        geos::geom::Geometry* result = NULL;
        
        //如果只有一条组成线，返回整条线
        if(lines.size() == 1)
        {
            result = geos::geom::GeometryFactory::getDefaultInstance()->createLineString(lines.at(0)->getCoordinates());
            return result;
        }
        
        //截取方向，0:向起点截取，1:向终点截取
        int direct = -1;
        
        geos::geom::Geometry* source = NULL;
        
        if(type == 0)
        {
            source = lines.at(0);
            
            for(int i =1; i<lines.size();i++)
            {
                int crossType = GetCrossPointType(source, lines.at(i));
                
                if(crossType == 0 || crossType ==1)//挂接点是起点
                {
                    direct = 0;
                    
                    break;
                }
                else if(crossType == 2 || crossType ==3)//挂接点是终点
                {
                    direct = 1;
                    
                    break;
                }
            }
        }
        else
        {
            source = lines.at(lines.size()-1);
            
            for(int i =lines.size()-2; i>=0;i--)
            {
                int crossType = GetCrossPointType(source, lines.at(i));
                
                if(crossType == 0 || crossType ==1)//挂接点是起点
                {
                    direct = 0;
                    
                    break;
                }
                else if(crossType == 2 || crossType ==3)//挂接点是终点
                {
                    direct = 1;
                    
                    break;
                }
            }
        }
        
        //未找到挂接线（不连续）,返回整条线
        if(direct == -1)
        {
            if(source)
                result = geos::geom::GeometryFactory::getDefaultInstance()->createLineString(source->getCoordinates());
            return result;
        }
        
        int index = -1;
        
        bool isOnVertx = LocatePointInPath(dynamic_cast<geos::geom::Point*>(point), dynamic_cast<geos::geom::LineString*>(source), index);
        
        if(index == -1)
        {
            Logger::LogD("GetSETipsStartEndLineGeo: start or end point is not on the line!");
            Logger::LogO("GetSETipsStartEndLineGeo: start or end point is not on the line!");
            
            return NULL;
        }
        
        geos::geom::CoordinateArraySequence coords;
        
        geos::geom::Coordinate coord;
        coord.x = point->getCoordinate()->x;
        coord.y = point->getCoordinate()->y;
        
        coords.add(coord);
        
        if(direct == 0)
        {
            if(index == 0)
            {
                coords.add(source->getCoordinates()->getAt(index));
            }
            else
            {
                for(int i = index-1; i>=0; i--)
                {
                    coords.add(source->getCoordinates()->getAt(i));
                }
            }
        }
        else if(direct == 1)
        {
            if(index == source->getCoordinates()->size()-1)
            {
                coords.add(source->getCoordinates()->getAt(index));
            }
            else
            {
                for(int i = index+1; i<source->getCoordinates()->size(); i++)
                {
                    coords.add(source->getCoordinates()->getAt(i));
                }
            }
        }
        
        result = geos::geom::GeometryFactory::getDefaultInstance()->createLineString(coords);
        
        return result;
    }
}