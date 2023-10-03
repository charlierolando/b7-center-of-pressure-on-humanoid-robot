#include <Wire.h>
#include "Arduino_FreeRTOS.h"
#include "Barelang7_COP.h"

volatile bool thread_1_2_stop = false;
volatile uint16_t xcop_global, ycop_global;
volatile bool tare_ = false;

//HX711 constructor (dout pin, sck pin) left
HX711_ADC LoadCell_1a(30, 32);
HX711_ADC LoadCell_2a(34, 36);
HX711_ADC LoadCell_3a(37, 35);
HX711_ADC LoadCell_4a(41, 39);

//HX711 constructor (dout pin, sck pin) right
HX711_ADC LoadCell_1b(38, 40);
HX711_ADC LoadCell_2b(42, 44);
HX711_ADC LoadCell_3b(45, 43);
HX711_ADC LoadCell_4b(49, 47);

//LCD constructor
LiquidCrystal_I2C lcd(0x27, 20, 4);

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {10, 9, 8, 7}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//Sensor Position to Foot Pad
int lxa = 25;   //Left
int lxb = 85;
int lxc = 25;
int lxd = 85;

int lya = 170;
int lyb = 170;
int lyc = 25;
int lyd = 25;

int rxa = 25;   //Right
int rxb = 85;
int rxc = 25;
int rxd = 85;

int rya = 170;
int ryb = 170;
int ryc = 25;
int ryd = 25;

//LoadCell Calibration Value
double calibrationValue_1a = 152.47; 
double calibrationValue_2a = 128.52;
double calibrationValue_3a = 181.56;
double calibrationValue_4a = 160.73;

double calibrationValue_1b = 154.32; 
double calibrationValue_2b = 144.94;
double calibrationValue_3b = 142.17;
double calibrationValue_4b = 167.69;

// Smoother Calculation Variables
const int numReadings_1a = 5;       //Left Pad
int readings_1a[numReadings_1a];
int sIndex_1a = 0;
double sTotal_1a = 0;
double sAverage_1a = 0;

const int numReadings_2a = 5;
int readings_2a[numReadings_2a];
int sIndex_2a = 0;
double sTotal_2a = 0;
double sAverage_2a = 0;

const int numReadings_3a = 5;
int readings_3a[numReadings_3a];
int sIndex_3a = 0;
double sTotal_3a = 0;
double sAverage_3a = 0;

const int numReadings_4a = 5;
int readings_4a[numReadings_4a];
int sIndex_4a = 0;
double sTotal_4a = 0;
double sAverage_4a = 0;

const int numReadings_1b = 5;       //Right Pad
int readings_1b[numReadings_1b];
int sIndex_1b = 0;
double sTotal_1b = 0;
double sAverage_1b = 0;

const int numReadings_2b = 5;
int readings_2b[numReadings_2b];
int sIndex_2b = 0;
double sTotal_2b = 0;
double sAverage_2b = 0;

const int numReadings_3b = 5;
int readings_3b[numReadings_3b];
int sIndex_3b = 0;
double sTotal_3b = 0;
double sAverage_3b = 0;

const int numReadings_4b = 5;
int readings_4b[numReadings_4b];
int sIndex_4b = 0;
double sTotal_4b = 0;
double sAverage_4b = 0;

// Change Variables
double t_1a;                 //Left Pad
double tChange_1a;
boolean Change_1a = false;

double t_2a;
double tChange_2a;
boolean Change_2a = false;

double t_3a;
double tChange_3a;
boolean Change_3a = false;

double t_4a;
double tChange_4a;
boolean Change_4a = false;

double t_1b;                 //Right Pad
double tChange_1b;
boolean Change_1b = false;

double t_2b;
double tChange_2b;
boolean Change_2b = false;

double t_3b;
double tChange_3b;
boolean Change_3b = false;

double t_4b;
double tChange_4b;
boolean Change_4b = false;

//CoP position
uint16_t xCOP = 0;
uint16_t yCOP = 0;

volatile uint16_t temp_lx = 0;
volatile uint16_t temp_ly = 0;
volatile uint16_t temp_rx = 0;
volatile uint16_t temp_ry = 0;

volatile uint16_t temp_xCOP = 0;
volatile uint16_t temp_yCOP = 0;

//Delay MultiTask
unsigned long  prevTime = millis();

//Receive String Data from Serial
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String commandString = "";

//Keypad Input Parameter
unsigned long loopCount;
unsigned long startTime;
String msg;

int dx_value_int;
String value_keypad = "";
String keypad_temp = "";

void setup() {
  Serial.begin(115200);

  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);

  // initialize the lcd
  lcd.init();

  // Print a message to the LCD.
  lcd.backlight();
  welcome_text_1();

  //Initial the loadcell
  LoadCell_1a.begin();
  LoadCell_2a.begin();
  LoadCell_3a.begin();
  LoadCell_4a.begin();

  LoadCell_1b.begin();
  LoadCell_2b.begin();
  LoadCell_3b.begin();
  LoadCell_4b.begin();

  // Keypad
  loopCount = 0;
  startTime = millis();
  msg = "";

  unsigned long stabilizingtime = 1000;
  boolean _tare = true;

  byte loadcell_1a_rdy = 0;     //Left Pad
  byte loadcell_2a_rdy = 0;
  byte loadcell_3a_rdy = 0;
  byte loadcell_4a_rdy = 0;

  byte loadcell_1b_rdy = 0;     //Right Pad
  byte loadcell_2b_rdy = 0;
  byte loadcell_3b_rdy = 0;
  byte loadcell_4b_rdy = 0;

  while ((loadcell_1a_rdy + loadcell_2a_rdy + loadcell_3a_rdy + loadcell_4a_rdy +
          loadcell_1b_rdy + loadcell_2b_rdy + loadcell_3b_rdy + loadcell_4b_rdy) < 8)
  {
    if (!loadcell_1a_rdy) loadcell_1a_rdy = LoadCell_1a.startMultiple(stabilizingtime, _tare);
    if (!loadcell_2a_rdy) loadcell_2a_rdy = LoadCell_2a.startMultiple(stabilizingtime, _tare);
    if (!loadcell_3a_rdy) loadcell_3a_rdy = LoadCell_3a.startMultiple(stabilizingtime, _tare);
    if (!loadcell_4a_rdy) loadcell_4a_rdy = LoadCell_4a.startMultiple(stabilizingtime, _tare);

    if (!loadcell_1b_rdy) loadcell_1b_rdy = LoadCell_1b.startMultiple(stabilizingtime, _tare);
    if (!loadcell_2b_rdy) loadcell_2b_rdy = LoadCell_2b.startMultiple(stabilizingtime, _tare);
    if (!loadcell_3b_rdy) loadcell_3b_rdy = LoadCell_3b.startMultiple(stabilizingtime, _tare);
    if (!loadcell_4b_rdy) loadcell_4b_rdy = LoadCell_4b.startMultiple(stabilizingtime, _tare);
  }
  if (LoadCell_1a.getTareTimeoutFlag()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Timeout 1"); //Definition of the LoadCell
  }
  if (LoadCell_2a.getTareTimeoutFlag()) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Timeout 2");
  }
  if (LoadCell_3a.getTareTimeoutFlag()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Timeout 3");
  }
  if (LoadCell_4a.getTareTimeoutFlag()) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Timeout 4");
  }
  if (LoadCell_1b.getTareTimeoutFlag()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Timeout 5");
  }
  if (LoadCell_2b.getTareTimeoutFlag()) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Timeout 6");
  }
  if (LoadCell_3b.getTareTimeoutFlag()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Timeout 7");
  }
  if (LoadCell_4b.getTareTimeoutFlag()) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Timeout 8");
  }
  LoadCell_1a.setCalFactor(calibrationValue_1a);    //Left Pad
  LoadCell_2a.setCalFactor(calibrationValue_2a);
  LoadCell_3a.setCalFactor(calibrationValue_3a);
  LoadCell_4a.setCalFactor(calibrationValue_4a);

  LoadCell_1b.setCalFactor(calibrationValue_1b);    //Right Pad
  LoadCell_2b.setCalFactor(calibrationValue_2b);
  LoadCell_3b.setCalFactor(calibrationValue_3b);
  LoadCell_4b.setCalFactor(calibrationValue_4b);

  //Initialize all the index readings used in Smoother() to 0:
  for (int thisReading_1a = 0; thisReading_1a < numReadings_1a; thisReading_1a++)   //Left Pad
    readings_1a[thisReading_1a] = 0;

  for (int thisReading_2a = 0; thisReading_2a < numReadings_2a; thisReading_2a++)
    readings_2a[thisReading_2a] = 0;

  for (int thisReading_3a = 0; thisReading_3a < numReadings_3a; thisReading_3a++)
    readings_3a[thisReading_3a] = 0;

  for (int thisReading_4a = 0; thisReading_4a < numReadings_4a; thisReading_4a++)
    readings_4a[thisReading_4a] = 0;

  for (int thisReading_1b = 0; thisReading_1b < numReadings_1b; thisReading_1b++)   //Right Pad
    readings_1a[thisReading_1b] = 0;

  for (int thisReading_2b = 0; thisReading_2b < numReadings_2b; thisReading_2b++)
    readings_2b[thisReading_2b] = 0;

  for (int thisReading_3b = 0; thisReading_3b < numReadings_3b; thisReading_3b++)
    readings_3b[thisReading_3b] = 0;

  for (int thisReading_4b = 0; thisReading_4b < numReadings_4b; thisReading_4b++)
    readings_4b[thisReading_4b] = 0;


  // Print a message to the LCD.
  lcd.backlight();
  welcome_text_2();

  // Now set up two tasks to run independently. //RTOS Task
  xTaskCreate(Thread_1, "Thread_1", 512, NULL, 1, NULL);
  xTaskCreate(Thread_2, "Thread_2", 256, NULL, 1, NULL);
  xTaskCreate(Thread_3, "Thread_3", 256, NULL, 1, NULL);
}

void loop() {}

void Smoother()
{
  sTotal_1a = sTotal_1a - readings_1a[sIndex_1a];
  readings_1a[sIndex_1a] = t_1a;
  sTotal_1a = sTotal_1a + readings_1a[sIndex_1a];
  sIndex_1a = sIndex_1a + 1;

  sTotal_2a = sTotal_2a - readings_2a[sIndex_2a];
  readings_2a[sIndex_2a] = t_2a;
  sTotal_2a = sTotal_2a + readings_2a[sIndex_2a];
  sIndex_2a = sIndex_2a + 1;

  sTotal_3a = sTotal_3a - readings_3a[sIndex_3a];
  readings_3a[sIndex_3a] = t_3a;
  sTotal_3a = sTotal_3a + readings_3a[sIndex_3a];
  sIndex_3a = sIndex_3a + 1;

  sTotal_4a = sTotal_4a - readings_4a[sIndex_4a];
  readings_4a[sIndex_4a] = t_4a;
  sTotal_4a = sTotal_4a + readings_4a[sIndex_4a];
  sIndex_4a = sIndex_4a + 1;

  sTotal_1b = sTotal_1b - readings_1b[sIndex_1b];
  readings_1b[sIndex_1b] = t_1b;
  sTotal_1b = sTotal_1b + readings_1b[sIndex_1b];
  sIndex_1b = sIndex_1b + 1;

  sTotal_2b = sTotal_2b - readings_2b[sIndex_2b];
  readings_2b[sIndex_2b] = t_2b;
  sTotal_2b = sTotal_2b + readings_2b[sIndex_2b];
  sIndex_2b = sIndex_2b + 1;

  sTotal_3b = sTotal_3b - readings_3b[sIndex_3b];
  readings_3b[sIndex_3b] = t_3b;
  sTotal_3b = sTotal_3b + readings_3b[sIndex_3b];
  sIndex_3b = sIndex_3b + 1;

  sTotal_4b = sTotal_4b - readings_4b[sIndex_4b];
  readings_4b[sIndex_4b] = t_4b;
  sTotal_4b = sTotal_4b + readings_4b[sIndex_4b];
  sIndex_4b = sIndex_4b + 1;

  if (sIndex_1a >= numReadings_1a)
    sIndex_1a = 0;
  sAverage_1a = sTotal_1a / numReadings_1a;

  if (sAverage_1a != t_1a)
    Change_1a = true;

  if (sIndex_2a >= numReadings_2a)
    sIndex_2a = 0;
  sAverage_2a = sTotal_2a / numReadings_2a;

  if (sAverage_2a != t_2a)
    Change_2a = true;

  if (sIndex_3a >= numReadings_3a)
    sIndex_3a = 0;
  sAverage_3a = sTotal_3a / numReadings_3a;

  if (sAverage_3a != t_3a)
    Change_3a = true;

  if (sIndex_4a >= numReadings_4a)
    sIndex_4a = 0;
  sAverage_4a = sTotal_4a / numReadings_4a;

  if (sAverage_4a != t_4a)
    Change_4a = true;

  if (sIndex_1b >= numReadings_1b)
    sIndex_1b = 0;
  sAverage_1b = sTotal_1b / numReadings_1b;

  if (sAverage_1b != t_1b)
    Change_1b = true;

  if (sIndex_2b >= numReadings_2b)
    sIndex_2b = 0;
  sAverage_2b = sTotal_2b / numReadings_2b;

  if (sAverage_2b != t_2b)
    Change_2b = true;

  if (sIndex_3b >= numReadings_3b)
    sIndex_3b = 0;
  sAverage_3b = sTotal_3b / numReadings_3b;

  if (sAverage_3b != t_3b)
    Change_3b = true;

  if (sIndex_4b >= numReadings_4b)
    sIndex_4b = 0;
  sAverage_4b = sTotal_4b / numReadings_4b;

  if (sAverage_4b != t_4b)
    Change_4b = true;
}

void welcome_text_1()
{
  lcd.setCursor(1, 1);
  lcd.print("Centre of Pressure");
  lcd.setCursor(6, 2);
  lcd.print("B7 X BFC");
}

void welcome_text_2()
{
  lcd.setCursor(1, 1);
  lcd.print("                  ");
  lcd.setCursor(6, 2);
  lcd.print("        ");
}
