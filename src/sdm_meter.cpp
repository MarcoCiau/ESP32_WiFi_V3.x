#ifdef ENABLE_SDM_METER
#include "sdm_meter.h"
#include "debug.h"
#include <SDM.h>  
#ifdef USE_SDM_SOFTWARE_SERIAL
#include <SoftwareSerial.h>  
#endif
#include "input.h"

#define READ_SAMPLE_RATE_MS 10000
static unsigned long readValuesTimer = 0;
static float accumulated_kwh = 0;
static uint32_t accumulated_wh = 0;
#ifdef USE_SDM_SOFTWARE_SERIAL
SoftwareSerial swSerSDM;                                                       
SDM sdm(swSerSDM, 4800, SDM_DERE, SWSERIAL_8N1, SDM_RX, SDM_TX);
#else
SDM sdm(SDM_PORT, 4800, SDM_DERE, SERIAL_8N1, SDM_RX_RO, SDM_TX_DI);
#endif

void set_accumulated_wh(uint32_t wh)
{
  char command[64];
  snprintf(command, sizeof(command), "$SK %u", wh);
  
  rapiSender.sendCmd(command, [](int ret)
  {
    if (ret == RAPI_RESPONSE_OK)DBUGLN("SUCCES!");
    else DBUGLN("ERROR!");
  });

}

void set_accumulated_kwh(float kwh)
{
    set_accumulated_wh((uint32_t) round(kwh * 1000.00));
}

void sdm_read_data()
{
    accumulated_kwh = sdm.readVal(SDM_TOTAL_ACTIVE_ENERGY);
    accumulated_wh = accumulated_kwh * 1000.00;
    DBUG("Total Active Energy:   ");
    DBUG(accumulated_kwh, 2);                             
    DBUG("KWH, ");
    DBUG(accumulated_wh);                            
    DBUGLN("WH, ");

    if (accumulated_kwh == NAN) return;

    DBUG(" RAPI -> Set accumulated Wh : ");
    set_accumulated_kwh(accumulated_kwh); 
}

void update_read_timer()
{
    readValuesTimer = millis(); 
}

void sdm_meter_begin()
{
    DBUGLN("SDM Meter Init!");
    sdm.begin();
    update_read_timer();
}

void sdm_meter_loop()
{
    if (millis() - readValuesTimer < READ_SAMPLE_RATE_MS) return;
    update_read_timer();
    sdm_read_data();
}

#endif 