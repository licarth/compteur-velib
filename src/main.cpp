#include <Arduino.h>
#include <LedControl.h>
#include <math.h>
#include <mgos.h>

int getdigit(int number, int digit)
{
   return (number / ((int) pow(10, digit)) % 10);
}

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

LedControl lc = LedControl(D7, D5, D6,2);

static const int METEO_SCREEN = 0;
static const int VELIB_SCREEN = 1;

void printNumber(int screen, int start, int v)
{
  int ones;
  int tens;
  int hundreds;

  boolean negative = false;

  if (v < -999 || v > 999)
    return;
  if (v < 0)
  {
    negative = true;
    v = v * -1;
  }
  ones = getdigit(v, 0);
  tens = getdigit(v, 1);
  hundreds = getdigit(v, 2);
  if (negative)
  {
    //print character '-' in the leftmost column
    lc.setChar(screen, start + 3, '-', false);
  }
  else
  {
    //print a blank in the sign column
    lc.setChar(screen, start + 3, ' ', false);
  }
  //Now print the number digit by digit
  lc.setDigit(screen, start + 0, (byte)ones, false);
  if (tens > 0)
    lc.setDigit(screen, start + 1, (byte)tens, false);
  if (hundreds > 0)
    lc.setDigit(screen, start + 2, (byte)hundreds, false);
}

void displayRainLevel(int i, int level)
{
  int byteSign = 0b00000000;
  switch (level)
  {
  case 1:
    byteSign = 0b10000000;
    break;
  case 2:
    byteSign = 0b00001000;
    break;
  case 3:
    byteSign = 0b00001001;
    break;
  case 4:
    byteSign = 0b01001001;
    break;

  default:
    lc.setRow(METEO_SCREEN, 7 - i, 0b10000000);
    break;
  }
  lc.setRow(METEO_SCREEN, 7 - i, byteSign);
}

void screenTest()
{
  lc.shutdown(VELIB_SCREEN, false);
  lc.shutdown(METEO_SCREEN, false);
  lc.setIntensity(VELIB_SCREEN, 5);
  lc.setIntensity(METEO_SCREEN, 5);
  lc.clearDisplay(VELIB_SCREEN);
  lc.clearDisplay(METEO_SCREEN);
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      lc.setRow(i, j, 0b11111111);
    }
  }
  delay(100);
  lc.clearDisplay(VELIB_SCREEN);
  lc.clearDisplay(METEO_SCREEN);
  for (int i = 0; i < 100; i++)
  {
    printNumber(VELIB_SCREEN, 0, i);
    printNumber(VELIB_SCREEN, 4, i);
    delay(10);
  }
  lc.clearDisplay(VELIB_SCREEN);
  lc.clearDisplay(METEO_SCREEN);
  for (int j = 1; j < 5; j++)
  {
    for (int i = 0; i < 8; i++)
    {
      displayRainLevel(i, j);
      delay(10);
    }
  }
  lc.clearDisplay(VELIB_SCREEN);
  lc.clearDisplay(METEO_SCREEN);
}


void setup(void) {
  delay(100);
  printf("Hello, Arduino world!\r\n");
  pinMode(D0, OUTPUT);
  screenTest();
}

extern "C" {
  void displayVelib(int a, int b){
    lc.clearDisplay(VELIB_SCREEN);
    printNumber(VELIB_SCREEN, 0, a);
    printNumber(VELIB_SCREEN, 4, b);
    return;
  }

  void displayRain(char *rainArray){
    displayRainLevel(0, rainArray[0] - '0');
    displayRainLevel(1, rainArray[1] - '0');
    displayRainLevel(2, rainArray[2] - '0');
    displayRainLevel(3, rainArray[3] - '0');
    displayRainLevel(4, rainArray[4] - '0');
    displayRainLevel(5, rainArray[5] - '0');
    displayRainLevel(6, rainArray[6] - '0');
    displayRainLevel(7, rainArray[7] - '0');
  }

  void setIntensity(int screen, int level) {
    lc.setIntensity(screen, level);
  }
  
  void shutdown() {
    lc.shutdown(VELIB_SCREEN,true);
    lc.shutdown(METEO_SCREEN,true);
  }
  void wakeup() {
    lc.shutdown(VELIB_SCREEN,false);
    lc.shutdown(METEO_SCREEN,false);
  }
}
