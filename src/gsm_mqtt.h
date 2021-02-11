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

// -------------------------------------------------------------------
// Return true if gsm modem is initialized, false if not
// -------------------------------------------------------------------
extern boolean gsm_mqtt_modem_is_initialized();

// -------------------------------------------------------------------
// Return true if gsm modem is connected to an GPRS, false if not
// -------------------------------------------------------------------
extern boolean gsm_mqtt_modem_is_connected();

// -------------------------------------------------------------------
// Set MQTT Network Client : wifi or gsm
//
// net_client_id: the network client ID. 1 = WIFI, 2 = GSM
// -------------------------------------------------------------------
extern void gsm_mqtt_set_network_client(uint8_t net_client_id = 1);

// -------------------------------------------------------------------
// GSM SIM initialization
// -------------------------------------------------------------------
extern void gsm_mqtt_modem_init();
#endif //ENABLE_SIM800L_MQTT
#endif //GSM_MQTT_H


