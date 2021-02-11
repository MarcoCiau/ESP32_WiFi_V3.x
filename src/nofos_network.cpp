#include "nofos_network.h"
#include "gsm_mqtt.h"
#include "net_manager.h"

#define GSM_MODEM_CONNECT_ATTEMP_TIMEOUT 60000 /* Try connect GSM module every 60 secs*/
#define GSM_MODEM_MAX_CONNECT_ATTEMPS 10 /* GSM Module MAX connections attemps  */
#define WIFI_STATUS_SUPERVISOR_TIMEOUT 10000 /* Get WiFi Status every 10 secs*/
#define NET_MAX_CONNECT_ATTEMP_BEFORE_FALLBACK 10 /* WiFi/GSM connections attemps before to switch into fallback mode */


/*  Nofos Network Clients Profiles */
enum NOFOS_NETWORK_PROFILE{ONLY_GSM = 1, GSM_WITH_FALLBACK, ONLY_WIFI, WIFI_WITH_FALLBACK};
enum NOFOS_NETWORK_STATUS {IDLE = 1, WIFI_AS_FALLBACK = 2, GSM_AS_FALLBACK};

uint8_t nofos_current_profile = ONLY_WIFI;
uint8_t nofos_net_status = IDLE;


static uint8_t gsmConnectionAttempsCounter = 0;
static uint8_t wifiConnectionAttempsCounter = 0;
static unsigned long wifiStatusSupervisorTimer  = 0;


uint8_t nofos_network_load_profile() {
   //TODO: load value from EEPROM
   return ONLY_WIFI;
}

bool nofos_network_save_profile(uint8_t profile)
{
  //TODO: save new network profile to the EEPROM
  return true;
}

void nofos_network_profile_init() {

  nofos_current_profile = nofos_network_load_profile();

  switch (nofos_current_profile)
  {
    case ONLY_GSM:
    case GSM_WITH_FALLBACK:
    /* Set MQTT Network Client */
    gsm_mqtt_set_network_client(2);
    /* Init GSM Module*/
    gsm_mqtt_modem_init();
    break;
    case ONLY_WIFI:
    case WIFI_WITH_FALLBACK:
    /* Set MQTT Network Client */
    gsm_mqtt_set_network_client(1);
      break;
    default:
      /* Set MQTT Network Client */
      gsm_mqtt_set_network_client(1);
      break;
  }
}

void nofos_network_begin {
    nofos_network_profile_init();
    gsm_mqtt_begin();
}

void nofos_network_loop()
{

}

void set_wifi_as_fallback()
{
  mqtt.setClient(nofos_wifi_client);
  nofos_net_status = WIFI_AS_FALLBACK;
}

boolean nofos_gsm_fallback_hdler() {
/* GSM Module Reconnection Logic :
  1. If the GSM is not available or disconnected, try to connect every 60 secs
  2. In ONLY_GSM profile, After 10 connection attemps, we should init and restart the gsm module connection
  3. In GSM_WITH_FALLBACK profile, if there is GSM connections troubles,  we will try reconnect to it   
  3. In GSM_WITH_FALLBACK profile, after 10 GSM connection attemps, we WILL switch to WiFi fallback
  4. In GSM_WITH_FALLBACK profile, if there is WiFi connections troubles,  we will try reconnect to it  
  5. In GSM_WITH_FALLBACK profile, after 10 WiFi Connection attemps, we WILL back to GSM  
*/

  if (nofos_net_profile != ONLY_GSM && nofos_net_profile != GSM_WITH_FALLBACK) return;

  /* Try reconnect the modem once each 60 secs */
  if (!modem_is_initialized && (millis() - modemReconnectAttempt < GSM_MODEM_CONNECT_ATTEMP_TIMEOUT)) return;
  modemReconnectAttempt = millis();

  if (!modem.isGprsConnected())
  {
    if (nofos_net_profile == ONLY_GSM && gsmConnectionAttempsCounter > GSM_MODEM_MAX_CONNECT_ATTEMPS)
    {
      checkSIMCardStatus();
      sim800l_init();
      gsmConnectionAttempsCounter = 0;
    }
    else if (nofos_net_profile == GSM_WITH_FALLBACK && gsmConnectionAttempsCounter > NET_MAX_CONNECT_ATTEMP_BEFORE_FALLBACK)
    {
      //TODO: start gsm fallback
      gsmConnectionAttempsCounter = 0;
      run_wifi_as_fallback();
    }
  }

  /* If the modem is not connected, don't try to connect it to the mqtt broker!  */
  // if (!modem_is_initialized && !modem.isGprsConnected()) return;
}

boolean nofos_wifi_fallback_hdler()
{
/* WiFi Reconnection Logic :
  1. Due the WiFi Connection attemps is handled by the OpenEVSE core, we will get the wifi status each 10 secs
  2. In ONLY_WIFI profile, we will ONLY get the wifi status from the OpenEVESE core each 10 secs
  3. In WIFI_WITH_FALLBACK profile, if there is WiFi connections troubles,  we will try reconnect to it   
  3. In WIFI_WITH_FALLBACK profile, after 10 WiFi connection attemps, we WILL switch to GSM fallback
  4. In WIFI_WITH_FALLBACK profile, if there is GSM connections troubles,  we will try reconnect to it  
  5. In WIFI_WITH_FALLBACK profile, after 10 GSM Connection attemps, we WILL back to WIFI  
*/

  if (nofos_net_profile != ONLY_WIFI && nofos_net_profile != WIFI_WITH_FALLBACK) return;
  /* Check WiFi Status once each 10 secs */
  if (millis() - wifiStatusSupervisorTimer < WIFI_STATUS_SUPERVISOR_TIMEOUT) return;
  wifiStatusSupervisorTimer = millis();

  // if (nofos_net_profile == ONLY_WIFI) /* In WIFI_ONLY we have not to do something, the openEVSE core will handle the WiFi Connection */

  if (nofos_net_profile == WIFI_WITH_FALLBACK)
  {
    if (net_is_connected()) wifiConnectionAttempsCounter = 0;
    else wifiConnectionAttempsCounter++;
    if (wifiConnectionAttempsCounter > NET_MAX_CONNECT_ATTEMP_BEFORE_FALLBACK)
    {
      /* Execute Fallback*/
      wifiConnectionAttempsCounter = 0;
    }
  }
}
