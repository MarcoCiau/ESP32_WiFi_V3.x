#include "MatrixDisplay\LedController.hpp"
#include "MatrixDisplay\MatrixDisplay.h"
#include "MatrixDisplay\MatrixDisplay_Anim_Critical.h"
#include "MatrixDisplay\MatrixDisplay_Anim_Wifi.h"
#include "MatrixDisplay\MatrixDisplay_Anim_Cell.h"
#include "LedManagerTask.h"


LedController<1, 1> lc = LedController<1, 1>();

TaskHandle_t ledManagerTaskHandle;
int matrixDelay;


void ledManagerTask(void* parameter) {
  for (;;) {

    matrixDelay = ledManager.loop(WakeReason_Scheduled);

    if (matrixDelay == MicroTask.Infinate)
      delay(100);
    else
      delay(matrixDelay);
  }
}


void initMatrix()
{
  lc = LedController<1, 1>(23, 18, 19);
  lc.init(1);// Pins: CS, # of Display connected
  lc.setIntensity(0);

  xTaskCreatePinnedToCore(
    ledManagerTask, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &ledManagerTaskHandle,  /* Task handle. */
    0); /* Core where the task should run */
}

void setMatrixIntensity(int intensity)
{
  lc.setIntensity(intensity);
}


int Critical_Anim(int animation)
{
  lc.displayOnSegment(0, *Critical_Anim_Pointers[animation]);
  lc.setIntensity(Critical_Anim_Brightness[animation]);
  return Critical_Anim_Delays[animation];
}


int Wifi_AccessPointWaiting_Anim(int animation)
{
  lc.displayOnSegment(0, *Wifi_AccessPointWaiting_Anim_Pointers[animation]);
  lc.setIntensity(Wifi_AccessPointWaiting_Anim_Brightness[animation]);
  return Wifi_AccessPointWaiting_Anim_Delays[animation];
}


int Wifi_Connecting_Anim(int animation)
{
  lc.displayOnSegment(0, *Cell_Connecting_Anim_Pointers[animation]);
  lc.setIntensity(Cell_Connecting_Anim_Brightness[animation]);
  return Cell_Connecting_Anim_Delays[animation];
}

int Wifi_Confirmed_Anim(int animation)
{
  lc.displayOnSegment(0, *Wifi_Confirmed_Anim_Pointers[animation]);
  lc.setIntensity(Wifi_Confirmed_Anim_Brightness[animation]);
  return Wifi_Confirmed_Anim_Delays[animation];
}



int Cell_Connecting_Anim(int animation)
{
  lc.displayOnSegment(0, *Cell_Connecting_Anim_Pointers[animation]);
  lc.setIntensity(Cell_Connecting_Anim_Brightness[animation]);
  return Cell_Connecting_Anim_Delays[animation];
}


int Cell_Confirmed_Anim(int animation)
{
  lc.displayOnSegment(0, *Cell_Confirmed_Anim_Pointers[animation]);
  lc.setIntensity(Cell_Confirmed_Anim_Brightness[animation]);
  return Cell_Confirmed_Anim_Delays[animation];
}

