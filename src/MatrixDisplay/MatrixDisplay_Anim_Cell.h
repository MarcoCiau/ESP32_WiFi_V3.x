ByteBlock Cell_Confirmed_Anim01 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B00000000,
  B00000000,
  B00000000
};

ByteBlock Cell_Confirmed_Anim02 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B01100000,
  B00000000,
  B00000000
};

ByteBlock Cell_Confirmed_Anim03 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B01100000,
  B00111000,
  B00000000
};

ByteBlock Cell_Confirmed_Anim04 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B01101100,
  B00111000,
  B00000000
};

ByteBlock Cell_Confirmed_Anim05 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000110,
  B01101100,
  B00111000,
  B00000000
};

ByteBlock Cell_Confirmed_Anim06 = {
  B00000000,
  B00000000,
  B00000000,
  B00000110,
  B11000110,
  B01101100,
  B00111000,
  B00000000
};

ByteBlock Cell_Confirmed_Anim07 = {
  B00000000,
  B00000000,
  B00000011,
  B00000110,
  B11000110,
  B01101100,
  B00111000,
  B00000000
};

ByteBlock Cell_Confirmed_Anim08 = {
  B00000000,
  B00000011,
  B00000011,
  B00000110,
  B11000110,
  B01101100,
  B00111000,
  B00000000
};

ByteBlock Cell_Connecting_Anim01 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00011000,
  B00011000,
  B00011000,
  B00011000
};

ByteBlock Cell_Connecting_Anim02 = {
  B00000000,
  B00000000,
  B00000000,
  B00111100,
  B00111100,
  B00011000,
  B00011000,
  B00011000
};

ByteBlock Cell_Connecting_Anim03 = {
  B00000000,
  B00000000,
  B00111100,
  B01000010,
  B01011010,
  B00111100,
  B00011000,
  B00011000
};

ByteBlock Cell_Connecting_Anim04 = {
  B00000000,
  B00011000,
  B01100110,
  B10000001,
  B10011001,
  B01111110,
  B00011000,
  B00011000
};

ByteBlock Cell_Connecting_Anim05 = {
  B00011000,
  B01100110,
  B10000001,
  B00000000,
  B00011000,
  B10011001,
  B01111110,
  B00011000
};

ByteBlock Cell_Connecting_Anim06 = {
  B01100110,
  B10000001,
  B00000000,
  B00000000,
  B00011000,
  B00011000,
  B10011001,
  B01111110
};

ByteBlock Cell_Connecting_Anim07 = {
  B10000001,
  B00000000,
  B00000000,
  B00000000,
  B00011000,
  B00011000,
  B00011000,
  B10011001
};

ByteBlock Cell_Connecting_Anim08 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00011000,
  B00011000,
  B00011000,
  B00011000
};

int Cell_Connecting_Anim_Count = 8;
int Cell_Connecting_Anim_Delays[] = { 50, 50, 50, 50, 50, 50, 50, 500 };
int Cell_Connecting_Anim_Brightness[] = { 10, 10, 10, 10, 10, 10, 10, 10 };

ByteBlock* Cell_Connecting_Anim_Pointers[] = { &Cell_Connecting_Anim01, &Cell_Connecting_Anim02, &Cell_Connecting_Anim03, &Cell_Connecting_Anim04, &Cell_Connecting_Anim05, &Cell_Connecting_Anim06, &Cell_Connecting_Anim07, &Cell_Connecting_Anim08 };


int Cell_Confirmed_Anim_Count = 8;
int Cell_Confirmed_Anim_Delays[] = { 50, 50, 50, 50, 50, 50, 50, 50, 50, 500 };
int Cell_Confirmed_Anim_Brightness[] = { 10, 10, 10, 10, 10, 10, 10, 2 };

ByteBlock* Cell_Confirmed_Anim_Pointers[] = { &Cell_Confirmed_Anim01, &Cell_Confirmed_Anim02, &Cell_Confirmed_Anim03, &Cell_Confirmed_Anim04, &Cell_Confirmed_Anim05, &Cell_Confirmed_Anim06, &Cell_Confirmed_Anim07, &Cell_Confirmed_Anim08 };
