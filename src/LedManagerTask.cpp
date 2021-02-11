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

uint animationCurrentFrame;
uint animationDelay = 0;


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
      return 100;

    case LedState_Ready:
      //      setAllRGB(0, 255, 0);
      return 100;

    case LedState_Connected:
      //      setAllRGB(255, 255, 0);
      return 100;

    case LedState_Charging:
      //      setAllRGB(0, 255, 255);
      return 100;

    case LedState_Sleeping:
      //      setAllRGB(255, 0, 255);
      return 100;

    case LedState_Warning:
      //      setAllRGB(255, 255, 0);
      return 100;

    case LedState_Error:
      //      setAllRGB(255, 0, 0);
      return 100;

    case LedState_CriticalError:
      animationDelay = Critical_Anim(animationCurrentFrame);

      animationCurrentFrame++;
      if (animationCurrentFrame >= Critical_Anim_Count)
        animationCurrentFrame = 0;

      return animationDelay;

    case LedState_WiFi_Access_Point_Waiting:
      animationDelay = Wifi_AccessPointWaiting_Anim(animationCurrentFrame);
      animationCurrentFrame++;
      if (animationCurrentFrame >= Wifi_AccessPointWaiting_Anim_Count)
        animationCurrentFrame = 0;
      return animationDelay;

    case LedState_WiFi_Access_Point_Connected:
      flashState = !flashState;
      return animationDelay;

    case LedState_WiFi_Client_ConnectedConfirmed:
      animationDelay = Wifi_Confirmed_Anim(animationCurrentFrame);
      animationCurrentFrame++;
      if (animationCurrentFrame >= Wifi_Confirmed_Anim_Count)
        animationCurrentFrame = 0;
      return animationDelay;

    case LedState_WiFi_Client_Connecting:
      animationDelay = Wifi_Connecting_Anim(animationCurrentFrame);
      animationCurrentFrame++;
      if (animationCurrentFrame >= Wifi_Connecting_Anim_Count)
        animationCurrentFrame = 0;
      return animationDelay;

    case LedState_WiFi_Client_Connected:
      return animationDelay;

    case LedState_Cell_Client_ConnectedConfirmed:
      animationDelay = Cell_Confirmed_Anim(animationCurrentFrame);
      animationCurrentFrame++;
      if (animationCurrentFrame >= Cell_Confirmed_Anim_Count)
        animationCurrentFrame = 0;
      return animationDelay;

    case LedState_Cell_Client_Connecting:
      animationDelay = Cell_Connecting_Anim(animationCurrentFrame);
      animationCurrentFrame++;
      if (animationCurrentFrame >= Cell_Connecting_Anim_Count)
        animationCurrentFrame = 0;
      return animationDelay;

    case LedState_Cell_Client_Connected:
      DEBUG.printf("LedManager CONNECTED state\n");
      return animationDelay;

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
    case LedState_Cell_Client_Connected:
    case LedState_Charging:
    case LedState_Sleeping:
    case LedState_Warning:
      return 50;

    case LedState_WiFi_Access_Point_Waiting:
    case LedState_WiFi_Access_Point_Connected:
    case LedState_WiFi_Client_Connecting:
    case LedState_WiFi_Client_ConnectedConfirmed:
    case LedState_Cell_Client_Connecting:
    case LedState_Cell_Client_ConnectedConfirmed:
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

//  DEBUG.printf("state was: %d \n", state);


  LedState newState = state < LedState_Off ? state : LedState_Off;
  int priority = getPriority(newState);

//  DEBUG.printf("newState: %d \n", newState);
//  DEBUG.printf("newState priority: %d \n", priority);


  LedState evseState = ledStateFromEvseState(this->evseState);
  int evsePriority = getPriority(evseState);
  if(evsePriority >= priority) {
    DEBUG.printf("Picked EVSE..\n");

    newState = evseState;
    priority = evsePriority;
  }

//  DEBUG.printf("evseState: %d \n", evseState);
//  DEBUG.printf("evsePriority priority: %d \n", evsePriority);

//  DEBUG.printf("newState: %d \n", newState);
//  DEBUG.printf("newState priority: %d \n", priority);



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
    DEBUG.printf("Picked wifi..\n");

    newState = wifiState;
    priority = wifiPriority;
  }

//  DEBUG.printf("wifiState: %d \n", wifiState);
//  DEBUG.printf("wifiPriority priority: %d \n", wifiPriority);

//  DEBUG.printf("newState: %d \n", newState);
//  DEBUG.printf("newState priority: %d \n", priority);


  LedState cellState;

  if (cellActive)
  {
    if (cellConnected)
      cellState = LedState_Cell_Client_ConnectedConfirmed;
    else
      cellState = LedState_Cell_Client_Connecting;

    int cellPriority = getPriority(cellState);
    if (cellPriority >= priority) {
      DEBUG.printf("Picked cell..\n");

      newState = cellState;
      priority = cellPriority;
    }

  }


  //  DEBUG.printf("wifiState: %d \n", wifiState);
  //  DEBUG.printf("wifiPriority priority: %d \n", wifiPriority);

  //  DEBUG.printf("newState: %d \n", newState);
  //  DEBUG.printf("newState priority: %d \n", priority);


  if(newState != state)
  {
    if ((state == LedState_WiFi_Client_Connecting) && (newState == LedState_WiFi_Client_Connected))
      newState = LedState_WiFi_Client_ConnectedConfirmed;

    if ((state == LedState_Cell_Client_Connecting) && (newState == LedState_Cell_Client_Connected))
      newState = LedState_Cell_Client_ConnectedConfirmed;

    state = newState;

    DEBUG.printf("New state: %d \n", state);

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

void LedManagerTask::setCellMode(bool active, bool connected)
{
  cellActive = active;
  cellConnected = connected;

  setNewState();
}

LedManagerTask ledManager;
