// Written by Austin Greene on Jan 21, 2019 - based on open source RTClib library examples.
//MOdified from CoralCam_TwiceDailyPhoto_ArduinoCode.ino by Jimmy Manley

// The on-board Pro-Mini LED tied to Pin 13 will be used as an indicator light during mode/power button pushes. 
// A small DS3231 RTC is used. The DS3231 has 2 onboard alarms that can be set, but in my experience only Alarm 1 functions reliably. 
// Instead, we'll programatically shift between alarm times. You can add more alarm times, at the cost of battery life. 
// After each alarm has gone off a flag is set, which in turn sets the next alarm time. 
// Once the last alarm is triggered, all flags are reset and the first alarm time is set once more. 

// For use with Arduino Pro Mini 3.3v and two BC547 transistors acting as an intervalometer for a modified Socoo C30 action camera (or similar). 
// NOTES:
// - If camera behaves erratic or misses photos, slightly extend delays between button pushes, or shorten button pushes.
// - If after wiring only one button on the camera appears to work (Mode/Power or Shutter), swap the position of the mode button wires on the PCB. 

// RTC Wiring
// RTC GND to Pro Mini GND
// RTC C to A5
// RTC D to A4
// RTC VCC to Pro Mini VCC
// RTC INT to D2

// Camera Mode/Power Button
// BC547 gate line (4.7k resistor in-line) to D6
// BC547 drain to Pro Mini GND

// Camera Shutter Button
// BC547 gate line (4.7k resistor in-line) to D5
// BC547 drain to Pro Mini GND

// Libraries for RTC
#include <Wire.h>
#include <SPI.h>   // not used here, but needed to prevent a RTClib compile error
#include <avr/sleep.h>
#include <RTClib.h>

// RTC definitions
RTC_DS3231 rtc;
int INTERRUPT_PIN = 2;
volatile int state = LOW; // Do not remove, raltes to library fxns. 
volatile boolean flag = false;
volatile boolean ALARM1 = false;
volatile int counter = 0;

int h;
int h2;
int m;

void setup () {
// the setup loop runs once when you press reset or power the board

  // initializing camera/led pins 
  pinMode(6, OUTPUT); //power and mode button
  pinMode(5, OUTPUT); //shutter button
  pinMode(13, OUTPUT); //LED indicator
  pinMode(INTERRUPT_PIN, INPUT);
  
  //pull up the interrupt pin
  digitalWrite(INTERRUPT_PIN, HIGH);
  
  Serial.begin(57600);
  rtc.begin();  // Includes a call to Wire.begin()
    
  rtc.adjust(DateTime(__DATE__, __TIME__));
  DateTime now = rtc.now();

  // flash two short times to show it's awake. Power LED has been removed, so using D13 LED. 
  digitalWrite(13, HIGH);
  delay(500); 
  digitalWrite(13, LOW);
  delay(500); 
  digitalWrite(13, HIGH);
  delay(500); 
  digitalWrite(13, LOW);
  delay(500); 
  
  runCamera(); //On first startup, run through a capture sequence so user can verify functionality. Remove this to save battery.
  
}

void loop () {
  
// After wakeup starts here
  DateTime now = rtc.now();
  if (rtc.checkIfAlarm(1)) { // Generic call to see if the alarm has been triggered. If it has, run awakeNow()
    Serial.println("Alarm Triggered"); 
    awakeNow();
  }

  if (ALARM1 == false) { // If alarm hasn't happened yet, set alarm to ALARM1 time. 
    DateTime now = rtc.now();

      h = now.hour();
      m = now.minute() + 1;
      if (  m > 59) { 
          //int h = now.hour();
          m = 0;
          h = h + 1;

          if (h > 23){

              h = 0;
          }
          
          
          }
    
    rtc.setAlarm1Simple(h, m); // ***SET FIRST CAPTURE TIME HERE IN 24 HR FORMAT AS (HOUR, MINUTES)*** 
    rtc.turnOnAlarm(1); // Turn alarm on
    if (rtc.checkAlarmEnabled(1)) { // If it's enabled, tell me. 
      Serial.println("First Alarm Enabled");
      delay(100);
    }
  }

  if (ALARM1 == true) { // If Alarm1 has already gone off, set alarm to second desired time. 
    DateTime now = rtc.now();
    h2 = now.hour();
    m = now.minute() + 4;
    //int m = now.minute() +1;
      if (  m > 59) {
          m = 0;
          //h2 = now.hour();
          h2 = h2 + 1;
          }

       if (h2 > 23){

              h2 = 0;
          }
    rtc.setAlarm1Simple(h2, m); // ***SET SECOND CAPTURE TIME HERE IN 24 HR FORMAT AS (HOUR, MINUTES)*** 
    rtc.turnOnAlarm(1); // Turn alarm on
    if (rtc.checkAlarmEnabled(1)) { // If it's enabled, tell me. 
      Serial.println("Second Alarm Enabled");
      delay(100);
    }
  }
  
  Serial.print("Time is: ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
  Serial.println("Going to sleep now.");
  delay(600);
  sleepNow(); 
}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0,logData, FALLING);
  sleep_mode();
  //HERE AFTER WAKING UP
  sleep_disable();
  detachInterrupt(0);
}

void awakeNow() { 
  counter++; //increment counter by 1 (Starts at 0) each time Arduino wakes up from alarm. 
  Serial.print("Wake counter is now: ");
  Serial.println(counter); 
  
  if (flag == true & counter == 1) {  //run first alarm routine, set ALARM1 flag to true. 
    Serial.println("I am awake now. Doing first alarm stuff...");
    // DO STUFF HERE
    runCamera();
    flag = false; // Reset flag.
    Serial.println("flag reset");
    ALARM1 = true;
    Serial.println("ALARM1 flag set to true");
  }

  if (flag == true & counter == 2) { // if counter is two (second wake of the day) then run second alarm routine, then reset flags 
    Serial.println("I am awake now. Doing second alarm stuff..."); 
    // DO STUFF HERE
    runCameraV();
    flag = false; // Reset flag.
    Serial.println("flag reset");
    ALARM1 = false; // Reset flag
    Serial.println("ALARM1 flag set to false");
    counter = 0; // Reset daily wakeup counter
    Serial.println("Daily counter reset.");
  }
  
}

void runCameraV() {
  // Below are the controls for "pushing" buttons on our camera. 
  // This is set to run whenever runCamera is called. 
  
  // flash two short times to show it's awake.
    digitalWrite(13, HIGH);
    delay(500); 
    digitalWrite(13, LOW);
    delay(500); 
    digitalWrite(13, HIGH);
    delay(500); 
    digitalWrite(13, LOW);
    delay(500); 
    
    delay(1000);             // This is a spacer. Adjust as needed.
  
    digitalWrite(6, HIGH);   // open power mosfet for 3 seconds to turn on camera
    delay(3000);                       // 3 second *button push* 
    digitalWrite(6, LOW);    // close power mosfet, *finger off button*
  
    delay(5000); // wait 5 seconds to allow for camera startup 
  
    digitalWrite(5, HIGH);   // open shutter mosfet for 1 second to start video recording
    delay(1000);                       // 1 second *button push* 
    digitalWrite(5, LOW);    // close shutter mosfet, *finger off button*

    delay(60000); // 30 second delay while video records, this is how long your video will be. Longer means less battery life. 

    digitalWrite(5, HIGH);   // open shutter mosfet for 1 second to stop video recording
    delay(1000);                       // 1 second *button push* 
    digitalWrite(5, LOW);    // close shutter mosfet, *finger off button*

    delay(5000); // wait 5 seconds before powering off camera - gives time to save video

    digitalWrite(6, HIGH);   // open power mosfet for 3 seconds to turn off camera
    delay(3000);                       // 3 second *button push* 
    digitalWrite(6, LOW);    // close power mosfet, *finger off button* should be off now. 

    delay(1000); // 1 second spacer delay. Adjust as needed
}

void runCamera() {
  // Below are the controls for "pushing" buttons on our camera. 
  // This is set to run whenever runCamera is called. 
  
  // flash two short times to show it's awake.
    digitalWrite(13, HIGH);
    delay(500); 
    digitalWrite(13, LOW);
    delay(500); 
    digitalWrite(13, HIGH);
    delay(500); 
    digitalWrite(13, LOW);
    delay(500); 
    
    delay(1000);             // This is a spacer. Adjust as needed.
  
    digitalWrite(6, HIGH);   // open power mosfet for 3 seconds to turn on GoPro
    delay(3000);                       // 3 second *button push* 
    digitalWrite(6, LOW);    // close power mosfet, *finger off button*
  
    delay(5000); // wait 5 seconds before changing to picture mode to allow for camera startup

    digitalWrite(6, HIGH);   // open power mosfet for 1 seconds to change camera mode from video to photo
    delay(1000);                       // 1 second *button push* 
    digitalWrite(6, LOW);    // close power mosfet, *finger off button*

    delay(5000); // 5 second delay before capturing a photo to allow for mode change
  
    digitalWrite(5, HIGH);   // open shutter mosfet for 1 second to take photo
    delay(1000);                       // 1 second *button push* 
    digitalWrite(5, LOW);    // close shutter mosfet, *finger off button*

    delay(5000); // wait 5 seconds before powering off, time to save photo

    digitalWrite(6, HIGH);   // open power mosfet for 3 seconds to turn off camera
    delay(3000);                       // 3 second *button push* 
    digitalWrite(6, LOW);    // close power mosfet, *finger off button* 

    delay(1000); // 1 second spacer delay. Adjust as needed
}

void logData() {
  //do something quick, flip a flag, and handle in loop();
  flag = true; 
}
