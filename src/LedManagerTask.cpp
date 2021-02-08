#ifndef ENABLE_DEBUG_LED
#undef ENABLE_DEBUG
#endif

#include "MatrixDisplay/MatrixDisplay.h"

#include <Arduino.h>
#include <openevse.h>

#ifdef ESP32
#include <analogWrite.h>
#endif

#include "debug.h"
#include "emonesp.h"
#include "LedManagerTask.h"

#define FADE_STEP         16
#define FADE_DELAY        50

#define CONNECTING_FLASH_TIME 450
#define CONNECTED_FLASH_TIME  250

#define TEST_LED_TIME     500



LedManagerTask::LedManagerTask() :
  MicroTasks::Task(),
  state(LedState_Off),
  evseState(OPENEVSE_STATE_STARTING)
{
}

void LedManagerTask::setup()
{
  DEBUG.printf("Ledmanager initialized.\n");
}

unsigned long LedManagerTask::loop(MicroTasks::WakeReason reason)
{
  DBUG("LED manager woke: ");
  DBUG(WakeReason_Scheduled == reason ? "WakeReason_Scheduled" :
       WakeReason_Event == reason ? "WakeReason_Event" :
       WakeReason_Message == reason ? "WakeReason_Message" :
       WakeReason_Manual == reason ? "WakeReason_Manual" :
       "UNKNOWN");
  DBUG(" ");
  DBUG(LedState_Off == state ? "LedState_Off" :
         LedState_Test_Red == state ? "LedState_Test_Red" :
         LedState_Test_Green == state ? "LedState_Test_Green" :
         LedState_Test_Blue == state ? "LedState_Test_Blue" :
         LedState_Unknown == state ? "LedState_Unknown" :
         LedState_Ready == state ? "LedState_Ready" :
         LedState_Connected == state ? "LedState_Connected" :
         LedState_Charging == state ? "LedState_Charging" :
         LedState_Sleeping == state ? "LedState_Sleeping" :
         LedState_Warning == state ? "LedState_Warning" :
         LedState_Error == state ? "LedState_Error" :
         LedState_WiFi_Access_Point_Waiting == state ? "LedState_WiFi_Access_Point_Waiting" :
         LedState_WiFi_Access_Point_Connected == state ? "LedState_WiFi_Access_Point_Connected" :
         LedState_WiFi_Client_Connected == state ? "LedState_WiFi_Client_Connected" :
         "UNKNOWN");

  switch(state)
  {
    case LedState_Off:
//      setAllRGB(0, 0, 0);
      return MicroTask.Infinate;

    case LedState_Test_Red:
      //      setAllRGB(255, 0, 0);
      state = LedState_Test_Green;
      return TEST_LED_TIME;

    case LedState_Test_Green:
      //      setAllRGB(0, 255, 0);
      state = LedState_Test_Blue;
      return TEST_LED_TIME;

    case LedState_Test_Blue:
      //      setAllRGB(0, 0, 255);
      state = LedState_Off;
      setNewState(false);
      return TEST_LED_TIME;

    case LedState_Unknown:
      //      setAllRGB(255, 255, 255);
      return MicroTask.Infinate;

    case LedState_Ready:
      //      setAllRGB(0, 255, 0);
      return MicroTask.Infinate;

    case LedState_Connected:
      //      setAllRGB(255, 255, 0);
      return MicroTask.Infinate;

    case LedState_Charging:
      //      setAllRGB(0, 255, 255);
      return MicroTask.Infinate;

    case LedState_Sleeping:
      //      setAllRGB(255, 0, 255);
      return MicroTask.Infinate;

    case LedState_Warning:
      //      setAllRGB(255, 255, 0);
      return MicroTask.Infinate;

    case LedState_Error:
      //      setAllRGB(255, 0, 0);
      return MicroTask.Infinate;

    case LedState_CriticalError:
      updateMatrix3();
      return MicroTask.Infinate;

    case LedState_WiFi_Access_Point_Waiting:
      //      setAllRGB(flashState ? 255 : 0, 
//                flashState ? 255 : 0,
//                0);
      flashState = !flashState;
      return CONNECTING_FLASH_TIME;

    case LedState_WiFi_Access_Point_Connected:
//      setAllRGB(0, flashState ? 255 : 0, 0);
      flashState = !flashState;
      return CONNECTED_FLASH_TIME;

    case LedState_WiFi_Client_ConnectedConfirmed:
      DEBUG.printf("LedManager CONNECTION CONFIRMED state\n");
      updateMatrix2();
      return MicroTask.Infinate;

    case LedState_WiFi_Client_Connecting:
      DEBUG.printf("LedManager CONNECTING state\n");
      updateMatrix();
      return MicroTask.Infinate;

    case LedState_WiFi_Client_Connected:
      DEBUG.printf("LedManager CONNECTED state\n");
      return MicroTask.Infinate;

  }

  return MicroTask.Infinate;
}


LedState LedManagerTask::ledStateFromEvseState(uint8_t state) 
{
  switch(state)
  {
    case OPENEVSE_STATE_STARTING:
      return LedState_Unknown;

    case OPENEVSE_STATE_NOT_CONNECTED:
      return LedState_Ready;

    case OPENEVSE_STATE_CONNECTED:
      return LedState_Connected;

    case OPENEVSE_STATE_CHARGING:
      return LedState_Charging;

    case OPENEVSE_STATE_VENT_REQUIRED:
    case OPENEVSE_STATE_DIODE_CHECK_FAILED:
    case OPENEVSE_STATE_GFI_FAULT:
    case OPENEVSE_STATE_NO_EARTH_GROUND:
    case OPENEVSE_STATE_STUCK_RELAY:
    case OPENEVSE_STATE_GFI_SELF_TEST_FAILED:
    case OPENEVSE_STATE_OVER_TEMPERATURE:
    case OPENEVSE_STATE_OVER_CURRENT:
      return LedState_Error;

    case OPENEVSE_STATE_SLEEPING:
    case OPENEVSE_STATE_DISABLED:
      return LedState_Sleeping;

    case 99:
      return LedState_CriticalError;
  }

  return LedState_Off;
}


int LedManagerTask::getPriority(LedState priorityState)
{
  switch (priorityState)
  {
    case LedState_Off:
      return 0;

    case LedState_Unknown:
    case LedState_Ready:
    case LedState_Connected:
    case LedState_WiFi_Client_Connected:
    case LedState_Charging:
    case LedState_Sleeping:
    case LedState_Warning:
      return 50;

    case LedState_WiFi_Access_Point_Waiting:
    case LedState_WiFi_Access_Point_Connected:
    case LedState_WiFi_Client_Connecting:
    case LedState_WiFi_Client_ConnectedConfirmed:
      return 100;

    case LedState_Error:
    case LedState_CriticalError:
      return 1000;
      
    case LedState_Test_Red:
    case LedState_Test_Green:
    case LedState_Test_Blue:
      return 2000;
  }

  return 0;
}

void LedManagerTask::setNewState(bool wake)
{
  DEBUG.printf("LedManager setNewState\n");
  DEBUG.printf("wake: %d \n", wake);

  DEBUG.printf("state was: %d \n", state);


  LedState newState = state < LedState_Off ? state : LedState_Off;
  int priority = getPriority(newState);

  DEBUG.printf("newState: %d \n", newState);
  DEBUG.printf("newState priority: %d \n", priority);


  LedState evseState = ledStateFromEvseState(this->evseState);
  int evsePriority = getPriority(evseState);
  if(evsePriority >= priority) {
    newState = evseState;
    priority = evsePriority;
  }

  DEBUG.printf("evseState: %d \n", evseState);
  DEBUG.printf("evsePriority priority: %d \n", evsePriority);

  DEBUG.printf("newState: %d \n", newState);
  DEBUG.printf("newState priority: %d \n", priority);



  LedState wifiState;

  if (wifiClient)
  {
    if (wifiConnected)
      wifiState = LedState_WiFi_Client_ConnectedConfirmed;
    else
      wifiState = LedState_WiFi_Client_Connecting;
  }
  else
  {
    if (wifiConnected)
      wifiState = LedState_WiFi_Access_Point_Connected;
    else
      wifiState = LedState_WiFi_Access_Point_Waiting;
  }

  int wifiPriority = getPriority(wifiState);
  if(wifiPriority >= priority) {
    newState = wifiState;
    priority = wifiPriority;
  }

  DEBUG.printf("wifiState: %d \n", wifiState);
  DEBUG.printf("wifiPriority priority: %d \n", wifiPriority);

  DEBUG.printf("newState: %d \n", newState);
  DEBUG.printf("newState priority: %d \n", priority);



  if(newState != state)
  {
    if ((state == LedState_WiFi_Client_Connecting) && (newState == LedState_WiFi_Client_Connected))
      newState = LedState_WiFi_Client_ConnectedConfirmed;

    state = newState;

    if(wake) {
      DEBUG.printf("Wakestate: %d \n", state);

      MicroTask.wakeTask(this);
    }
  }
}

void LedManagerTask::clear()
{
  state = LedState_Off;
  MicroTask.wakeTask(this);
}

void LedManagerTask::test()
{
  state = LedState_Test_Red;
  MicroTask.wakeTask(this);
}

void LedManagerTask::setEvseState(uint8_t newState)
{
  DBUGVAR(newState);
  evseState = newState;
  setNewState();
}

void LedManagerTask::setWifiMode(bool client, bool connected)
{
  DBUGF("WiFi mode %d %d", client, connected);
  wifiClient = client;
  wifiConnected = connected;

  setNewState();
}

LedManagerTask ledManager;
