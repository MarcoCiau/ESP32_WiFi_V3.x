#include "MatrixDisplay\LedController.hpp"
#include "MatrixDisplay\MatrixDisplay.h"

LedController<1, 1> lc = LedController<1, 1>();

int anim_delay_wifi_frames = 2;
int anim_delay_wifi_connecting = 500;

ByteBlock wifi01 = {
  B00000000,
  B01000010,
  B00000000,
  B00000000,
  B00111000,
  B00010000,
  B00000000,
  B00000000,
};

ByteBlock wifi02 = {
  B00000000,
  B01100110,
  B01100110,
  B00000000,
  B00000000,
  B01000010,
  B00111100,
  B00000000
};

ByteBlock wifi03 = {
  B00000000,
  B01100110,
  B01100110,
  B00000000,
  B00000000,
  B00111100,
  B01000010,
  B00000000
};


bool initMatrix()
{
  lc = LedController<1, 1>(23, 18, 19);
  lc.init(1);// Pins: CS, # of Display connected
  lc.setIntensity(0);

  return true;
}

bool updateMatrix()
{
  lc.displayOnSegment(0, wifi01);

  return true;
}

bool updateMatrix2()
{
  lc.displayOnSegment(0, wifi02);

  return true;
}

bool updateMatrix3()
{
  lc.displayOnSegment(0, wifi03);

  return true;
}
