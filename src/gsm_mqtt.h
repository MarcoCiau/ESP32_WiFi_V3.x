#ifndef GSM_MQTT_H
#define GSM_MQTT_H

#ifdef ENABLE_NOFOS_GTWY
#include <Arduino.h>
#include <ArduinoJson.h>

/* NOFOS MQTT API*/

// -------------------------------------------------------------------
// Setup GSM-MQTT Setup
// begin function
// -------------------------------------------------------------------
extern void nofos_mqtt_begin();

// -------------------------------------------------------------------
// Connect Nofos GSM MQTT Client
// Return true if we are connected to an MQTT broker, false if not
// -------------------------------------------------------------------
extern boolean nofos_mqtt_connect();

// -------------------------------------------------------------------
// Perform the background MQTT operations. Must be called in the main
// loop function
// -------------------------------------------------------------------
extern void nofos_mqtt_loop();

// -------------------------------------------------------------------
// Publish values to MQTT
// data: a comma seperated list of name:value pairs to send
// -------------------------------------------------------------------
extern void nofos_mqtt_publish(JsonDocument &data);

// -------------------------------------------------------------------
// Restart the MQTT connection
// -------------------------------------------------------------------
extern void nofos_mqtt_restart();

// -------------------------------------------------------------------
// Return true if we are connected to an MQTT broker, false if not
// -------------------------------------------------------------------
extern boolean nofos_mqtt_connected();

// -------------------------------------------------------------------
// Set MQTT Network Client : wifi or gsm
// net_client_id: the network client ID. 1 = WIFI, 2 = GSM
// -------------------------------------------------------------------
extern void nofos_mqtt_set_network_client(uint8_t net_client_id);

/* GSM Modem SIM800L API */

// -------------------------------------------------------------------
// GSM SIM initialization
// -------------------------------------------------------------------
extern void gsm_modem_init();

// -------------------------------------------------------------------
// GSM SIM restart and reinitialization
// -------------------------------------------------------------------
extern void gsm_modem_restart();

// -------------------------------------------------------------------
// Return true if gsm modem is initialized, false if not
// -------------------------------------------------------------------
extern boolean gsm_modem_is_initialized();

// -------------------------------------------------------------------
// Return true if gsm modem is connected to an GPRS, false if not
// -------------------------------------------------------------------
extern boolean gsm_modem_is_connected();

#endif //ENABLE_NOFOS_GTWY
#endif //GSM_MQTT_H


