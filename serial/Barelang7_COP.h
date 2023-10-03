#ifndef Barelang7_COP
#define Barelang7_COP

#include <Arduino.h>
#include <inttypes.h>
#include <Wire.h>
#include "Print.h" 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*@author Mark Stanley, Alexander Brevig*/
#ifndef Keypadlib_KEY_H_
#define Keypadlib_KEY_H_

#define OPEN LOW
#define CLOSED HIGH

typedef unsigned int uint;
typedef enum{ IDLE, PRESSED, HOLD, RELEASED } KeyState;

const char NO_KEY = '\0';

class Key {
public:
	// members
	char kchar;
	int kcode;
	KeyState kstate;
	boolean stateChanged;

	// methods
	Key();
	Key(char userKeyChar);
	void key_update(char userKeyChar, KeyState userState, boolean userStatus);

private:

};

#endif

#ifndef KEYPAD_H
#define KEYPAD_H

#ifndef INPUT_PULLUP
#warning "Using  pinMode() INPUT_PULLUP AVR emulation"
#define INPUT_PULLUP 0x2
#define pinMode(_pin, _mode) _mypinMode(_pin, _mode)
#define _mypinMode(_pin, _mode)  \
do {							 \
	if(_mode == INPUT_PULLUP)	 \
		pinMode(_pin, INPUT);	 \
		digitalWrite(_pin, 1);	 \
	if(_mode != INPUT_PULLUP)	 \
		pinMode(_pin, _mode);	 \
}while(0)
#endif


#define OPEN LOW
#define CLOSED HIGH

typedef char KeypadEvent;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef struct {
    byte rows;
    byte columns;
} KeypadSize;

#define LIST_MAX 10		// Max number of keys on the active list.
#define MAPSIZE 10		// MAPSIZE is the number of rows (times 16 columns)
#define makeKeymap(x) ((char*)x)


//class Keypad : public Key, public HAL_obj {
class Keypad : public Key {
public:

	Keypad(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols);

	virtual void pin_mode(byte pinNum, byte mode) { pinMode(pinNum, mode); }
	virtual void pin_write(byte pinNum, boolean level) { digitalWrite(pinNum, level); }
	virtual int  pin_read(byte pinNum) { return digitalRead(pinNum); }

	uint bitMap[MAPSIZE];	// 10 row x 16 column array of bits. Except Due which has 32 columns.
	Key key[LIST_MAX];
	unsigned long holdTimer;

	char getKey();
	bool getKeys();
	KeyState getState();
	void begin(char *userKeymap);
	bool isPressed(char keyChar);
	void setDebounceTime(uint);
	void setHoldTime(uint);
	void addEventListener(void (*listener)(char));
	int findInList(char keyChar);
	int findInList(int keyCode);
	char waitForKey();
	bool keyStateChanged();
	byte numKeys();

private:
	unsigned long startTime;
	char *keymap;
    byte *rowPins;
    byte *columnPins;
	KeypadSize sizeKpd;
	uint debounceTime;
	uint holdTime;
	bool single_key;

	void scanKeys();
	bool updateList();
	void nextKeyState(byte n, boolean button);
	void transitionTo(byte n, KeyState nextState);
	void (*keypadEventListener)(char);
};

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*@author Olav Kallhovd*/ 
#ifndef HX711_ADC_config_h
#define HX711_ADC_config_h

//number of samples in moving average dataset, value must be 1, 2, 4, 8, 16, 32, 64 or 128.
#define SAMPLES 					16		//default value: 16

//adds extra sample(s) to the dataset and ignore peak high/low sample, value must be 0 or 1.
#define IGN_HIGH_SAMPLE 			1		//default value: 1
#define IGN_LOW_SAMPLE 				1		//default value: 1

//microsecond delay after writing sck pin high or low. This delay could be required for faster mcu's.
//So far the only mcu reported to need this delay is the ESP32 (issue #35), both the Arduino Due and ESP8266 seems to run fine without it.
//Change the value to '1' to enable the delay.
#define SCK_DELAY					0		//default value: 0

//if you have some other time consuming (>60μs) interrupt routines that trigger while the sck pin is high, this could unintentionally set the HX711 into "power down" mode
//if required you can change the value to '1' to disable interrupts when writing to the sck pin.
#define SCK_DISABLE_INTERRUPTS		0		//default value: 0

#endif

#ifndef HX711_ADC_h
#define HX711_ADC_h

#define DATA_SET 	SAMPLES + IGN_HIGH_SAMPLE + IGN_LOW_SAMPLE // total samples in memory

#if (SAMPLES  != 1) & (SAMPLES  != 2) & (SAMPLES  != 4) & (SAMPLES  != 8) & (SAMPLES  != 16) & (SAMPLES  != 32) & (SAMPLES  != 64) & (SAMPLES  != 128)
	#error "number of SAMPLES not valid!"
#endif

#if (SAMPLES  == 1) & ((IGN_HIGH_SAMPLE  != 0) | (IGN_LOW_SAMPLE  != 0))
	#error "number of SAMPLES not valid!"
#endif

#if 		(SAMPLES == 1)
#define 	DIVB 0
#elif 		(SAMPLES == 2)
#define 	DIVB 1
#elif 		(SAMPLES == 4)
#define 	DIVB 2
#elif  		(SAMPLES == 8)
#define 	DIVB 3
#elif  		(SAMPLES == 16)
#define 	DIVB 4
#elif  		(SAMPLES == 32)
#define 	DIVB 5
#elif  		(SAMPLES == 64)
#define 	DIVB 6
#elif  		(SAMPLES == 128)
#define 	DIVB 7
#endif

#define SIGNAL_TIMEOUT	100

class HX711_ADC
{	
		
	public:
		HX711_ADC(uint8_t dout, uint8_t sck); 		//constructor
		void setGain(uint8_t gain = 128); 			//value must be 32, 64 or 128*
		void begin();								//set pinMode, HX711 gain and power up the HX711
		void begin(uint8_t gain);					//set pinMode, HX711 selected gain and power up the HX711
		void start(unsigned long t); 					//start HX711 and do tare 
		void start(unsigned long t, bool dotare);		//start HX711, do tare if selected
		int startMultiple(unsigned long t); 			//start and do tare, multiple HX711 simultaniously
		int startMultiple(unsigned long t, bool dotare);	//start and do tare if selected, multiple HX711 simultaniously
		void tare(); 								//zero the scale, wait for tare to finnish (blocking)
		void tareNoDelay(); 						//zero the scale, initiate the tare operation to run in the background (non-blocking)
		bool getTareStatus();						//returns 'true' if tareNoDelay() operation is complete
		void setCalFactor(float cal); 				//set new calibration factor, raw data is divided by this value to convert to readable data
		float getCalFactor(); 						//returns the current calibration factor
		float getData(); 							//returns data from the moving average dataset 

		int getReadIndex(); 						//for testing and debugging
		float getConversionTime(); 					//for testing and debugging
		float getSPS();								//for testing and debugging
		bool getTareTimeoutFlag();					//for testing and debugging
		void disableTareTimeout();					//for testing and debugging
		long getSettlingTime();						//for testing and debugging
		void powerDown(); 							//power down the HX711
		void powerUp(); 							//power up the HX711
		long getTareOffset();						//get the tare offset (raw data value output without the scale "calFactor")
		void setTareOffset(long newoffset);			//set new tare offset (raw data value input without the scale "calFactor")
		uint8_t update(); 							//if conversion is ready; read out 24 bit data and add to dataset
		bool dataWaitingAsync(); 					//checks if data is available to read (no conversion yet)
		bool updateAsync(); 						//read available data and add to dataset 
		void setSamplesInUse(int samples);			//overide number of samples in use
		int getSamplesInUse();						//returns current number of samples in use
		void resetSamplesIndex();					//resets index for dataset
		bool refreshDataSet();						//Fill the whole dataset up with new conversions, i.e. after a reset/restart (this function is blocking once started)
		bool getDataSetStatus();					//returns 'true' when the whole dataset has been filled up with conversions, i.e. after a reset/restart
		float getNewCalibration(float known_mass);	//returns and sets a new calibration value (calFactor) based on a known mass input
		bool getSignalTimeoutFlag();				//returns 'true' if it takes longer time then 'SIGNAL_TIMEOUT' for the dout pin to go low after a new conversion is started
		void setReverseOutput();					//reverse the output value

	protected:
		void conversion24bit(); 					//if conversion is ready: returns 24 bit data and starts the next conversion
		long smoothedData();						//returns the smoothed data value calculated from the dataset
		uint8_t sckPin; 							//HX711 pd_sck pin
		uint8_t doutPin; 							//HX711 dout pin
		uint8_t GAIN;								//HX711 GAIN
		float calFactor = 1.0;						//calibration factor as given in function setCalFactor(float cal)
		float calFactorRecip = 1.0;					//reciprocal calibration factor (1/calFactor), the HX711 raw data is multiplied by this value
		volatile long dataSampleSet[DATA_SET + 1];	// dataset, make voltile if interrupt is used 
		long tareOffset = 0;
		int readIndex = 0;
		unsigned long conversionStartTime = 0;
		unsigned long conversionTime = 0;
		uint8_t isFirst = 1;
		uint8_t tareTimes = 0;
		uint8_t divBit = DIVB;
		const uint8_t divBitCompiled = DIVB;
		bool doTare = 0;
		bool startStatus = 0;
		unsigned long startMultipleTimeStamp = 0;
		unsigned long startMultipleWaitTime = 0;
		uint8_t convRslt = 0;
		bool tareStatus = 0;
		unsigned int tareTimeOut = (SAMPLES + IGN_HIGH_SAMPLE + IGN_HIGH_SAMPLE) * 150; // tare timeout time in ms, no of samples * 150ms (10SPS + 50% margin)
		bool tareTimeoutFlag = 0;
		bool tareTimeoutDisable = 0;
		int samplesInUse = SAMPLES;
		long lastSmoothedData = 0;
		bool dataOutOfRange = 0;
		unsigned long lastDoutLowTime = 0;
		bool signalTimeoutFlag = 0;
		bool reverseVal = 0;
		bool dataWaiting = 0;
};	

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*@author YWROBOT*/
#ifndef LiquidCrystal_I2C_h
#define LiquidCrystal_I2C_h

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

class LiquidCrystal_I2C : public Print {
public:
  LiquidCrystal_I2C(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows);
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS );
  void clear();
  void home();
  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void printLeft();
  void printRight();
  void leftToRight();
  void rightToLeft();
  void shiftIncrement();
  void shiftDecrement();
  void noBacklight();
  void backlight();
  void autoscroll();
  void noAutoscroll(); 
  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 
#if defined(ARDUINO) && ARDUINO >= 100
  virtual size_t write(uint8_t);
#else
  virtual void write(uint8_t);
#endif
  void command(uint8_t);
  void init();

////compatibility API function aliases
void blink_on();						// alias for blink()
void blink_off();       					// alias for noBlink()
void cursor_on();      	 					// alias for cursor()
void cursor_off();      					// alias for noCursor()
void setBacklight(uint8_t new_val);				// alias for backlight() and nobacklight()
void load_custom_character(uint8_t char_num, uint8_t *rows);	// alias for createChar()
void printstr(const char[]);

////Unsupported API functions (not implemented in this library)
uint8_t status();
void setContrast(uint8_t new_val);
uint8_t keypad();
void setDelay(int,int);
void on();
void off();
uint8_t init_bargraph(uint8_t graphtype);
void draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end);
void draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end);
	 

private:
  void init_priv();
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void expanderWrite(uint8_t);
  void pulseEnable(uint8_t);
  uint8_t _Addr;
  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;
  uint8_t _numlines;
  uint8_t _cols;
  uint8_t _rows;
  uint8_t _backlightval;
};

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
