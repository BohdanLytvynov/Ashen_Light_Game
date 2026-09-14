// Fill out your copyright notice in the Description page of Project Settings.


#include "Utilities/Sensors/SensorTraceUtility.h"
#include "Components/Base/HitSensor.h"

bool FSensorTraceUtility::PerformTraceWithSensorConfig(UHitSensor* sensorTemplate, const FVector& start, const FVector& end, const FQuat& q, const FCollisionShape& collShape, bool traceComplex, FHitResult& hit, bool debug)
{
    if (!sensorTemplate) return false;
    UWorld* w = sensorTemplate->GetWorld();
    if (!w) return false;
    bool r = sensorTemplate->DoScanInternal(w, hit, start, end, q, collShape, traceComplex);
    if (debug)
    {
        float debugRadius = 10.0f;
        if (collShape.IsSphere())
        {
            debugRadius = collShape.GetSphereRadius();
        }
        else if (collShape.IsCapsule())
        {
            debugRadius = collShape.GetCapsuleRadius();
        }                
        sensorTemplate->Debug(w, start, end, debugRadius);
    }
    return r;
}
