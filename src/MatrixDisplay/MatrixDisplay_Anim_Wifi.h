ByteBlock Wifi_Confirmed_Anim01 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B00000000,
  B00000000,
  B00000000
};

ByteBlock Wifi_Confirmed_Anim02 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B01100000,
  B00000000,
  B00000000
};

ByteBlock Wifi_Confirmed_Anim03 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B01100000,
  B00111000,
  B00000000
};

ByteBlock Wifi_Confirmed_Anim04 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B01101100,
  B00111000,
  B00000000
};

ByteBlock Wifi_Confirmed_Anim05 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000110,
  B01101100,
  B00111000,
  B00000000
};

ByteBlock Wifi_Confirmed_Anim06 = {
  B00000000,
  B00000000,
  B00000000,
  B00000110,
  B11000110,
  B01101100,
  B00111000,
  B00000000
};

ByteBlock Wifi_Confirmed_Anim07 = {
  B00000000,
  B00000000,
  B00000011,
  B00000110,
  B11000110,
  B01101100,
  B00111000,
  B00000000
};

ByteBlock Wifi_Confirmed_Anim08 = {
  B00000000,
  B00000011,
  B00000011,
  B00000110,
  B11000110,
  B01101100,
  B00111000,
  B00000000
};

int Wifi_Confirmed_Anim_Count = 8;
int Wifi_Confirmed_Anim_Delays[] = { 50, 50, 50, 50, 50, 50, 50, 50, 50, 500 };
int Wifi_Confirmed_Anim_Brightness[] = { 10, 10, 10, 10, 10, 10, 10, 2 };

ByteBlock* Wifi_Confirmed_Anim_Pointers[] = { &Wifi_Confirmed_Anim01, &Wifi_Confirmed_Anim02, &Wifi_Confirmed_Anim03, &Wifi_Confirmed_Anim04, &Wifi_Confirmed_Anim05, &Wifi_Confirmed_Anim06, &Wifi_Confirmed_Anim07, &Wifi_Confirmed_Anim08 };
