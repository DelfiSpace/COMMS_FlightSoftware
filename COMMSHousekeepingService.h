/*
 * COMMSTelemetryService.h
 *
 *  Created on: 4 Aug 2019
 *      Author: stefanosperett
 */

#ifndef COMMSHOUSEKEEPINGSERVICE_H_
#define COMMSHOUSEKEEPINGSERVICE_H_

#include "HousekeepingService.h"
#include "COMMSTelemetryContainer.h"

class COMMSHousekeepingService: public HousekeepingService
{
protected:
    COMMSTelemetryContainer telemetryContainer[2];

public:
    virtual TelemetryContainer* getContainerToRead()
    {
        return &(telemetryContainer[(telemetryIndex + 1) % 2]);
    };

    virtual TelemetryContainer* getContainerToWrite()
    {
        return &(telemetryContainer[telemetryIndex]);
    };
};
#endif /* COMMSHOUSEKEEPINGSERVICE_H_ */
