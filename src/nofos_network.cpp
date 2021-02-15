#ifndef ENABLE_NOFOS_GTWY
#undef ENABLE_NOFOS_GTWY
#endif
#ifdef ENABLE_NOFOS_GTWY
#include "nofos_network.h"
#include "gsm_mqtt.h"
#include "net_manager.h"
#include "debug.h"
#include "emonesp.h"

#define GSM_MODEM_CONNECT_NEXT_ATTEMPT_TIMEOUT 60000 /* Try connect GSM module every 60 secs*/
#define MQTT_STATUS_SUPERVISOR_TIMEOUT 10000 /* Get MQTT Client status every 10 secs*/
#define WIFI_STATUS_SUPERVISOR_TIMEOUT 10000 /* Get WiFi Status every 10 secs*/
#define NET_MAX_CONNECT_ATTEMPTS 10 /* WiFi-GSM connections attemps before to switch into fallback mode */

/*  Nofos Network Clients Profiles */
enum NOFOS_NETWORK_PROFILE{ ONLY_GSM = 1, GSM_WITH_FALLBACK, ONLY_WIFI, WIFI_WITH_FALLBACK};
enum NOFOS_NETWORK_STATE { WIFI_AS_MAIN_NETWORK = 1, GSM_AS_MAIN_NETWORK};

uint8_t nofos_current_profile = ONLY_WIFI;
uint8_t nofos_net_status = WIFI_AS_MAIN_NETWORK;

/* MQTT, GSM & WiFi Connections attemps counters */
static uint8_t mqttConnectionAttempsCounter = 0; 
static uint8_t gsmConnectionAttempsCounter = 0;
static uint8_t wifiConnectionAttempsCounter = 0;

/* Timer Counters */
static unsigned long gsmModemConnectionAttempTimer = 0;
static unsigned long wifiStatusSupervisorTimer  = 0;
static unsigned long mqttStatusSupervisorTimer  = 0;

void debug_nofos_network_profile(boolean debug)
{
  if (!debug) return;
 switch (nofos_current_profile)
 {
 case ONLY_GSM:
   DBUGLN("Nofos Network Profile: Only GSM");
   break;
 case ONLY_WIFI:
   DBUGLN("Nofos Network Profile: Only WiFi");
   break;
 case GSM_WITH_FALLBACK:
   DBUGLN("Nofos Network Profile: GSM with Fallback");
   break;
 case WIFI_WITH_FALLBACK:
   DBUGLN("Nofos Network Profile: WiFi with Fallback");
   break;
 }
}

void set_wifi_as_main_network()
{
  DBUGLN("\nINFO: GSM Network unavailable. Switch into WiFi Network...\n");
  nofos_mqtt_set_network_client(1);
  nofos_net_status = WIFI_AS_MAIN_NETWORK;
}

void set_gsm_as_main_network()
{
  DBUGLN("\nINFO: WiFi Network unavailable. Switch into GSM Network...\n");
  nofos_mqtt_set_network_client(2);
  nofos_net_status = GSM_AS_MAIN_NETWORK;
}

uint8_t nofos_network_load_profile() {
   //TODO: load value from EEPROM
   return GSM_WITH_FALLBACK;
}

bool nofos_network_save_profile(uint8_t profile)
{
  //TODO: save new network profile to the EEPROM
  return true;
}

void nofos_network_profile_init() {

  nofos_current_profile = nofos_network_load_profile();
  debug_nofos_network_profile(true);
  switch (nofos_current_profile)
  {
    case ONLY_GSM:
    case GSM_WITH_FALLBACK:
    /* Set Nofos Network Client */
    set_gsm_as_main_network();
    /* Init GSM Module*/
    gsm_modem_init();
    break;
    case ONLY_WIFI:
    case WIFI_WITH_FALLBACK:
    /* Set Nofos Network Client */
    set_wifi_as_main_network();
    break;
    default:
    /* Set Nofos Network Client */
    set_wifi_as_main_network();
    break;
  }
}

/* Check if MQTT Client is connected. */
void check_mqtt_client_status()
{
  if (millis() - mqttStatusSupervisorTimer < MQTT_STATUS_SUPERVISOR_TIMEOUT) return; /* Timer isn't expired */
  mqttStatusSupervisorTimer = millis();
  if (nofos_mqtt_connected())
  {
    mqttConnectionAttempsCounter = 0; /* Success */
  } 
  else mqttConnectionAttempsCounter++; /* Error */
}

/* Handle MQTT Connection. After NET_MAX_CONNECT_ATTEMPTS, re-init GSM module*/
void handle_mqtt_connections()
{
  check_mqtt_client_status(); 
  if (mqttConnectionAttempsCounter > NET_MAX_CONNECT_ATTEMPTS )
  {
    DBUGF("\n\nMQTT Connection fail after %d connections, restarting GSM Module...\n\n", NET_MAX_CONNECT_ATTEMPTS);
    gsm_modem_restart(); /* Init and connect GSM Module Again*/
  } 
}

/* Check if gsm and gprs is connected. Init and connect the GSM Module again if required. */
uint8_t handle_gsm_network()
{
  /* 
  1. If the GSM is not available or disconnected, try to connect every 60 secs
  */
  if ((millis() - gsmModemConnectionAttempTimer < GSM_MODEM_CONNECT_NEXT_ATTEMPT_TIMEOUT) ) return 0; /* Timer isn't expired */
  gsmModemConnectionAttempTimer = millis();
  if (!gsm_modem_is_connected()) gsm_modem_restart(); /* Init and connect GSM Module Again*/
  if (nofos_current_profile == ONLY_GSM) handle_mqtt_connections();
  if (gsm_modem_is_initialized() && gsm_modem_is_connected() && nofos_mqtt_connected()) return 1; /* Success */
  else return 2; /* Error */
}

/* Check if wifi is connected. */
uint8_t handle_wifi_network()
{
  /* Get WiFi status once each WIFI_STATUS_SUPERVISOR_TIMEOUT (10 secs) */
  if ((millis() - wifiStatusSupervisorTimer < WIFI_STATUS_SUPERVISOR_TIMEOUT)) return 0; /* Timer isn't expired */
  wifiStatusSupervisorTimer = millis(); 
  if (net_is_connected() == true) return 1; /* Success */
  else return 2; /* Error */
}

void fallback_network_handler()
{
 /* GSM & WiFi Netowrk Reconnection Logic :
  STEP 1. If the GSM is not available or disconnected, try to connect every 60 secs
  STEP 2. In GSM_WITH_FALLBACK profile, if there is GSM connections troubles,  we will try reconnect to it   
  STEP 3. In GSM_WITH_FALLBACK profile, after 10 GSM connection attemps, we WILL switch to WiFi fallback
  STEP 4. In GSM_WITH_FALLBACK profile, if there is WiFi connections troubles,  we will try reconnect to it  
  STEP 5. In GSM_WITH_FALLBACK profile, after 10 WiFi Connection attemps, we will back to GSM  
*/ 
  if (nofos_net_status == GSM_AS_MAIN_NETWORK)
  {
    /* STEP 1 and 2*/
    uint8_t state = handle_gsm_network();
    if (state == 2) gsmConnectionAttempsCounter++;
    else if (state == 1) gsmConnectionAttempsCounter = 0;
    /* STEP 3 */
    if (gsmConnectionAttempsCounter > NET_MAX_CONNECT_ATTEMPTS)
    {
      gsmConnectionAttempsCounter = 0;
      set_wifi_as_main_network();
    }
    /* Execute MQTT if GSM GPRS is connected */
    if (gsm_modem_is_connected()) nofos_mqtt_loop();
  }
  else if (nofos_net_status == WIFI_AS_MAIN_NETWORK)
  {
    /* STEP 4*/
    uint8_t state = handle_wifi_network();
    if (state == 2) wifiConnectionAttempsCounter++;
    else if (state == 1) wifiConnectionAttempsCounter = 0;

    /* STEP 5 */
    if (wifiConnectionAttempsCounter > NET_MAX_CONNECT_ATTEMPTS)
    {
      wifiConnectionAttempsCounter = 0;
      set_gsm_as_main_network();
    }
    /* Execute MQTT if WiFi is connected */
    if (net_is_connected() == true) nofos_mqtt_loop();
  }
}

void only_gsm_network_loop() 
{
  /* 
  1. If the GSM is not available or disconnected, try to connect every 60 secs
  */
  handle_gsm_network();
  if (gsm_modem_is_connected()) nofos_mqtt_loop();
}

void only_wifi_network_loop()
{
  /* WiFi Reconnection Logic :
  1. Due the WiFi Connection attemps is handled by the OpenEVSE core, we will get the wifi status each 10 secs
  2. We will ONLY get the wifi status from the OpenEVESE core each 10 secs
  */
  handle_wifi_network();
  if (net_is_connected() == true) nofos_mqtt_loop();
}

void gsm_with_fallback_network_loop()
{
  fallback_network_handler();
}

void wifi_with_fallback_network_loop()
{
  fallback_network_handler();
}

void nofos_network_begin() 
{
  DBUGLN("Nofos Network Begin...");
  nofos_network_profile_init();
  nofos_mqtt_begin();
  gsmModemConnectionAttempTimer = millis();
}

void nofos_network_loop()
{
  Profile_Start(nofos_mqtt_loop);
  if (nofos_current_profile == ONLY_WIFI) only_wifi_network_loop();
  else if (nofos_current_profile == ONLY_GSM) only_gsm_network_loop();
  else if (nofos_current_profile == WIFI_WITH_FALLBACK) wifi_with_fallback_network_loop();
  else if (nofos_current_profile == GSM_WITH_FALLBACK) gsm_with_fallback_network_loop();
  Profile_End(nofos_mqtt_loop, 5);
}
#endif // ENABLE_NOFOS_GTWY