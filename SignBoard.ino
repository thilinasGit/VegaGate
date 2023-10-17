/*
  Countdown on a single DMD display
*/

#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include <fonts/Arial14.h>
#include <ESP8266WiFi.h>

const char *ssid = "GateSpot";
const char *password = "12121212";

const int COUNTDOWN_FROM = 6;
int counter = COUNTDOWN_FROM;
bool greeted;
int animatorIndex = 0, aDelay = 0;
int trigger;

SPIDMD dmd(1, 1,15,16,12,0); // DMD controls the entire display
DMD_TextBox box(dmd, 0, 2);  // "box" provides a text box to automatically write to/scroll the display

// the setup routine runs once when you press reset:
void setup() {
  dmd.setBrightness(255);
  dmd.begin();
  WiFi.softAP(ssid, password);
  pinMode(D4, INPUT_PULLUP);
}

// the loop routine runs over and over again forever:
void loop() {
  trigger=0;
  while (trigger<2 && counter == 6) {
    
    if (aDelay >= 10) {
      aDelay = 0;
      box.clear();
      dmd.selectFont(SystemFont5x7);
      int index = 0;
      if (animatorIndex < 4)index = animatorIndex;
      else index = 8 - animatorIndex;
      DMD_TextBox box(dmd, index, 4);
      box.println(" ...");
      animatorIndex++;
      if (animatorIndex >= 8)animatorIndex = 0;

    }

    aDelay++;
    delay(30);
    if(digitalRead(D4)==LOW)
      trigger++;
    else trigger--;

    if (trigger<0) trigger=0;
    
    if (counter == 6 && WiFi.softAPgetStationNum() > 0) {
      if (!greeted) {
        box.clear();
        dmd.selectFont(SystemFont5x7);
        DMD_TextBox box(dmd, 1, 0);
        box.println("Hello Sir");
        delay(1500);
        box.clear();
        counter = 3;
        greeted = true;
      } 
    }else greeted = false;
  }

    if (counter == 6) {
      box.clear();
      dmd.selectFont(SystemFont5x7);
      DMD_TextBox box(dmd, 5, 4);
      box.println(F("STOP"));
      delay(1500);
      box.clear();
    } else {
      box.clear();
      dmd.selectFont(Arial14);
      DMD_TextBox box(dmd, 9 , 2);
      box.print(F("0"));
      box.println(counter);
      delay(1000);
      box.clear();
    }

    counter--;

    if (counter == 0) {
      box.clear();
      dmd.selectFont(Arial14);
      DMD_TextBox box(dmd, 6, 2);
      box.println(F("GO"));
      delay(2000);
      while (!digitalRead(D4)) {
        /// Keep the GO text until vehicle leave
        delay(2000);
      }

      box.clear();
      counter = COUNTDOWN_FROM;

    }
  }
