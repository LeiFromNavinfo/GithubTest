#include "Editor.h"

namespace Editor
{
    SnapEnvironment::SnapEnvironment()
    {
        
    }

    SnapEnvironment::~SnapEnvironment()
    {
        std::vector<DataSnapper*>::iterator iter = this->m_pSnappers.begin();

        for(; iter != this->m_pSnappers.end(); ++iter)
        {
            delete *iter;
        }
        this->m_pSnappers.clear();
    }

    void SnapEnvironment::AddSnapper(DataSnapper* snapper)
    {
        this->m_pSnappers.push_back(snapper);
    }

    void SnapEnvironment::RemoveSnapper(DataSnapper* snapper)
    {
        std::vector<DataSnapper*>::iterator iter = this->m_pSnappers.begin();

        for(; iter != this->m_pSnappers.end(); ++iter)
        {
            if(*iter == snapper)
            {
                iter = this->m_pSnappers.erase(iter);
            }

            if(iter == this->m_pSnappers.end())
            {
                break;
            }
        }
    }

    SnapResult* SnapEnvironment::Snapping(EditorGeometry::Box2D box, EditorGeometry::Point2D point)
    {
        if(m_pSnappers.size() == 0)
        {
            return NULL;
        }

        std::vector<DataSnapper*>::iterator iter = m_pSnappers.begin();

        SnapResult* targetResult = NULL;

        double minDistance = DBL_MAX;

        for(;iter != m_pSnappers.end(); ++iter)
        {
            DataSnapper* snapper = *iter;

            SnapResult* result = snapper->Snapping(box, point);

            if(NULL == result)
            {
                continue;
            }
            
            if(result->GetDistance() < minDistance)
            {
                targetResult = result;

                minDistance = result->GetDistance();
            }
            else
            {
                delete result;
            }
        }
        return targetResult;
    }

}