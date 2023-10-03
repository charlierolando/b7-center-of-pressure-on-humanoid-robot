#include <Arduino.h>
#include <inttypes.h>
#include "Barelang7_COP.h"

// default constructor
Key::Key() {
	kchar = NO_KEY;
	kstate = IDLE;
	stateChanged = false;
}

// constructor
Key::Key(char userKeyChar) {
	kchar = userKeyChar;
	kcode = -1;
	kstate = IDLE;
	stateChanged = false;
}


void Key::key_update (char userKeyChar, KeyState userState, boolean userStatus) {
	kchar = userKeyChar;
	kstate = userState;
	stateChanged = userStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Keypad::Keypad(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols) {
	rowPins = row;
	columnPins = col;
	sizeKpd.rows = numRows;
	sizeKpd.columns = numCols;

	begin(userKeymap);

	setDebounceTime(10);
	setHoldTime(500);
	keypadEventListener = 0;

	startTime = 0;
	single_key = false;
}

// Let the user define a keymap - assume the same row/column count as defined in constructor
void Keypad::begin(char *userKeymap) {
    keymap = userKeymap;
}

// Returns a single key only. Retained for backwards compatibility.
char Keypad::getKey() {
	single_key = true;

	if (getKeys() && key[0].stateChanged && (key[0].kstate==PRESSED))
		return key[0].kchar;
	
	single_key = false;

	return NO_KEY;
}

// Populate the key list.
bool Keypad::getKeys() {
	bool keyActivity = false;

	// Limit how often the keypad is scanned. This makes the loop() run 10 times as fast.
	if ( (millis()-startTime)>debounceTime ) {
		scanKeys();
		keyActivity = updateList();
		startTime = millis();
	}

	return keyActivity;
}

// Private : Hardware scan
void Keypad::scanKeys() {
	// Re-intialize the row pins. Allows sharing these pins with other hardware.
	for (byte r=0; r<sizeKpd.rows; r++) {
		pin_mode(rowPins[r],INPUT_PULLUP);
	}

	// bitMap stores ALL the keys that are being pressed.
	for (byte c=0; c<sizeKpd.columns; c++) {
		pin_mode(columnPins[c],OUTPUT);
		pin_write(columnPins[c], LOW);	// Begin column pulse output.
		for (byte r=0; r<sizeKpd.rows; r++) {
			bitWrite(bitMap[r], c, !pin_read(rowPins[r]));  // keypress is active low so invert to high.
		}
		// Set pin to high impedance input. Effectively ends column pulse.
		pin_write(columnPins[c],HIGH);
		pin_mode(columnPins[c],INPUT);
	}
}

// Manage the list without rearranging the keys. Returns true if any keys on the list changed state.
bool Keypad::updateList() {

	bool anyActivity = false;

	// Delete any IDLE keys
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kstate==IDLE) {
			key[i].kchar = NO_KEY;
			key[i].kcode = -1;
			key[i].stateChanged = false;
		}
	}

	// Add new keys to empty slots in the key list.
	for (byte r=0; r<sizeKpd.rows; r++) {
		for (byte c=0; c<sizeKpd.columns; c++) {
			boolean button = bitRead(bitMap[r],c);
			char keyChar = keymap[r * sizeKpd.columns + c];
			int keyCode = r * sizeKpd.columns + c;
			int idx = findInList (keyCode);
			// Key is already on the list so set its next state.
			if (idx > -1)	{
				nextKeyState(idx, button);
			}
			// Key is NOT on the list so add it.
			if ((idx == -1) && button) {
				for (byte i=0; i<LIST_MAX; i++) {
					if (key[i].kchar==NO_KEY) {		// Find an empty slot or don't add key to list.
						key[i].kchar = keyChar;
						key[i].kcode = keyCode;
						key[i].kstate = IDLE;		// Keys NOT on the list have an initial state of IDLE.
						nextKeyState (i, button);
						break;	// Don't fill all the empty slots with the same key.
					}
				}
			}
		}
	}

	// Report if the user changed the state of any key.
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].stateChanged) anyActivity = true;
	}

	return anyActivity;
}

// Private
// This function is a state machine but is also used for debouncing the keys.
void Keypad::nextKeyState(byte idx, boolean button) {
	key[idx].stateChanged = false;

	switch (key[idx].kstate) {
		case IDLE:
			if (button==CLOSED) {
				transitionTo (idx, PRESSED);
				holdTimer = millis(); }		// Get ready for next HOLD state.
			break;
		case PRESSED:
			if ((millis()-holdTimer)>holdTime)	// Waiting for a key HOLD...
				transitionTo (idx, HOLD);
			else if (button==OPEN)				// or for a key to be RELEASED.
				transitionTo (idx, RELEASED);
			break;
		case HOLD:
			if (button==OPEN)
				transitionTo (idx, RELEASED);
			break;
		case RELEASED:
			transitionTo (idx, IDLE);
			break;
	}
}

// New in 2.1
bool Keypad::isPressed(char keyChar) {
	for (byte i=0; i<LIST_MAX; i++) {
		if ( key[i].kchar == keyChar ) {
			if ( (key[i].kstate == PRESSED) && key[i].stateChanged )
				return true;
		}
	}
	return false;	// Not pressed.
}

// Search by character for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int Keypad::findInList (char keyChar) {
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kchar == keyChar) {
			return i;
		}
	}
	return -1;
}

// Search by code for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int Keypad::findInList (int keyCode) {
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kcode == keyCode) {
			return i;
		}
	}
	return -1;
}

// New in 2.0
char Keypad::waitForKey() {
	char waitKey = NO_KEY;
	while( (waitKey = getKey()) == NO_KEY );	// Block everything while waiting for a keypress.
	return waitKey;
}

// Backwards compatibility function.
KeyState Keypad::getState() {
	return key[0].kstate;
}

// The end user can test for any changes in state before deciding
// if any variables, etc. needs to be updated in their code.
bool Keypad::keyStateChanged() {
	return key[0].stateChanged;
}

// The number of keys on the key list, key[LIST_MAX], equals the number
// of bytes in the key list divided by the number of bytes in a Key object.
byte Keypad::numKeys() {
	return sizeof(key)/sizeof(Key);
}

// Minimum debounceTime is 1 mS. Any lower *will* slow down the loop().
void Keypad::setDebounceTime(uint debounce) {
	debounce<1 ? debounceTime=1 : debounceTime=debounce;
}

void Keypad::setHoldTime(uint hold) {
    holdTime = hold;
}

void Keypad::addEventListener(void (*listener)(char)){
	keypadEventListener = listener;
}

void Keypad::transitionTo(byte idx, KeyState nextState) {
	key[idx].kstate = nextState;
	key[idx].stateChanged = true;

	// Sketch used the getKey() function.
	// Calls keypadEventListener only when the first key in slot 0 changes state.
	if (single_key)  {
	  	if ( (keypadEventListener!=NULL) && (idx==0) )  {
			keypadEventListener(key[0].kchar);
		}
	}
	// Sketch used the getKeys() function.
	// Calls keypadEventListener on any key that changes state.
	else {
	  	if (keypadEventListener!=NULL)  {
			keypadEventListener(key[idx].kchar);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HX711_ADC::HX711_ADC(uint8_t dout, uint8_t sck) //constructor
{ 	
	doutPin = dout;
	sckPin = sck;
} 

void HX711_ADC::setGain(uint8_t gain)  //value should be 32, 64 or 128*
{
	if(gain < 64) GAIN = 2; //32, channel B
	else if(gain < 128) GAIN = 3; //64, channel A
	else GAIN = 1; //128, channel A
}

//set pinMode, HX711 gain and power up the HX711
void HX711_ADC::begin()
{
	pinMode(sckPin, OUTPUT);
	pinMode(doutPin, INPUT);
	setGain(128);
	powerUp();
}

//set pinMode, HX711 selected gain and power up the HX711
void HX711_ADC::begin(uint8_t gain)
{
	pinMode(sckPin, OUTPUT);
	pinMode(doutPin, INPUT);
	setGain(gain);
	powerUp();
}

/*  start(t): 
*	will do conversions continuously for 't' +400 milliseconds (400ms is min. settling time at 10SPS). 
*   Running this for 1-5s in setup() - before tare() seems to improve the tare accuracy */
void HX711_ADC::start(unsigned long t)
{
	t += 400;
	lastDoutLowTime = millis();
	while(millis() < t) 
	{
		update();
		yield();
	}
	tare();
	tareStatus = 0;
}	

/*  start(t, dotare) with selectable tare:
*	will do conversions continuously for 't' +400 milliseconds (400ms is min. settling time at 10SPS). 
*   Running this for 1-5s in setup() - before tare() seems to improve the tare accuracy. */
void HX711_ADC::start(unsigned long t, bool dotare)
{
	t += 400;
	lastDoutLowTime = millis();
	while(millis() < t) 
	{
		update();
		yield();
	}
	if (dotare)
	{
		tare();
		tareStatus = 0;
	}
}	

/*  startMultiple(t): use this if you have more than one load cell and you want to do tare and stabilization simultaneously.
*	Will do conversions continuously for 't' +400 milliseconds (400ms is min. settling time at 10SPS). 
*   Running this for 1-5s in setup() - before tare() seems to improve the tare accuracy */
int HX711_ADC::startMultiple(unsigned long t)
{
	tareTimeoutFlag = 0;
	lastDoutLowTime = millis();
	if(startStatus == 0) {
		if(isFirst) {
			startMultipleTimeStamp = millis();
			if (t < 400) 
			{
				startMultipleWaitTime = t + 400; //min time for HX711 to be stable
			} 
			else 
			{
				startMultipleWaitTime = t;
			}
			isFirst = 0;
		}	
		if((millis() - startMultipleTimeStamp) < startMultipleWaitTime) {
			update(); //do conversions during stabilization time
			yield();
			return 0;
		}
		else { //do tare after stabilization time is up
			static unsigned long timeout = millis() + tareTimeOut;
			doTare = 1;
			update();
			if(convRslt == 2) 
			{	
				doTare = 0;
				convRslt = 0;
				startStatus = 1;
			}
			if (!tareTimeoutDisable) 
			{
				if (millis() > timeout) 
				{ 
				tareTimeoutFlag = 1;
				return 1; // Prevent endless loop if no HX711 is connected
				}
			}
		}
	}
	return startStatus;
}

/*  startMultiple(t, dotare) with selectable tare: 
*	use this if you have more than one load cell and you want to (do tare and) stabilization simultaneously.
*	Will do conversions continuously for 't' +400 milliseconds (400ms is min. settling time at 10SPS). 
*   Running this for 1-5s in setup() - before tare() seems to improve the tare accuracy */
int HX711_ADC::startMultiple(unsigned long t, bool dotare)
{
	tareTimeoutFlag = 0;
	lastDoutLowTime = millis();
	if(startStatus == 0) {
		if(isFirst) {
			startMultipleTimeStamp = millis();
			if (t < 400) 
			{
				startMultipleWaitTime = t + 400; //min time for HX711 to be stable
			} 
			else 
			{
				startMultipleWaitTime = t;
			}
			isFirst = 0;
		}	
		if((millis() - startMultipleTimeStamp) < startMultipleWaitTime) {
			update(); //do conversions during stabilization time
			yield();
			return 0;
		}
		else { //do tare after stabilization time is up
			if (dotare) 
			{
				static unsigned long timeout = millis() + tareTimeOut;
				doTare = 1;
				update();
				if(convRslt == 2) 
				{	
					doTare = 0;
					convRslt = 0;
					startStatus = 1;
				}
				if (!tareTimeoutDisable) 
				{
					if (millis() > timeout) 
					{ 
					tareTimeoutFlag = 1;
					return 1; // Prevent endless loop if no HX711 is connected
					}
				}
			}
			else return 1;
		}
	}
	return startStatus;
}

//zero the scale, wait for tare to finnish (blocking)
void HX711_ADC::tare() 
{
	uint8_t rdy = 0;
	doTare = 1;
	tareTimes = 0;
	tareTimeoutFlag = 0;
	unsigned long timeout = millis() + tareTimeOut;
	while(rdy != 2) 
	{
		rdy = update();
		if (!tareTimeoutDisable) 
		{
			if (millis() > timeout) 
			{ 
				tareTimeoutFlag = 1;
				break; // Prevent endless loop if no HX711 is connected
			}
		}
		yield();
	}
}

//zero the scale, initiate the tare operation to run in the background (non-blocking)
void HX711_ADC::tareNoDelay() 
{
	doTare = 1;
	tareTimes = 0;
	tareStatus = 0;
}

//set new calibration factor, raw data is divided by this value to convert to readable data
void HX711_ADC::setCalFactor(float cal) 
{
	calFactor = cal;
	calFactorRecip = 1/calFactor;
}

//returns 'true' if tareNoDelay() operation is complete
bool HX711_ADC::getTareStatus() 
{
	bool t = tareStatus;
	tareStatus = 0;
	return t;
}

//returns the current calibration factor
float HX711_ADC::getCalFactor() 
{
	return calFactor;
}

//call the function update() in loop or from ISR
//if conversion is ready; read out 24 bit data and add to dataset, returns 1
//if tare operation is complete, returns 2
//else returns 0
uint8_t HX711_ADC::update() 
{
	byte dout = digitalRead(doutPin); //check if conversion is ready
	if (!dout) 
	{
		conversion24bit();
		lastDoutLowTime = millis();
		signalTimeoutFlag = 0;
	}
	else 
	{
		//if (millis() > (lastDoutLowTime + SIGNAL_TIMEOUT))
		if (millis() - lastDoutLowTime > SIGNAL_TIMEOUT)
		{
			signalTimeoutFlag = 1;
		}
		convRslt = 0;
	}
	return convRslt;
}

// call the function dataWaitingAsync() in loop or from ISR to check if new data is available to read
// if conversion is ready, just call updateAsync() to read out 24 bit data and add to dataset
// returns 1 if data available , else 0
bool HX711_ADC::dataWaitingAsync() 
{
	if (dataWaiting) { lastDoutLowTime = millis(); return 1; }
	byte dout = digitalRead(doutPin); //check if conversion is ready
	if (!dout) 
	{
		dataWaiting = true;
		lastDoutLowTime = millis();
		signalTimeoutFlag = 0;
		return 1;
	}
	else
	{
		//if (millis() > (lastDoutLowTime + SIGNAL_TIMEOUT))
		if (millis() - lastDoutLowTime > SIGNAL_TIMEOUT)
		{
			signalTimeoutFlag = 1;
		}
		convRslt = 0;
	}
	return 0;
}

// if data is available call updateAsync() to convert it and add it to the dataset.
// call getData() to get latest value
bool HX711_ADC::updateAsync() 
{
	if (dataWaiting) { 
		conversion24bit();
		dataWaiting = false;
		return true;
	}
	return false;

}

float HX711_ADC::getData() // return fresh data from the moving average dataset
{
	long data = 0;
	lastSmoothedData = smoothedData();
	data = lastSmoothedData - tareOffset ;
	float x = (float)data * calFactorRecip;
	return x;
}

long HX711_ADC::smoothedData() 
{
	long data = 0;
	long L = 0xFFFFFF;
	long H = 0x00;
	for (uint8_t r = 0; r < (samplesInUse + IGN_HIGH_SAMPLE + IGN_LOW_SAMPLE); r++) 
	{
		#if IGN_LOW_SAMPLE
		if (L > dataSampleSet[r]) L = dataSampleSet[r]; // find lowest value
		#endif
		#if IGN_HIGH_SAMPLE
		if (H < dataSampleSet[r]) H = dataSampleSet[r]; // find highest value
		#endif
		data += dataSampleSet[r];
	}
	#if IGN_LOW_SAMPLE 
	data -= L; //remove lowest value
	#endif
	#if IGN_HIGH_SAMPLE 
	data -= H; //remove highest value
	#endif
	//return data;
	return (data >> divBit);

}

void HX711_ADC::conversion24bit()  //read 24 bit data, store in dataset and start the next conversion
{
	conversionTime = micros() - conversionStartTime;
	conversionStartTime = micros();
	unsigned long data = 0;
	uint8_t dout;
	convRslt = 0;
	if(SCK_DISABLE_INTERRUPTS) noInterrupts();

	for (uint8_t i = 0; i < (24 + GAIN); i++) 
	{ 	//read 24 bit data + set gain and start next conversion
		digitalWrite(sckPin, 1);
		if(SCK_DELAY) delayMicroseconds(1); // could be required for faster mcu's, set value in config.h
		digitalWrite(sckPin, 0);
		if (i < (24)) 
		{
			dout = digitalRead(doutPin);
			data = (data << 1) | dout;
		} else {
			if(SCK_DELAY) delayMicroseconds(1); // could be required for faster mcu's, set value in config.h
		}
	}
	if(SCK_DISABLE_INTERRUPTS) interrupts();
	
	/*
	The HX711 output range is min. 0x800000 and max. 0x7FFFFF (the value rolls over).
	In order to convert the range to min. 0x000000 and max. 0xFFFFFF,
	the 24th bit must be changed from 0 to 1 or from 1 to 0.
	*/
	data = data ^ 0x800000; // flip the 24th bit 
	
	if (data > 0xFFFFFF) 
	{
		dataOutOfRange = 1;
		//Serial.println("dataOutOfRange");
	}
	if (reverseVal) {
		data = 0xFFFFFF - data;
	}
	if (readIndex == samplesInUse + IGN_HIGH_SAMPLE + IGN_LOW_SAMPLE - 1) 
	{
		readIndex = 0;
	}
	else 
	{
		readIndex++;
	}
	if(data > 0)  
	{
		convRslt++;
		dataSampleSet[readIndex] = (long)data;
		if(doTare) 
		{
			if (tareTimes < DATA_SET) 
			{
				tareTimes++;
			}
			else 
			{
				tareOffset = smoothedData();
				tareTimes = 0;
				doTare = 0;
				tareStatus = 1;
				convRslt++;
			}
		}
	}
}

//power down the HX711
void HX711_ADC::powerDown() 
{
	digitalWrite(sckPin, LOW);
	digitalWrite(sckPin, HIGH);
}

//power up the HX711
void HX711_ADC::powerUp() 
{
	digitalWrite(sckPin, LOW);
}

//get the tare offset (raw data value output without the scale "calFactor")
long HX711_ADC::getTareOffset() 
{
	return tareOffset;
}

//set new tare offset (raw data value input without the scale "calFactor")
void HX711_ADC::setTareOffset(long newoffset)
{
	tareOffset = newoffset;
}

//for testing and debugging:
//returns current value of dataset readIndex
int HX711_ADC::getReadIndex()
{
	return readIndex;
}

//for testing and debugging:
//returns latest conversion time in millis
float HX711_ADC::getConversionTime()
{
	return conversionTime/1000.0;
}

//for testing and debugging:
//returns the HX711 conversions ea seconds based on the latest conversion time. 
//The HX711 can be set to 10SPS or 80SPS. For general use the recommended setting is 10SPS.
float HX711_ADC::getSPS()
{
	float sps = 1000000.0/conversionTime;
	return sps;
}

//for testing and debugging:
//returns the tare timeout flag from the last tare operation. 
//0 = no timeout, 1 = timeout
bool HX711_ADC::getTareTimeoutFlag() 
{
	return tareTimeoutFlag;
}

void HX711_ADC::disableTareTimeout()
{
	tareTimeoutDisable = 1;
}

long HX711_ADC::getSettlingTime() 
{
	long st = getConversionTime() * DATA_SET;
	return st;
}

//overide the number of samples in use
//value is rounded down to the nearest valid value
void HX711_ADC::setSamplesInUse(int samples)
{
	int old_value = samplesInUse;
	
	if(samples <= SAMPLES)
	{
		if(samples == 0) //reset to the original value
		{
			divBit = divBitCompiled;
		} 
		else
		{
			samples >>= 1;
			for(divBit = 0; samples != 0; samples >>= 1, divBit++);
		}
		samplesInUse = 1 << divBit;
		
		//replace the value of all samples in use with the last conversion value
		if(samplesInUse != old_value) 
		{
			for (uint8_t r = 0; r < samplesInUse + IGN_HIGH_SAMPLE + IGN_LOW_SAMPLE; r++) 
			{
				dataSampleSet[r] = lastSmoothedData;
			}
			readIndex = 0;
		}
	}
}

//returns the current number of samples in use.
int HX711_ADC::getSamplesInUse()
{
	return samplesInUse;
}

//resets index for dataset
void HX711_ADC::resetSamplesIndex()
{
	readIndex = 0;
}

//Fill the whole dataset up with new conversions, i.e. after a reset/restart (this function is blocking once started)
bool HX711_ADC::refreshDataSet()
{
	int s = getSamplesInUse() + IGN_HIGH_SAMPLE + IGN_LOW_SAMPLE; // get number of samples in dataset
	resetSamplesIndex();
	while ( s > 0 ) {
		update();
		yield();
		if (digitalRead(doutPin) == LOW) { // HX711 dout pin is pulled low when a new conversion is ready
			getData(); // add data to the set and start next conversion
			s--;
		}
	}
	return true;
}

//returns 'true' when the whole dataset has been filled up with conversions, i.e. after a reset/restart.
bool HX711_ADC::getDataSetStatus()
{
	bool i = false;
	if (readIndex == samplesInUse + IGN_HIGH_SAMPLE + IGN_LOW_SAMPLE - 1) 
	{
		i = true;
	}
	return i;
}

//returns and sets a new calibration value (calFactor) based on a known mass input
float HX711_ADC::getNewCalibration(float known_mass)
{
	float readValue = getData();
	float exist_calFactor = getCalFactor();
	float new_calFactor;
	new_calFactor = (readValue * exist_calFactor) / known_mass;
	setCalFactor(new_calFactor);
    return new_calFactor;
}

//returns 'true' if it takes longer time then 'SIGNAL_TIMEOUT' for the dout pin to go low after a new conversion is started
bool HX711_ADC::getSignalTimeoutFlag()
{
	return signalTimeoutFlag;
}

//reverse the output value (flip positive/negative value)
//tare/zero-offset must be re-set after calling this.
void HX711_ADC::setReverseOutput() {
	reverseVal = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#define printIIC(args)	Wire.write(args)
inline size_t LiquidCrystal_I2C::write(uint8_t value) {
	send(value, Rs);
	return 1;
}

#else
#include "WProgram.h"

#define printIIC(args)	Wire.send(args)
inline void LiquidCrystal_I2C::write(uint8_t value) {
	send(value, Rs);
}

#endif
#include "Wire.h"



// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LiquidCrystal_I2C::LiquidCrystal_I2C(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows)
{
  _Addr = lcd_Addr;
  _cols = lcd_cols;
  _rows = lcd_rows;
  _backlightval = LCD_NOBACKLIGHT;
}

void LiquidCrystal_I2C::init(){
	init_priv();
}

void LiquidCrystal_I2C::init_priv()
{
	Wire.begin();
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	begin(_cols, _rows);  
}

void LiquidCrystal_I2C::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	delay(50); 
  
	// Now we pull both RS and R/W low to begin commands
	expanderWrite(_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	delay(1000);

  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46
	
	  // we start in 8bit mode, try to set 4 bit mode
   write4bits(0x03 << 4);
   delayMicroseconds(4500); // wait min 4.1ms
   
   // second try
   write4bits(0x03 << 4);
   delayMicroseconds(4500); // wait min 4.1ms
   
   // third go!
   write4bits(0x03 << 4); 
   delayMicroseconds(150);
   
   // finally, set to 4-bit interface
   write4bits(0x02 << 4); 


	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);  
	
	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();
	
	// clear it off
	clear();
	
	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);
	
	home();
  
}

/********** high level commands, for the user! */
void LiquidCrystal_I2C::clear(){
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidCrystal_I2C::home(){
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidCrystal_I2C::setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > _numlines ) {
		row = _numlines-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystal_I2C::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_I2C::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal_I2C::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_I2C::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal_I2C::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_I2C::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal_I2C::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LiquidCrystal_I2C::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal_I2C::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal_I2C::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal_I2C::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal_I2C::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal_I2C::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
		write(charmap[i]);
	}
}

// Turn the (optional) backlight off/on
void LiquidCrystal_I2C::noBacklight(void) {
	_backlightval=LCD_NOBACKLIGHT;
	expanderWrite(0);
}

void LiquidCrystal_I2C::backlight(void) {
	_backlightval=LCD_BACKLIGHT;
	expanderWrite(0);
}



/*********** mid level commands, for sending data/cmds */

inline void LiquidCrystal_I2C::command(uint8_t value) {
	send(value, 0);
}


/************ low level data pushing commands **********/

// write either command or data
void LiquidCrystal_I2C::send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
       write4bits((highnib)|mode);
	write4bits((lownib)|mode); 
}

void LiquidCrystal_I2C::write4bits(uint8_t value) {
	expanderWrite(value);
	pulseEnable(value);
}

void LiquidCrystal_I2C::expanderWrite(uint8_t _data){                                        
	Wire.beginTransmission(_Addr);
	printIIC((int)(_data) | _backlightval);
	Wire.endTransmission();   
}

void LiquidCrystal_I2C::pulseEnable(uint8_t _data){
	expanderWrite(_data | En);	// En high
	delayMicroseconds(1);		// enable pulse must be >450ns
	
	expanderWrite(_data & ~En);	// En low
	delayMicroseconds(50);		// commands need > 37us to settle
} 


// Alias functions

void LiquidCrystal_I2C::cursor_on(){
	cursor();
}

void LiquidCrystal_I2C::cursor_off(){
	noCursor();
}

void LiquidCrystal_I2C::blink_on(){
	blink();
}

void LiquidCrystal_I2C::blink_off(){
	noBlink();
}

void LiquidCrystal_I2C::load_custom_character(uint8_t char_num, uint8_t *rows){
		createChar(char_num, rows);
}

void LiquidCrystal_I2C::setBacklight(uint8_t new_val){
	if(new_val){
		backlight();		// turn backlight on
	}else{
		noBacklight();		// turn backlight off
	}
}

void LiquidCrystal_I2C::printstr(const char c[]){
	//This function is not identical to the function used for "real" I2C displays
	//it's here so the user sketch doesn't have to be changed 
	print(c);
}


// unsupported API functions
void LiquidCrystal_I2C::off(){}
void LiquidCrystal_I2C::on(){}
void LiquidCrystal_I2C::setDelay (int cmdDelay,int charDelay) {}
uint8_t LiquidCrystal_I2C::status(){return 0;}
uint8_t LiquidCrystal_I2C::keypad (){return 0;}
uint8_t LiquidCrystal_I2C::init_bargraph(uint8_t graphtype){return 0;}
void LiquidCrystal_I2C::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end){}
void LiquidCrystal_I2C::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_row_end){}
void LiquidCrystal_I2C::setContrast(uint8_t new_val){}
