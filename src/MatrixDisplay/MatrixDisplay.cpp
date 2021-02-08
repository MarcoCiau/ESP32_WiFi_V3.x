#include "MatrixDisplay\LedController.hpp"
#include "MatrixDisplay\MatrixDisplay.h"

LedController<1, 1> lc = LedController<1, 1>();


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
