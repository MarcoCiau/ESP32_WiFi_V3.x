#ifndef GSM_MQTT_H
#define GSM_MQTT_H

#ifdef ENABLE_SIM800L_MQTT
#include <Arduino.h>
#include <ArduinoJson.h>


// -------------------------------------------------------------------
// Setup GSM-MQTT Setup
// begin function
// -------------------------------------------------------------------
extern void gsm_mqtt_begin();

// -------------------------------------------------------------------
// Perform the background MQTT operations. Must be called in the main
// loop function
// -------------------------------------------------------------------
extern void gsm_mqtt_loop();

// -------------------------------------------------------------------
// Publish values to MQTT
//
// data: a comma seperated list of name:value pairs to send
// -------------------------------------------------------------------
extern void gsm_mqtt_publish(JsonDocument &data);

// -------------------------------------------------------------------
// Restart the MQTT connection
// -------------------------------------------------------------------
extern void gsm_mqtt_restart();


// -------------------------------------------------------------------
// Return true if we are connected to an MQTT broker, false if not
// -------------------------------------------------------------------
extern boolean gsm_mqtt_connected();
#endif //ENABLE_SIM800L_MQTT
#endif //GSM_MQTT_H


