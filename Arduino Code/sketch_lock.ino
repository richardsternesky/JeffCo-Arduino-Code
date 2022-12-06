/**
   Author:Ab Kurk
   version: 1.0
   date: 4/02/2018
   Description:
   This sketch is part of the guide to putting your Arduino to sleep
   tutorial. We use the:
   Adafruit DS3231 RTC
   Adafruit 5V ready Micro SD break out board
   Arduino Pro Mini
   DHT11 or DHT22 humidety/Temperature sensor
   In this example we use the RTC to wake up the Arduino to log the temp and humidity on to an SD card.
   After the data has been logged the Arduino goes back to sleep and gets woken up 5 minutes later to
   start all over again
   Link To Tutorial http://www.thearduinomakerman.info/blog/2018/1/24/guide-to-arduino-sleep-mode
   Link To Project   http://www.thearduinomakerman.info/blog/2018/2/5/wakeup-rtc-datalogger
*/

#include <avr/sleep.h>//this AVR library contains the methods that controls the sleep modes
#define interruptPin 2 //Pin we are going to use to wake up the Arduino
#include <DS3232RTC.h>  //RTC Library https://github.com/JChristensen/DS3232RTC

//Variables needed for the SERVO
#define POUT1  9     // what digital pin we're connecting the first lock to
#define POUT2  10     // what digital pin we're connecting the second lock to
#define POUT3  8     // what digital pin we're connecting the third lock to
#define POUT4  11     // what digital pin we're connecting the fourth lock to


DS3232RTC myRTC;
//RTC Module global variables
const int time_interval = 1; // Sets the wakeup intervall in minutes
const double forward_its = 1700; // Number of loops required to send motor forward
const double back_its = 1000; // Number of loops required to send motor backward

bool go_forward = true;

void assert_lock();

void setup() {
  Serial.begin(115200);//Start Serial Comunication
  myRTC.begin();
  pinMode(LED_BUILTIN, OUTPUT); //We use the led on pin 13 to indecate when Arduino is A sleep
  pinMode(interruptPin, INPUT_PULLUP); //Set pin d2 to input using the buildin pullup resistor
  digitalWrite(LED_BUILTIN, HIGH); //turning LED on
  pinMode(POUT1, OUTPUT);//configure motor pin
  //pinMode(POUT2, OUTPUT);//configure motor pin
  //pinMode(POUT3, OUTPUT);//configure motor pin
  //pinMode(POUT4, OUTPUT);//configure motor pin
  digitalWrite(POUT1, LOW);
  //digitalWrite(POUT2, LOW);
  //digitalWrite(POUT3, LOW);
  //digitalWrite(POUT4, LOW);

  // initialize the alarms to known values, clear the alarm flags, clear the alarm interrupt flags
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_DATE, 0, 0, 0, 1);
  myRTC.setAlarm(DS3232RTC::ALM2_MATCH_DATE, 0, 0, 0, 1);
  myRTC.alarm(DS3232RTC::ALARM_1);
  myRTC.alarm(DS3232RTC::ALARM_2);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_1, false);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_2, false);
  myRTC.squareWave(DS3232RTC::SQWAVE_NONE);
  /*
     Uncomment the block block to set the time on your RTC. Remember to comment it again
     otherwise you will set the time at everytime you upload the sketch
     /
    /* Begin block
    tmElements_t tm;
    tm.Hour = 00;               // set the RTC to an arbitrary time
    tm.Minute = 00;
    tm.Second = 00;
    tm.Day = 4;
    tm.Month = 2;
    tm.Year = 2018 - 1970;      // tmElements_t.Year is the offset from 1970
    RTC.write(tm);              // set the RTC from the tm structure
    Block end * */
  tmElements_t tm;
  tm.Hour = 00;               // set the RTC to an arbitrary time
  tm.Minute = 00;
  tm.Second = 00;
  tm.Day = 17;
  tm.Month = 9;
  tm.Year = 2022 - 1970;      // tmElements_t.Year is the offset from 1970
  myRTC.write(tm);              // set the RTC from the tm structure

  time_t t; //create a temporary time variable so we can set the time and read the time from the RTC
  t = myRTC.get(); //Gets the current time of the RTC
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);// Setting alarm 1 to go off 1 minute from now
  // clear the alarm flag
  myRTC.alarm(DS3232RTC::ALARM_1);
  // configure the INT/SQW pin for "interrupt" operation (disable square wave output)
  myRTC.squareWave(DS3232RTC::SQWAVE_NONE);
  // enable interrupt output for Alarm 1
  myRTC.alarmInterrupt(DS3232RTC::ALARM_1, true);
}

void loop() {
  delay(5000);//wait 5 seconds before going to sleep. In real senairio keep this as small as posible
  Going_To_Sleep();
}

void Going_To_Sleep() {
  sleep_enable();//Enabling sleep mode
  attachInterrupt(0, wakeUp, LOW);//attaching a interrupt to pin d2
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);//Setting the sleep mode, in our case full sleep
  digitalWrite(LED_BUILTIN, LOW); //turning LED off
  time_t t;// creates temp time variable
  t = myRTC.get(); //gets current time from rtc
  Serial.println("Sleep  Time: " + String(hour(t)) + ":" + String(minute(t)) + ":" + String(second(t))); //prints time stamp on serial monitor
  delay(1000); //wait a second to allow the led to be turned off before going to sleep
  sleep_cpu();//activating sleep mode
  Serial.println("just woke up!");//next line of code executed after the interrupt
  digitalWrite(LED_BUILTIN, HIGH); //turning LED on
  go_forward = !go_forward;//alternate between locking and unlocking
  assert_lock();//function that reads the temp and the humidity
  t = myRTC.get();
  Serial.println("WakeUp Time: " + String(hour(t)) + ":" + String(minute(t)) + ":" + String(second(t))); //Prints time stamp
  //Set New Alarm
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);

  // clear the alarm flag
  myRTC.alarm(DS3232RTC::ALARM_1);
}

void wakeUp() {
  Serial.println("Interrupt Fired");//Print message to serial monitor
  sleep_disable();//Disable sleep mode
  detachInterrupt(0); //Removes the interrupt from pin 2;

}


//This function either locks or unlocks the door depending on the current state of go_forward
void assert_lock() {
  if (go_forward) Serial.println("Forward");
  else Serial.println("Backward");
  double its = go_forward ? forward_its : back_its;
  digitalWrite(POUT1, HIGH);
  //digitalWrite(POUT2, HIGH);
  //digitalWrite(POUT3, HIGH);
  //digitalWrite(POUT4, HIGH);
  delayMicroseconds(its);
  digitalWrite(POUT1, LOW);
  //digitalWrite(POUT2, LOW);
  //digitalWrite(POUT3, LOW);
  //digitalWrite(POUT4, LOW);
  delayMicroseconds(20000 - its);
}
