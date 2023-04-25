#include <TrinketHidCombo.h>

boolean pressButtons = true;
boolean debug = false;
unsigned long lastActionTime = millis();

void setup()
{
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, INPUT);
  setupKeyBoard();
}

void setupKeyBoard()
{
  digitalWrite(1, HIGH);
  TrinketHidCombo.begin();
  while (TrinketHidCombo.isConnected() == 0)
  {
    TrinketHidCombo.poll();
    delay(100);
  }
  digitalWrite(1, LOW);
}

void loop()
{
  TrinketHidCombo.poll();
  int initialPulse = 0;

  // waiting for initial pulse (9ms on, 4.5ms off)
  while (true)
  {
    initialPulse = pulseIn(2, HIGH, 20000);
    if (initialPulse > 4250 && initialPulse < 5250)
      break;
    digitalWrite(1, HIGH);
    TrinketHidCombo.poll(); // this will get stuck if something gets wrong (pin 1 will be HIGH)
    digitalWrite(1, LOW);
  }

  // reading length of next n pulses
  int n = 32;
  int x[n];
  digitalWrite(1, HIGH);
  for (int i = 0; i < n; i++)
  {
    x[i] = pulseIn(2, HIGH, 20000);
    if (x[i] == 0)
      break;
    TrinketHidCombo.poll();
  }
  digitalWrite(1, LOW);

  // reading data from pulses lengths
  int nrUrzadzeniaH = naInt(x, 0, 8);
  int nrUrzadzeniaL = naInt(x, 8, 16);
  int nrPrzycisku = naInt(x, 16, 24);
  int nrPrzyciskuNeg = naInt(x, 24, 32);
  doAction(nrUrzadzeniaH, nrUrzadzeniaL, nrPrzycisku, nrPrzyciskuNeg);
}

int naInt(int x[], int l, int p)
{
  int out = 0;
  for (int i = l; i < p; i++)
  {
    out *= 2;
    if (x[i] > 1000)
      out++;
  }
  return out;
}

void doAction(int nrUrzadzeniaH, int nrUrzadzeniaL, int nrPrzycisku, int nrPrzyciskuNeg)
{
  if (nrPrzycisku + nrPrzyciskuNeg != 255)
    return;
  if (nrUrzadzeniaH != 0 || nrUrzadzeniaL != 12)
  {
    notRecognisedButton(nrUrzadzeniaH, nrUrzadzeniaL, nrPrzycisku, nrPrzyciskuNeg);
    return;
  }
  if(!pressButtons)
  {
    notRecognisedButton(nrUrzadzeniaH, nrUrzadzeniaL, nrPrzycisku, nrPrzyciskuNeg);
    return;
  }
  if(millis()-lastActionTime < 200) 
  {
    delay(10);
    return;
  }
  lastActionTime = millis();
  switch (nrPrzycisku)
  {
  case 4:
    pressButton(KEYCODE_F4, KEYCODE_MOD_LEFT_ALT);
    break;
  case 64:
    pressSystemCtrlButton(SYSCTRLKEY_SLEEP);
    break;
  case 240:
    pressButton(KEYCODE_ESC);
    break;
 case 16:
    pressButton(KEYCODE_TAB, KEYCODE_MOD_LEFT_ALT, 1000);
    break;
  case 208:
    pressButton(KEYCODE_ARROW_UP);
    break;
  case 48:
    pressButton(KEYCODE_ARROW_DOWN);
    break;
  case 176:
    pressButton(KEYCODE_ARROW_LEFT);
    break;
  case 112:
    pressButton(KEYCODE_ARROW_RIGHT);
    break;
  case 8:
    pressButton(KEYCODE_ENTER);
    break;
  case 224:
    pressButton(KEYCODE_TAB, KEYCODE_MOD_LEFT_SHIFT);
    break;
  case 32:
    pressButton(KEYCODE_TAB);
    break;
  case 12:
    pressButton(KEYCODE_SPACE);
    break;
  case 200:
    pressButton(KEYCODE_F);
    break;
  case 65:
    pressMultimediaButton(MMKEY_VOL_UP);
    break;
  case 193:
    pressMultimediaButton(MMKEY_VOL_DOWN);
    break;
  case 40:
    pressButton(KEYCODE_M);
    break;
  case 89:
    pressButton(KEYCODE_C);
    break;
  default:
    notRecognisedButton(nrUrzadzeniaH, nrUrzadzeniaL, nrPrzycisku, nrPrzyciskuNeg);
  }
}

void pressButton(uint8_t keycode)
{
  TrinketHidCombo.pressKey(0, keycode);
  TrinketHidCombo.pressKey(0, 0);
}

void pressButton(uint8_t keycode, uint8_t modifiers)
{
  TrinketHidCombo.pressKey(modifiers, keycode);
  TrinketHidCombo.pressKey(0, 0);
}

void pressButton(uint8_t keycode, uint8_t modifiers, uint8_t dalayTime)
{
  TrinketHidCombo.pressKey(modifiers, keycode);
  for (int i=0;i<dalayTime/100;i++)
  {
    TrinketHidCombo.poll();
    delay(100);
  }
  TrinketHidCombo.pressKey(0, 0);
}

void pressMultimediaButton(uint8_t keycode)
{
  TrinketHidCombo.pressMultimediaKey(keycode);
  TrinketHidCombo.pressKey(0, 0);
}

void pressSystemCtrlButton(uint8_t keycode)
{
  TrinketHidCombo.pressSystemCtrlKey(keycode);
  TrinketHidCombo.pressKey(0, 0);
}

void notRecognisedButton(int nrUrzadzeniaH, int nrUrzadzeniaL, int nrPrzycisku, int nrPrzyciskuNeg)
{
  if(debug)
  {
    wypiszInt(nrUrzadzeniaH);
    TrinketHidCombo.typeChar(' ');
    wypiszInt(nrUrzadzeniaL);
    TrinketHidCombo.typeChar(' ');
    wypiszInt(nrPrzycisku);
    TrinketHidCombo.typeChar('\n');
  }
}

void wypiszInt(int x)
{
  TrinketHidCombo.typeChar('i');
  if (x < 0)
  {
    TrinketHidCombo.typeChar('-');
    x *= -1;
  }
  if (x == 0)
    TrinketHidCombo.typeChar('0');

  int cyfry[7];
  int index = 0;
  while (x > 0)
  {
    cyfry[index++] = '0' + x % 10;
    x /= 10;
  }
  for (int i = index - 1; i >= 0; i--)
    TrinketHidCombo.typeChar(cyfry[i]);
}
