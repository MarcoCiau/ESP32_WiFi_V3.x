#ifdef ENABLE_SDM_METER
#include "sdm_meter.h"
#include "debug.h"
#include <SDM.h>  
#include <SoftwareSerial.h>  

#define READ_SAMPLE_RATE_MS 10000
static unsigned long readValuesTimer = 0;

#ifdef USE_SDM_SOFTWARE_SERIAL
SoftwareSerial swSerSDM;                                                       
SDM sdm(swSerSDM, 4800, SDM_DERE, SWSERIAL_8N1, SDM_RX, SDM_TX);
#else
SDM sdm(Serial2, 4800, SDM_DERE, SERIAL_8N1, false);
#endif

void sdm_read_data()
{
    char bufout[10];
    sprintf(bufout, "%c[1;0H", 27);
    DBUG(bufout);

    DBUG("Voltage:   ");
    DBUG(sdm.readVal(SDM_PHASE_1_VOLTAGE), 2);                            //display voltage
    DBUGLN("V");

    DBUG("Current:   ");
    DBUG(sdm.readVal(SDM_PHASE_1_CURRENT), 2);                            //display current  
    DBUGLN("A");

    DBUG("Power:     ");
    DBUG(sdm.readVal(SDM_PHASE_1_POWER), 2);                              //display power
    DBUGLN("W");

    DBUG("Frequency: ");
    DBUG(sdm.readVal(SDM_FREQUENCY), 2);                                  //display frequency
    DBUGLN("Hz");   
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