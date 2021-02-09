ByteBlock Critical_Anim01 = {
  B00000000,
  B01100110,
  B01100110,
  B00000000,
  B00000000,
  B00111100,
  B01000010,
  B00000000
};

ByteBlock Critical_Anim02 = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
};

int Critical_Anim_Count = 10;
int Critical_Anim_Delays[] = { 50, 50, 50, 50, 50, 50, 50, 50, 500, 500 };
int Critical_Anim_Brightness[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 0 };

ByteBlock* Critical_Anim_Pointers[] = { &Critical_Anim01, &Critical_Anim01, &Critical_Anim01, &Critical_Anim01, &Critical_Anim01, &Critical_Anim01, &Critical_Anim01, &Critical_Anim01, &Critical_Anim01, &Critical_Anim02 };
