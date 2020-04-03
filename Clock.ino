
/**
 * @author Daniel Stara (for the most part)
 * -Used a couple of libraries from Adafruit (bless them) 
 * to communicate with the clock and lcd screen.
 * 
 * This application is basically a clock that runs with a 
 * chronodot timer and an adafruit lcd shield kit.  The 
 * time can be set with the buttons on the shield.  
 * 
 * 
 * 1. DO NOT hit the button on the very right below 
 *    the orange thing.  That is the reset button
 * 2. Might be adding functionality for an MP3 player
 * 
 * 
 */
//import statements I think
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include "RTClib.h"  // Credit: Adafruit


//Initializers that I don't understand (I2C)
RTC_DS1307 RTC;
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//button manager
int topButton = 8;
int leftButton = 16;
int rightButton = 2;
int bottomButton = 4;
int sideButton = 1;

//variables to control the lcd turning on and off
bool isBacklightOn = false;
unsigned long timer;
unsigned long lightTimer = 0;
unsigned long timeOn = 120000;

//variables for the bells
int bong = 9;
int bell = 8;

//delay for the watchdog
int delayTime = 1000;


/*
 * setup to get things ready - runs once at startup and 
 * whenever you open the serial monitor (learned that the hard way)
 */
void setup() {
 
  // Begin the Serial connection 
  Serial.begin(9600);

  //instantiate the lcd screen
  prepareLcd();
 
  // Instantiate the RTC
  prepareRTC();
 
  Serial.println("Setup complete.");

  //set backlight and initialize digital outputs
  lcd.setBacklight(255);


  pinMode(bell, OUTPUT);
  pinMode(bong, OUTPUT);

}//end of setup

/*
 * loop that the microcontroller runs over and over and over...and over
 * as fast as its little legs will let it
 */
void loop() {

  //timer for the 
  timer = millis();
  
  // Get the current time
  DateTime now = RTC.now();   
 
  // Display the current time for debugging
  // printDataSerial(now);

  //print the date to the lcd
  printDate(now);

  //print the time to the lcd
  printTime(now);

  //get button input
  uint8_t buttons = lcd.readButtons();



//handle the backlight for the lcd ------
  //set the backlight for the lcd
  if(isBacklightOn){
    lcd.setBacklight(255);
  }else{
    lcd.setBacklight(0);
  }

  if(buttons == sideButton){
    isBacklightOn = true;
    lightTimer = timer;
  }

  if((timer - lightTimer) > timeOn){
    
    isBacklightOn = false;
  }

  //---------


  //test this stupid timer bug
  Serial.print("timer: ");
  Serial.print(timer);
  Serial.print(" ");
  Serial.print("lightTimer: ");
  Serial.print(lightTimer);
  Serial.print(" ");
  Serial.print(isBacklightOn);
  Serial.println();
  

  //button handlers---------
  //add to the hour
  if(buttons == (topButton)){
    addHour(now);
  }
  //add to the minute
  if(buttons == (leftButton)){
    addMinute(now);
  }
  //month button
  if(buttons == (bottomButton)){
    addMonth(now);
  }
  //add to the day
  if(buttons == (rightButton)){
    addDay(now);
  }
  //add to year
  if(buttons == (sideButton + topButton)){
    addYear(now);
  }
  //or subtract year
  if(buttons == (sideButton + bottomButton)){
    subtractYear(now);
  }
  //-----------
  

  //if any button is pressed speed that sh** up!
  if(isBacklightOn){
    delayTime = 10;
  }else{
    delayTime = 1000;
  }




  /*
   * 
   *   bells will ring from 8 - 9
   *   angelus at 7:30, noon, 6
   *   if we're on a new hour
   *   ring the bells
  */
  //if we're on a new hour
  if(now.minute() == 0 && now.second() == 0){

    //if its not noon or 6pm
    if(!(now.hour() == 12) && !(now.hour() == 18)){
      
      //if we're between 8am and noon
      if(now.hour() > 7 && now.hour() < 12){
        ringBell(now.hour());
      }
  
      //if we're after noon, need to convert time from military
      if(now.hour() > 12 && now.hour() < 22){
        ringBell(now.hour() - 12);
      }
      
    }
  }



  //do the angelus at 7:30, noon, and 6 right on the dot   
  if(now.hour() == 7 && now.minute() == 30 && now.second() == 0){
    ringAngelus();
  }
  if(now.hour() == 12 && now.minute() == 00 && now.second() == 0){
    ringAngelus();
  }
  if(now.hour() == 18 && now.minute() == 00 && now.second() == 0){
    ringAngelus();
  }

  
  //long (watchdog) delay
  delay(delayTime);
  
}//end of loop


void ringAngelus(){
  for(int i = 0; i < 3; i++){
    ringBell(3);
    delay(4500);
  }

  digitalWrite(bell, HIGH);
  delay(18000);
  digitalWrite(bell, LOW);

}

void ringBell(int numRings){

  for(int i = 0; i < numRings; i++){
    digitalWrite(bong, HIGH);
    delay(500);
    digitalWrite(bong, LOW);
    delay(2000);
  }
  
}

/**
 * functions to change the time
 */
void subtractYear(DateTime now){
    int newYear = now.year()-1;
    
    RTC.adjust(DateTime(newYear, now.month(), now.day(), now.hour(), now.minute(), now.second()));
    //update the lcd
    lcd.clear();
    printDate(now);
    printTime(now);
}
void addYear(DateTime now){
    int newYear = now.year()+1;
    
    RTC.adjust(DateTime(newYear, now.month(), now.day(), now.hour(), now.minute(), now.second()));
    //update the lcd
    lcd.clear();
    printDate(now);
    printTime(now);
}
void addMonth(DateTime now){
    int newMonth = now.month()+1;

    if(newMonth > 12){
      newMonth = 1;
    }
    
    RTC.adjust(DateTime(now.year(), newMonth, now.day(), now.hour(), now.minute(), now.second()));
    //update the lcd
    lcd.clear();
    printDate(now);
    printTime(now);
}
void addDay(DateTime now){
    int newDay = now.day()+1;

    if(newDay > 31){
      newDay = 1;
    }
    
    RTC.adjust(DateTime(now.year(), now.month(), newDay, now.hour(), now.minute(), now.second()));
    //update the lcd
    lcd.clear();
    printDate(now);
    printTime(now);
}
void addMinute(DateTime now){
    int newMinute = now.minute()+1;
    if(newMinute > 59){
      newMinute = 1;
    }
    
    RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), newMinute, now.second()));
    //update the lcd
    lcd.clear();
    printDate(now);
    printTime(now);
}
void addHour(DateTime now){
    int newHour = now.hour()+1;
   
    //if we've gone past military time (lol traveling through time)
    if(newHour > 23){
      newHour = 0;
      Serial.print(now.minute());
      lcd.clear();
    }
    
    RTC.adjust(DateTime(now.year(), now.month(), now.day(), newHour, now.minute(), now.second()));
    //update the lcd
    lcd.clear();
    printDate(now);
    printTime(now);
}




/*
 * method to print the date to the lcd
 */
void printDate(DateTime now){
  lcd.setCursor(0,0);
  
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.day(), DEC);
  lcd.print('/');
  lcd.print(now.year(), DEC);
  lcd.print("     ");
}

/*
 * method to print the time to the lcd
 */
void printTime(DateTime now){
  lcd.setCursor(0,1);

  lcd.print(now.hour(), DEC); 
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  //a bunch of spaces because the format was screwing me up.
  lcd.print("    ");
}

/*
 * method to print data to the serial monitor
 */
void printDataSerial(DateTime now){
  Serial.print("Current time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}

void prepareLcd(){
  lcd.begin(16, 2);
  lcd.print("Hey");
  lcd.setCursor(0, 1);
  lcd.print(":)");
}

void prepareRTC(){
  Wire.begin();
  RTC.begin();
  
 
  // Check if the RTC is running.
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running");
  }

  // This section grabs the current datetime and compares it to
  // the compilation time.  If necessary, the RTC is updated.
  DateTime now = RTC.now();
//  if (now.unixtime() < compiled.unixtime()) {
//    Serial.println("RTC is older than compile time! Updating");
//    RTC.adjust(DateTime(__DATE__, __TIME__));
//  }
}
