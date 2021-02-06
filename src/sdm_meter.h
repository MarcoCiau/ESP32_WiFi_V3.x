#ifndef SDM_METER_H
#define SDM_METER_H
#ifdef ENABLE_SDM_METER
#include <Arduino.h>
extern void sdm_meter_begin();
extern void sdm_meter_loop();
#endif //ENABLE_SDM_METER
#endif //SDM_METER_H