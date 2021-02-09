#include "MatrixDisplay\LedController.hpp"
#include "MatrixDisplay\MatrixDisplay.h"
#include "MatrixDisplay\MatrixDisplay_Anim_Critical.h"
#include "MatrixDisplay\MatrixDisplay_Anim_Wifi.h"

LedController<1, 1> lc = LedController<1, 1>();


bool initMatrix()
{
  lc = LedController<1, 1>(23, 18, 19);
  lc.init(1);// Pins: CS, # of Display connected
  lc.setIntensity(0);

  return true;
}


int Critical_Anim(int animation)
{
  lc.displayOnSegment(0, *Critical_Anim_Pointers[animation]);
  lc.setIntensity(Critical_Anim_Brightness[animation]);
  return Critical_Anim_Delays[animation];
}


int Wifi_Confirmed_Anim(int animation)
{
  lc.displayOnSegment(0, *Wifi_Confirmed_Anim_Pointers[animation]);
  lc.setIntensity(Wifi_Confirmed_Anim_Brightness[animation]);
  return Wifi_Confirmed_Anim_Delays[animation];
}

