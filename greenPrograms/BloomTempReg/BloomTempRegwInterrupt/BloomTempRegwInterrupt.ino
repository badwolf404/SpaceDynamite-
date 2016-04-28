//Temperature/Humidity Sensor Program 

//Grabs temperature and humidity from DHT22 sensors

//LIBRARIES 
#include <dht.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

// Creats a DHT object
dht DHT;

///////////////////////////////////GLOBAL VARIABLES///////////////////////////////

//bool for lightStatus, and tInF1,2 for temps and h1,2 for hums
bool isLightOn;
float tInF1;
float h1;
float tInF2;
float h2;

// Defines DHT22 pins
#define tempPin1 11
#define tempPin2 12 

//Defines LDR pins
#define lightPin1 0
#define lightPin2 1
 
//Outlets 2,3,4,5 are pin numbers on Arduino
#define outlet1 2
#define outlet2 3
#define outlet3 4
#define outlet4 5

//Parameters for desired temperature range
//Sleep Mode 
#define lowTempSleep 68
#define highTempSleep 72

//Awake Mode
#define lowTempAwake 75
#define highTempAwake 79

//Current Mode
int currentModeHigh = 0;
int currentModeLow = 0;

//Hot/Cold flags so we know when to come on and off
bool warmFlag = 0;
bool coolFlag = 0;

//Check flags to know whether we should be checking for hot or cold
bool checkHeaterTurnOn = 1;
bool checkCoolerTurnOn = 1;

//Delay is done in small 2 second increments to help with LCD printout / it doesn't seem to like too big of numbers
int timeDelay = 2000;

///////////////////////////////////GLOBAL VARIABLES END///////////////////////////////



//////////////////////////////////////////FUNCTIONS///////////////////////////////////
//Getters for Temp and Hum
float getTemp(int tempDataPin);
float getHum(int tempDataPin, float tInF);

float getAvg(float tInF, int anotherTInF);
void print2digits(int number);
bool getLightStatus(int lightDataPin1, int lightDataPin2);

//Print method for Serial
void printSerial(int sensorNum1, int sensorNum2, float tInF, float tInF2, float h, float h2, bool lightStatus);





///////////////////////////////////////FUNCTIONS END///////////////////////////////////

void setup()
{
  Serial.begin(9600);
  while (!Serial) ; // wait for serial (This came with time sensor deal, may not need)
  /*
  pinMode(outlet1, OUTPUT);
  pinMode(outlet2, OUTPUT);
  pinMode(outlet3, OUTPUT);
  pinMode(outlet4, OUTPUT);
  */
  // 10 second delay at start to help grab first data
  //delay(2000);
  
    // initialize Timer1
  cli();         // disable global interrupts
  TCCR1A = 0;    // set entire TCCR1A register to 0
  TCCR1B = 0;    // set entire TCCR1B register to 0 
               // (as we do not know the initial  values) 
  

  // enable Timer1 overflow interrupt:
  TIMSK1 |= (1 << TOIE1); //Atmega8 has no TIMSK1 but a TIMSK register

  // Set CS10 bit so timer runs at clock speed: (no prescaling)
  TCCR1B |= (3 << CS10); // Sets bit CS10 in TCCR1B
  // This is achieved by shifting binary 1 (0b00000001)
  // to the left by CS10 bits. This is then bitwise
  // OR-ed into the current value of TCCR1B, which effectively set
  // this one bit high. Similar: TCCR1B |= _BV(CS10);

  // enable global interrupts:
  sei();
}

///////////////////////////////////////INTERRUPT///////////////////////////////////////
ISR(TIMER1_OVF_vect)
{
  // Printing the results on the serial monitor
  tmElements_t tm;
  //Only want printed values at certain intervals
  //if (RTC.read(tm)) 
  //{
    
    //if( (tm.Minute % 2 == 0) && (tm.Second < 4) )
    //{

      //////////////////////////////////////////REAL TIME CLOCK SETUP AND LIGHT STATUS//////////////////////////////////////
      Serial.print("Time,Date: ");
      print2digits(tm.Hour);
      Serial.print(':');
      print2digits(tm.Minute);
      Serial.print(':'); 
      print2digits(tm.Second);
      Serial.print(",");
      Serial.print(tm.Day); 
      Serial.print('/');
      Serial.print(tm.Month); 
      Serial.print('/');
      Serial.print(tmYearToCalendar(tm.Year));
      Serial.print("  |  Light Status: " );
      
      if(isLightOn)
      {
        Serial.print("on");
      }
      else
      {
        Serial.print("off");
      }
     
    
    
    
    //Saving this code in case need to check if clock has stopped working.
    /*
    else 
    {
      if (RTC.chipPresent()) 
      {
        Serial.println("The DS1307 is stopped.  Please run the SetTime");
        Serial.println("example to initialize the time and begin running.");
        Serial.println();
      }
      else 
      {
        Serial.println("DS1307 read error!  Please check the circuitry."); 
        Serial.println();
      }
    }
    */
    
     
    //////////////////////////////////////////REAL TIME CLOCK SETUP AND LIGHT STATUS END//////////////////////////////////////
   
      Serial.print("  |  ");
      
      Serial.print("isTemperature");
      /*
      Serial.print(1);
      Serial.print(" = ");
      Serial.print(tInF1);
      Serial.print(" F ");
      Serial.print("    Humidity");  
      Serial.print(1);
      Serial.print(" = ");
      Serial.print(h1);
      Serial.print(" % ");
      Serial.print("  |  ");
      Serial.print(" Temperature");
      Serial.print(2);
      Serial.print(" = ");
      Serial.print(tInF2);
      Serial.print(" F ");
      Serial.print("    Humidity");  
      Serial.print(2);
      Serial.print(" = ");
      Serial.print(h2);
      Serial.print(" % ");
      Serial.print("\n");
     */
    //}
  //}
}
  
void loop() 
{

  //////////////////////////////////////////DATA COLLECTION//////////////////////////////////////////////////////

  //LIGHT CHECK
  isLightOn = getLightStatus(lightPin1, lightPin2);

  //TEMP SENSOR 1
  
  //Get temp and hum from getters
  tInF1 = getTemp(tempPin1);
  h1 = getHum(tempPin1, tInF1);

  //TEMP SENSOR 2
  
  //Get temp and hum from getters
  tInF2 = getTemp(tempPin2);
  h2 = getHum(tempPin2, tInF2);
  
  //print data to serial
  //printSerial(1, 2, tInF1, tInF2, h1, h2, isLightOn);

  ///////////////////////////////////////////DATA COLLECTION END//////////////////////////////////////////////////


  ///////////////////////////////////////////TEMPERATURE REGULATION///////////////////////////////////////////////
  //Get average of the two temperature sensors
  float average1 = getAvg(tInF1, tInF2);

  //If lights are on then set the high and low awake temps to current mode
  if(isLightOn)
  {
      currentModeHigh = highTempAwake;
      currentModeLow = lowTempAwake;
  }
  //If lights are off then set the high and low sleep temps to current mode
  else if(!isLightOn)
  {
      currentModeHigh = highTempSleep;
      currentModeLow = lowTempSleep;
  }


  //Possible Conditions are too hot or too cold
  //If too hot, turn on cooler, if too cool, turn on heater
  //Heater = outlet1 and cooler = outlet 2

  /////////////////////////////////////////////COLD CONDITIONS//////////////////////////////////////////////////

  //Initialized to true, it helps program determine whether it should bother to check to turn heater on
  //If cooler has been on for a bit, this flag will prevent the heater from coming on right away, unless
  //The temp drops 1 degree past the lowest desire.
  if(checkHeaterTurnOn)
  {
      //If the average temp is lower than the current low mode, turn on heater
      if(average1 < currentModeLow)
      {
        //HIGH = outlet on, LOW = outlet off
        digitalWrite(outlet1, HIGH);
      }

      //If the average temp is greater than the current high mode - 1, turn off heater
      if(average1 >= currentModeHigh - 1)
      {
        digitalWrite(outlet1, LOW);
        coolFlag = 1;
      }

       //Don't need to check for cooler since we just turned on heat, will check for cooler again if we bump up 1 degree above high temp
       if(coolFlag)
       {
         checkCoolerTurnOn = 0;
       }
       
       //This is if the heater happens to bump up the avg temp one degree above desired temp, then we check to turn cooler on
       if(average1 >= currentModeHigh + 1)
       {
         checkCoolerTurnOn = 1;
         coolFlag = 0;
       }
       
     
  }

  //////////////////////////////////////////////HOT CONDITIONS//////////////////////////////////////////////////

  if(checkCoolerTurnOn)
  {
       //If the average temp is higher than the current high mode, turn on cooler
       if(average1 >= currentModeHigh)
       {
        digitalWrite(outlet2, HIGH);
       }

       //If the average temp is less than the current low mode - 1, turn off cooler, set warmFlag to 1
       if(average1 < currentModeLow + 1)
       {
         digitalWrite(outlet2, LOW);
         warmFlag = 1;
       }

       //Don't need to check to turn on heater since we just cooled down
       if(warmFlag)
       {
         checkCoolerTurnOn = 0;
       }

       //Check again turn on heater since we have now bumped one degree below desired low temp
       if(average1 < currentModeLow - 1)
       {
         checkCoolerTurnOn = 1;
       }

       
  }
  
  ////////////////////////////////////////TEMPERATURE REGULATION END///////////////////////////////////////////////
  
  

}



float getTemp(int tempDataPin)
{
    int readData = DHT.read22(tempDataPin); // Reads the data from the sensor
    float t = DHT.temperature; // Gets the values of the temperature
  
    float tInF = (1.8 * t) + 32; // Convert to F

    if(readData == -3) // error checking for the dht22 sensor
    {
      tInF = 0;
    }
    
    //Wait for timeDelay (if grabbing sensors back to back this puts 2 seconds needed for sensors)
    delay(timeDelay);

    return tInF;
  
}

float getHum(int tempDataPin, float tInF)
{
    int readData = DHT.read22(tempDataPin); // Reads the data from the sensor
    float h = DHT.humidity; // Gets the values of the humidity
  
    if(tInF == 0) // error checking for the dht22 sensor
    {
      h = 0;
    }
  
    return h;
  
}

void printSerial(int sensorNum1, int sensorNum2, float tInF, float tInF2, float h, float h2, bool lightStatus)
{
  // Printing the results on the serial monitor
  tmElements_t tm;
  //Only want printed values at certain intervals
  //if (RTC.read(tm)) 
  //{
    
    //if( (tm.Minute % 2 == 0) && (tm.Second < 4) )
    //{

      //////////////////////////////////////////REAL TIME CLOCK SETUP AND LIGHT STATUS//////////////////////////////////////
      Serial.print("Time,Date: ");
      print2digits(tm.Hour);
      Serial.print(':');
      print2digits(tm.Minute);
      Serial.print(':'); 
      print2digits(tm.Second);
      Serial.print(",");
      Serial.print(tm.Day); 
      Serial.print('/');
      Serial.print(tm.Month); 
      Serial.print('/');
      Serial.print(tmYearToCalendar(tm.Year));
      Serial.print("  |  Light Status: " );
      
      if(lightStatus)
      {
        Serial.print("on");
      }
      else
      {
        Serial.print("off");
      }
     
    
    
    
    //Saving this code in case need to check if clock has stopped working.
    /*
    else 
    {
      if (RTC.chipPresent()) 
      {
        Serial.println("The DS1307 is stopped.  Please run the SetTime");
        Serial.println("example to initialize the time and begin running.");
        Serial.println();
      }
      else 
      {
        Serial.println("DS1307 read error!  Please check the circuitry."); 
        Serial.println();
      }
    }
    */
    
     
    //////////////////////////////////////////REAL TIME CLOCK SETUP AND LIGHT STATUS END//////////////////////////////////////
   
      Serial.print("  |  ");
      /*
      Serial.print(" Temperature");
      Serial.print(sensorNum1);
      Serial.print(" = ");
      Serial.print(tInF);
      Serial.print(" F ");
      Serial.print("    Humidity");  
      Serial.print(sensorNum1);
      Serial.print(" = ");
      Serial.print(h);
      Serial.print(" % ");
      Serial.print("  |  ");
      Serial.print(" Temperature");
      Serial.print(sensorNum2);
      Serial.print(" = ");
      Serial.print(tInF2);
      Serial.print(" F ");
      Serial.print("    Humidity");  
      Serial.print(sensorNum2);
      Serial.print(" = ");
      Serial.print(h2);
      Serial.print(" % ");
      Serial.print("\n");
      */
    //}
  //}
}


float getAvg(float tInF, int anotherTInF)
{
    //Error check to make sure we are not averaging two numbers with a zero
    if(tInF == 0)
    {
      tInF = anotherTInF;
    }
    
    //Error check to make sure we are not averaging two numbers with a zero
    if(anotherTInF == 0)
    {
       anotherTInF = tInF;
    }

    float average = ((tInF + anotherTInF) * (.5));

    return average;

}

bool getLightStatus(int lightDataPin1, int lightDataPin2)
{
     //LDR sensor is high value when dark (around 1000) and low value when light (around 50)
     int darkCalibration = 400;
     //read the input
     int lightVal1 = analogRead(lightDataPin1); 
     delay(100);
     int lightVal2 = analogRead(lightDataPin2);
     delay(100);
     

     
 
     if( (lightVal1 < darkCalibration) && (lightVal2 < darkCalibration) )
     {
          return true;
     }

     else 
     {
          return false;
     }
  
}


void print2digits(int number)
{

 if (number >= 0 && number < 10) 
  {
    Serial.write('0');
  }
  Serial.print(number);

}
