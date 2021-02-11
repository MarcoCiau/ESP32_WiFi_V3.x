#ifdef ENABLE_SIM800L_MQTT
#include "emonesp.h"
#include "gsm_mqtt.h"
#include "app_config.h"
#include "divert.h"
#include "input.h"
#include "espal.h"
#include "net_manager.h"
#include "LedManagerTask.h"

#include "openevse.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#define TINY_GSM_MODEM_SIM800

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true

// set GSM PIN, if any
#define GSM_PIN ""

// Your GSM Network APN Settings
const char apn[] = "TM";
const char gprsUser[] = ""; //if any
const char gprsPass[] = ""; //if any

// MQTT Settings
const char* broker = "test.mosquitto.org";
const char* mqttUser = "";//if any
const char* mqttPassword = "";//if any
uint16_t brokerPort = 1883;

static long gsmNextMqttReconnectAttempt = 0;
static unsigned long modemReconnectAttempt  = 0;
static unsigned long gsmMqttRestartTime = 0;
static uint8_t connectionAttempsCounter = 0;
String lastWillMsg = "";

#include <WiFi.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#ifndef MQTT_CONNECT_TIMEOUT
#define MQTT_CONNECT_TIMEOUT (5 * 1000)
#endif // !MQTT_CONNECT_TIMEOUT

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SIM800L_PORT, DEBUG_PORT);
  TinyGsm modem(debugger);
#else
TinyGsm modem(SIM800L_PORT);
#endif

/*  Nofos Network Clients Profiles */
enum NOFOS_NETWORK_PROFILE{ONLY_GSM = 1, GSM_WITH_FALLBACK, ONLY_WIFI, WIFI_WITH_FALLBACK};
uint8_t nofos_net_profile = ONLY_WIFI;

/* Nofos Network Clients : GSM & WiFi */
TinyGsmClient nofos_gsm_client(modem);
WiFiClient nofos_wifi_client;

/* MQTT Client */
PubSubClient mqtt;

uint32_t lastReconnectAttempt = 0;
bool modemIsAvailable = false;



uint8_t load_nofos_net_profile() {
   //TODO: load value from EEPROM
   return ONLY_WIFI;
}

bool save_net_profile(uint8_t profile)
{
  //TODO: save new network profile to the EEPROM
  return true;
}

void init_nofos_network() {

  nofos_net_profile = load_nofos_net_profile();

  switch (nofos_net_profile)
  {
    case ONLY_GSM:
    case GSM_WITH_FALLBACK:
      /* Set MQTT Network Client */
      mqtt.setClient(nofos_gsm_client);
      // Init port baud and GPIO's
      setup_modem();
      // Init SIM800L Module
      sim800l_init();
    break;
    case ONLY_WIFI:
    case WIFI_WITH_FALLBACK:
      /* Set MQTT Network Client */
      mqtt.setClient(nofos_wifi_client);
      break;
    default:
      /* Set MQTT Network Client */
      mqtt.setClient(nofos_wifi_client);
      break;
  }
}

bool config_mqtt_managed_enabled() {
  return true;
}

bool checkSIMCardStatus()
{
  /* CHECK IF SIM CARD is READY */
  if (modem.getSimStatus() != 1)
  {
    DBUGLN("ERROR: SIM Card unplugged or locked!");
    return false;
  }

  return true;
}

// void nofos_net_fallback_handler()
// {

// }

// -------------------------------------------------------------------
// MQTT msg Received callback function:
// Function to be called when msg is received on MQTT subscribed topic
// Used to receive RAPI commands via MQTT
// //e.g to set current to 13A: <base-topic>/rapi/$SC 13
// -------------------------------------------------------------------

void gsm_mqtt_callback(char* topic, byte* payload, unsigned int len)
{
    String topic_str = String(topic);
    DBUGLN("MQTT received:");
    DBUGLN("Topic: " + topic_str);

    String payload_str = String(payload_str);
    DBUGLN("Payload: " + payload_str);

    // If MQTT message is solar PV
    if (topic_str == mqtt_solar){
        solar = atoi((char*)payload_str.c_str());
        DBUGF("solar:%dW", solar);
        divert_update_state();
    }
    else if (topic_str == mqtt_grid_ie){
        grid_ie = atoi((char*)payload_str.c_str());
        DBUGF("grid:%dW", grid_ie);
        divert_update_state();
    }
    else if (topic_str == mqtt_vrms){
        double volts = atof((char*)payload_str.c_str());
        if (volts >= 60.0 && volts <= 300.0) {
        voltage = volts;
        DBUGF("voltage:%.1f", volts);
        OpenEVSE.setVoltage(volts, [](int ret) {
            // Only gives better power calculations so not critical if this fails
        });
        } else {
        DBUGF("voltage:%.1f (ignoring, out of range)", volts);
        }
    }
    // If MQTT message to set divert mode is received
    else if (topic_str == mqtt_topic + "/divertmode/set"){
        byte newdivert = atoi((char*)payload_str.c_str());
        if ((newdivert==1) || (newdivert==2)){
        divertmode_update(newdivert);
        }
    }
    else
    {
        // If MQTT message is RAPI command
        // Detect if MQTT message is a RAPI command e.g to set 13A <base-topic>/rapi/$SC 13
        // Locate '$' character in the MQTT message to identify RAPI command
        int rapi_character_index = topic_str.indexOf('$');
        DBUGVAR(rapi_character_index);
        if (rapi_character_index > 1) 
        {
            DBUGF("Processing as RAPI");
            // Print RAPI command from mqtt-sub topic e.g $SC
            // ASSUME RAPI COMMANDS ARE ALWAYS PREFIX BY $ AND TWO CHARACTERS LONG)
            String cmd = topic_str.substring(rapi_character_index);
            
            if (strlen((char*)payload) > 0)
            {
                // If MQTT msg contains a payload e.g $SC 13. Not all rapi commands have a payload e.g. $GC
                cmd += " "+ payload_str;
            }

            rapiSender.sendCmd(cmd, [](int ret)
            {
                if (RAPI_RESPONSE_OK == ret || RAPI_RESPONSE_NK == ret)
                {
                    String rapiString = rapiSender.getResponse();
                    String mqtt_data = rapiString;
                    String mqtt_sub_topic = mqtt_topic + "/rapi/out";
                    mqtt.publish((char*)mqtt_sub_topic.c_str(), (char*)mqtt_data.c_str());
                }
            });
        }
    }
}

void sim800l_init()
{

  ledManager.setCellMode(true, false);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBUG("Initializing SIM800L modem, wait 10 sec...");
  modemIsAvailable = modem.restart();
  // modem.init();

  if (!modemIsAvailable)
  {
    checkSIMCardStatus();
    DBUGLN(" ERROR: SIM800L unavailable or disconnected!");
    return;
  } 


  DBUGLN("OK: SIM800L ready!");
  String modemInfo = modem.getModemInfo();
  DBUGLN("Modem Info: " + modemInfo);

#if TINY_GSM_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
    modem.simUnlock(GSM_PIN);
  }
#endif


#if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
  // The XBee must run the gprsConnect function BEFORE waiting for network!
  modem.gprsConnect(apn, gprsUser, gprsPass);
#endif

  DBUG("Waiting for network...");
  if (!modem.waitForNetwork()) {
    checkSIMCardStatus();
    DBUGLN(" network fail");
    modemIsAvailable = false;
    return;
  }
  DBUGLN(" success");

  if (modem.isNetworkConnected()) {
    DBUGLN("Network connected");
  }

#if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
    DBUG("Connecting to " + String(apn));
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      DBUGLN(" fail");
      return;
    }
    DBUGLN(" success");

  if (modem.isGprsConnected()) {
    DBUGLN("GPRS connected");
  }
#endif

  ledManager.setCellMode(true, true);

}

void setup_modem()
{
#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
#endif


    /// *****************************************************
    ///  This following initialization is for Lilygo only
/*
    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);
*/
    /// *****************************************************


    // Initialize the indicator as an output
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LED_OFF);

    DBUGLN("Setting up GPRS modem");
    // Set GSM module baud rate
    // TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
    SIM800L_PORT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    delay(3000);

}

void gsm_mqtt_begin()
{
  DBUGLN("Begin GSM-MQTT...");
  // Init port baud and GPIO's
  // setup_modem();
  // Init SIM800L Module
  // sim800l_init();
  // MQTT Broker setup
  // nofos_net_client = &nofos_wifi_client;
  init_nofos_network();
  mqtt.setServer(broker, brokerPort);
  mqtt.setCallback(gsm_mqtt_callback);
  modemReconnectAttempt = millis();
}

boolean gsm_mqtt_connect()
{

    DBUGF("GSM-MQTT Connecting to... %s", broker);

    // Build the last will message
    DynamicJsonDocument willDoc(JSON_OBJECT_SIZE(3) + 60);

    willDoc["state"] = "disconnected";
    willDoc["id"] = ESPAL.getLongId();
    willDoc["name"] = esp_hostname;

    lastWillMsg = "";
    serializeJson(willDoc, lastWillMsg);

    DBUGVAR(lastWillMsg);

    boolean connectStatus = mqtt.connect(esp_hostname.c_str(), mqtt_user.c_str(), mqtt_pass.c_str(), mqtt_announce_topic.c_str(), MQTTQOS1, false, lastWillMsg.c_str());

    if (connectStatus)
    {
        DBUGLN("GSM-MQTT Connected");

        DynamicJsonDocument doc(JSON_OBJECT_SIZE(5) + 200);

        doc["state"] = "connected";
        doc["id"] = ESPAL.getLongId();
        doc["name"] = esp_hostname;
        doc["mqtt"] = mqtt_topic;
        doc["http"] = "http://"+ipaddress+"/";

        // Once connected, publish an announcement..
        String announce = "";
        serializeJson(doc, announce);

        DBUGLN("Announcing to " + String(mqtt_announce_topic));

        DBUGVAR(announce);
        mqtt.publish((char*)mqtt_announce_topic.c_str(), (char*)announce.c_str(), true);

        // MQTT Topic to subscribe to receive RAPI commands via MQTT
        String mqtt_sub_topic = mqtt_topic + "/rapi/in/#";

        // e.g to set current to 13A: <base-topic>/rapi/in/$SC 13
        mqtt.subscribe((char*)mqtt_sub_topic.c_str());

        if (config_divert_enabled())
        {
            if (mqtt_solar!="") mqtt.subscribe((char*)mqtt_solar.c_str());
            

            if (mqtt_grid_ie!="") mqtt.subscribe((char*)mqtt_grid_ie.c_str());
        }

        if (mqtt_vrms!="") mqtt.subscribe((char*)mqtt_vrms.c_str());

        mqtt_sub_topic = mqtt_topic + "/divertmode/set";      // MQTT Topic to change divert mode
        mqtt.subscribe((char*)mqtt_sub_topic.c_str());

        connectionAttempsCounter = 0;
    }
    else
    {
      DBUGLN("GSM-MQTT is NOT connected.");
      return false;
    }


    connectionAttempsCounter++;
    return true;
}

void gsm_mqtt_publish(JsonDocument &data)
{
    Profile_Start(gsm_mqtt_publish);
    if(!config_mqtt_managed_enabled() || !mqtt.connected()) {
        return;
    }

    JsonObject root = data.as<JsonObject>();
    for (JsonPair kv : root) 
    {
        String topic = mqtt_topic + "/";
        topic += kv.key().c_str();
        String val = kv.value().as<String>();
        mqtt.publish((char*)topic.c_str(), (char*)val.c_str());
        topic = mqtt_topic + "/";
    }

  Profile_End(gsm_mqtt_publish, 5);


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
}

void gsm_mqtt_loop()
{
    Profile_Start(gsm_mqtt_loop);
    /* Try reconnect the modem once each hour */
    if (!modemIsAvailable && (millis() - modemReconnectAttempt < 60*60*1000)) return;
    modemReconnectAttempt = millis();
    if (!modem.isGprsConnected() || (connectionAttempsCounter > 10))
    {
      checkSIMCardStatus();
      connectionAttempsCounter = 0;
      sim800l_init();
    }

    /* If the modem is not connected, don't try to connect it to the mqtt broker!  */
    if (!modemIsAvailable && !modem.isGprsConnected()) return;

    // Restart MQTT connection is required?
    if (gsmMqttRestartTime > 0 && millis() > gsmMqttRestartTime)
    {
        gsmMqttRestartTime = 0;

        if (mqtt.connected())
        {
            DBUGF("Disconnecting GSM_MQTT");
            mqtt.disconnect();
        }

        gsmNextMqttReconnectAttempt = 0;
    }

    if (config_mqtt_managed_enabled() && !mqtt.connected())
    {
        unsigned long now = millis();
        // try and reconnect every x seconds
        if (millis() - gsmNextMqttReconnectAttempt > 10000L) 
        {
            DBUGF("Trying GSM_MQTT Connect...\n");
            gsmNextMqttReconnectAttempt = now;
            // gsm_mqtt_connect(); // Attempt to reconnect
            if (gsm_mqtt_connect()) {
              gsmNextMqttReconnectAttempt = 0;
            }
        }
      delay(10);
      return;
    }
    mqtt.loop();
    Profile_End(mqtt_loop, 5);
}

void gsm_mqtt_restart()
{
  // If connected disconnect MQTT to trigger re-connect with new details
  gsmMqttRestartTime = millis();
}

boolean gsm_mqtt_connected()
{
  return mqtt.connected();
}

#endif
