#ifdef ENABLE_SIM800L_MQTT
#include "emonesp.h"
#include "gsm_mqtt.h"
#include "app_config.h"
#include "divert.h"
#include "input.h"
#include "espal.h"
#include "net_manager.h"

#include "openevse.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#define TINY_GSM_MODEM_SIM800

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true

// set GSM PIN, if any
#define GSM_PIN ""

// Your GSM Network APN Settings
const char apn[] = "internet.mvne1.com";
const char gprsUser[] = ""; //if any
const char gprsPass[] = ""; //if any

// MQTT Settings
const char* broker = "broker.hivemq.com";
const char* mqttUser = "";//if any
const char* mqttPassword = "";//if any
uint16_t brokerPort = 1883;

/* MQTT Topics */
const char* topicLed = "GsmClientTest/led";
const char* topicInit = "GsmClientTest/init";
const char* topicLedStatus = "GsmClientTest/ledStatus";


static long gsmNextMqttReconnectAttempt = 0;
static unsigned long gsmMqttRestartTime = 0;

String lastWillMsg = "";

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


TinyGsmClient client(modem);
PubSubClient mqtt(client);

uint32_t lastReconnectAttempt = 0;

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
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBUGLN("Initializing SIM800L modem...");
  modem.restart();
  // modem.init();

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
    DBUGLN(" fail");
    delay(10000);
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
      delay(10000);
      return;
    }
    DBUGLN(" success");

  if (modem.isGprsConnected()) {
    DBUGLN("GPRS connected");
  }
#endif
}

void gsm_mqtt_begin()
{
  DBUGLN("Begin GSM-MQTT...");
  // Set GSM module baud rate
  SIM800L_PORT.begin(9600);
  delay(1000);
  // Init SIM800L Module
  sim800l_init();
  // MQTT Broker setup
  mqtt.setServer(broker, brokerPort);
  mqtt.setCallback(gsm_mqtt_callback);

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
        DBUGVAR(announce);
        mqtt.publish((char*)mqtt_announce_topic.c_str(), (char*)announce.c_str(), true);
        mqtt.publish(topicInit, "GsmClientTest started");

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
    }
    return true;
}

void gsm_mqtt_publish(JsonDocument &data)
{
    Profile_Start(gsm_mqtt_publish);
    if(!config_mqtt_enabled() || !mqtt.connected()) {
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

void gsm_mqtt_loop()
{
    Profile_Start(gsm_mqtt_loop);

    if (!modem.isGprsConnected())sim800l_init();
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

    if (config_mqtt_enabled() && !mqtt.connected())
    {
        unsigned long now = millis();
        // try and reconnect every x seconds
        if (millis() - gsmNextMqttReconnectAttempt > 10000L) 
        {
            DBUGF("Trying GSM_MQTT Connect\n");
            gsmNextMqttReconnectAttempt = now;
            // gsm_mqtt_connect(); // Attempt to reconnect
            if (gsm_mqtt_connect()) {
              gsmNextMqttReconnectAttempt = 0;
            }
        }
      delay(100);
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