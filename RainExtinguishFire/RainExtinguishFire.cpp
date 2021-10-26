#include "plugin.h"
#include "CGeneral.h"
#include "CCamera.h"
#include "CZone.h"
#include "CCullZones.h"
#include "CMessages.h"
#include "CFireManager.h"
#include "CWeather.h"
#include "CTimer.h"
#include "CGame.h"

using namespace plugin;

CCullZone *CCullZones__aAttributeZones = nullptr;
int& CCullZones::NumAttributeZones = *(int*)0xC87AC8;
unsigned int maxFires = 0;

eZoneAttributes CCullZones__FindAttributesForCoors3D(float x, float y, float z)
{
    if (CCullZones::NumAttributeZones <= 0)
        return eZoneAttributes::NONE;

    int out = eZoneAttributes::NONE;
    for (int i = 0; i < CCullZones::NumAttributeZones; ++i)
    {
        if (CCullZones__aAttributeZones[i].IsPointWithin({ x, y, z }))
        {
            out |= CCullZones__aAttributeZones[i].flags;
        }
    }

    return static_cast<eZoneAttributes>(out);
}

class RainExtinguishFire
{
public:
    RainExtinguishFire()
    {
        Events::processScriptsEvent += []
        {
            if (maxFires == 0) {
                CCullZones__aAttributeZones = injector::ReadMemory<CCullZone*>(0x72DAF9 + 1, true);
                maxFires = injector::ReadMemory<uint32_t>(0x538BF4 + 4, true) * 10;
            }
            else
            {
                float rainIntensity = CWeather::Rain - 0.3f;
                if (rainIntensity > 0.1f && CGame::currArea == 0)
                {
                    rainIntensity *= 0.75f;

                    for (int i = 0; i < maxFires; ++i)
                    {
                        if (gFireManager.m_aFires[i].m_fStrength > 0.0f)
                        {
                            CVector firePos = gFireManager.m_aFires[i].m_vecPosition;
                            eZoneAttributes zoneAttr = CCullZones__FindAttributesForCoors3D(firePos.x, firePos.y, firePos.z);
                            if ((zoneAttr & eZoneAttributes::CAM_NO_RAIN) == 0)
                            {
                                // use default game call to extinguish point with water; safer
                                gFireManager.ExtinguishPointWithWater(gFireManager.m_aFires[i].m_vecPosition, 0.5f, rainIntensity);
                            }
                        }
                    }
                }
            }
        };
        
    }
} rainExtinguishFire;
