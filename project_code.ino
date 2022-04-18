// Stepper Motor Library - Controls 
#include <Stepper.h>
// LCD Library - Display Temp, Humidity, Date/Time of Start
#include <LiquidCrystal.h>
// Clock Library - Logs Date and Time
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
// Temp/Humidity Sensor Library
#include <DHT.h>
#include <DHT_U.h>

#define THRESHOLD 75
#define WATER_LEVEL 00
#define HUMIDITY_SENSOR 9

// Time
  const char *monthName[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  tmElements_t tm;
  bool parse=false;
  bool config=false;

// Defining ADC Registers
  volatile unsigned char *myADMUX = (unsigned char *)  0x7C;
  volatile unsigned char *myADCSRB = (unsigned char *) 0x7B;
  volatile unsigned char *myADCSRA = (unsigned char *) 0x7A;
  volatile unsigned int *myADCL = (unsigned int *)  0x78;

// Define Port B Registers
  volatile unsigned char* port_b = (unsigned char*) 0x25;
  volatile unsigned char* ddr_b  = (unsigned char*) 0x24;
  volatile unsigned char* pin_b  = (unsigned char*) 0x23;
// Define Port H Registers
  volatile unsigned char* port_h = (unsigned char*) 0x102;
  volatile unsigned char* ddr_h  = (unsigned char*) 0x101;
  volatile unsigned char* pin_h  = (unsigned char*) 0x100;

  int start = 3;
  int disable = 0;
  int idle = 1;
  int error = 2;
  // Values to track
  int water_level = 0;
  int temp = 0;
  int humidity = 0;
  const char *states [4] = {"Disable","Idle","Error","Running"};

void setup() {
  // Serial Monitor Initialization
  Serial.begin(9600);
  // Initialize ADC registers
      adc_init();
  // PIN SETUP
    // Set PH6 as input
      *ddr_h &= 0xBF;
      *port_h &= 0xBF;
      // Set PB7, PB6, PB5, PB4 as output
      set_PB_as_output(7); // Disable - Yellow
      set_PB_as_output(6); // Idle - Green
      set_PB_as_output(5); // Error - Red
      set_PB_as_output(4); // Running - Blue
  // Clock Setup
      configure_RTC();
      print_state_change(disable);
      *port_b = 0x00;
      write_pb(7, 1);   
}

void loop() {
  start = (*pin_h & 0x40);
    // Start running process
    // Check state: IDLE, RUNNING, ERROR
    // If IDLE: Green LED high
    // Monitor: TEMP, WATER_LEVEL, Log transition times
  if ((start & 0x40) != 0)
  {  
    print_state_change(1);
    write_pb(7,0);
    write_pb(6, 1);
    while ((start & 0x40) != 0)
    {
      start = (*pin_h & 0x40);
      // Read temp
      int check = humiditySensor.read11(HUMIDITY_SENSOR);
      temp = humiditySensor.temperature;
        while (temp < THRESHOLD) {
        delay(30);
        check = humiditySensor.read11(HUMIDITY_SENSOR);
        temp = humiditySensor.temperature;
    },
      // Read humidity
      // print humidity and temp
      if (temp > THRESHOLD)
      {
        // Running state
        write_pb(4, 1);
        print_state_change(start);
        // 
        if (water_level <= WATER_LEVEL)
        {
          // Transition to ERROR state
          print_state_change(error);
          write_pb(4, 0);
          write_pb(5, 1);
          // Turn off motor
          write_pb(5, 0);
          print_state_change(start);
        }
        write_pb(4, 0);
      }
    }
    // Turn Fan OFF
    // Monitor START
    print_state_change(disable);
    // Turn Idle LED off
    write_pb(6, 0);
    // Set Yellow LED to high
    write_pb(7, 1);
  }
}
    
    /*
    if (idle == 1)
    {
       write_pb(4, 1);
    }
    // If RUNNING: Fan high, Blue LED high - Yellow/Red LEDs low
    // Monitor: TEMP, WATER_LEVEL
    else if (start == 1)
    {
       write_pb(5, 1);
    }
    // If ERROR: Motor low, Display error, RED LED high
    // Monitor: RESET state, ERROR_MSG
    else if (error == 1)
    {
       write_pb(6, 1); 
    }
    */

/*//////////////////////////////////////////////////////////////
PIN FUNCTIONS
//////////////////////////////////////////////////////////////*/
void set_PB_as_output(unsigned char pin_num)
{
    *ddr_b |= 0x01 << pin_num;
}
void write_pb(unsigned char pin_num, unsigned char state)
{
  if(state == 0)
  {
    *port_b &= ~(0x01 << pin_num);
  }
  else
  {
    *port_b |= 0x01 << pin_num;
  }
}

/*//////////////////////////////////////////////////////////////
RTC FUNCTIONS
//////////////////////////////////////////////////////////////*/
void configure_RTC()
{
  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm)) {
      config = true;
    }
  } 
}
bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}
bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}
void print_state_change(unsigned int index) 
{
        Serial.print(" Transistion to state:");
        Serial.println(states[index]);
         while(!Serial);
         delay(200);
         if (parse&&config)
         {
            Serial.print(__TIME__);
            Serial.print(", Date=");
            Serial.println(__DATE__);
         }
}


/*//////////////////////////////////////////////////////////////
ISR FUNCTIONS
//////////////////////////////////////////////////////////////*/

/*//////////////////////////////////////////////////////////////
ADC FUNCTIONS
- adc_init(): Function to initialize ADC registers
- adc_read(): Function to read and convert ADC input value
//////////////////////////////////////////////////////////////*/

void adc_init()
{
  *myADCSRA = 0x80; // Turns ADC on and sets clock prescaler to (111)2 -> (128)10
  *myADCSRB &= 0xF0; // Sets ADC0: MUX[0:5] = 000000
  *myADMUX = 0x40; // Sets Vref to AVCC, right justified, ADC output channel = 0 (0000)
}

unsigned int adc_read(unsigned char adc_channel)
{ 
  *myADMUX &= 0xE0; // clears channel selection
  *myADCSRB &= 0xF7; // clears mux 5 channel selection

  if (adc_channel > 7)
  {
    adc_channel -= 8;
    *myADCSRB |= 0x08; // Set mux 5 to 1: (MUX 5, MUX 3, MUX 2, MUX 1) -> 1XXX
  }
  *myADMUX += adc_channel;
  *myADCSRA |= 0x40;  // Starts conversion
  while ((*myADCSRA & 0x40) != 0); // See if conversion is done
  return *myADCL;
}
