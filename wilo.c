//
// Wilo EMHIL 505 EM - open source firmware
// Jakub Strnad 2025
//
// MCU is Renesas H8/36077GFZV (5V version)
// IGBT module is Mitsubishi PS21963-4C
//
// - use Renesas C/C++ Compiler Package for H8SX, H8S, H8 Family to compile this source code
// - interrupt handlers are included here -> remove intprg.c from your project
//
// - use Renesas Flash Development Toolkit to write the flash in Boot Mode
// - pin 7 of J301 must be shorted to GND to enable Boot Mode
//
// WARNING: There is no way to revert back to the original firmware,
// WARNING: I was not able to read the original firmware from the MCU.
// WARNING: Use at your own risk !
                  
#include "typedefine.h"
#include "iodefine.h"
#include <stdarg.h>
#include <stdio.h>
#include <machine.h>
#include <mathf.h>

#define TEMP_K 273.15
#define TEMP_0 298.15
#define TEMP_R0 100000.0
#define TEMP_B 3950.0
#define TEMP_RDIV 68000.0

#define PWM_MAX 2000

#define FAULT_SHORT 0x01
#define FAULT_PRESSURE 0x02
#define FAULT_UV 0x04
#define FAULT_OV 0x08
#define FAULT_OC 0x10
#define FAULT_XTAL 0x20
#define FAULT_TEMP 0x40

enum keyEnum { KEY_NONE, KEY_RUN, KEY_AUTO, KEY_UP, KEY_DOWN, KEY_MENU, KEY_ENTER, KEY_INVALID };

const int8_t svpwmU[] = {
	5, 11, 16, 22, 27, 32, 38, 43, 48, 53, 59, 64, 69, 74, 79, 84,
	89, 94, 99, 104, 108, 111, 112, 114, 115, 117, 118, 119, 120, 121, 122, 123,
	123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 126, 126,
	125, 125, 124, 124, 123, 122, 121, 120, 119, 118, 117, 116, 114, 113, 112, 110,
	112, 113, 114, 116, 117, 118, 119, 120, 121, 122, 123, 124, 124, 125, 125, 126,
	126, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 123,
	122, 121, 120, 119, 118, 117, 115, 114, 112, 111, 108, 104, 99, 94, 89, 84,
	79, 74, 69, 64, 59, 53, 48, 43, 38, 32, 27, 22, 16, 11, 5, 0,
	-5, -11, -16, -22, -27, -32, -38, -43, -48, -53, -59, -64, -69, -74, -79, -84,
	-89, -94, -99, -104, -108, -111, -112, -114, -115, -117, -118, -119, -120, -121, -122, -123,
	-123, -124, -125, -125, -126, -126, -126, -127, -127, -127, -127, -127, -127, -127, -126, -126,
	-125, -125, -124, -124, -123, -122, -121, -120, -119, -118, -117, -116, -114, -113, -112, -110,
	-112, -113, -114, -116, -117, -118, -119, -120, -121, -122, -123, -124, -124, -125, -125, -126,
	-126, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -123,
	-122, -121, -120, -119, -118, -117, -115, -114, -112, -111, -108, -104, -99, -94, -89, -84,
	-79, -74, -69, -64, -59, -53, -48, -43, -38, -32, -27, -22, -16, -11, -5, 0
};

const int8_t svpwmV[] = {
	127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118, 117,
	116, 115, 113, 112, 111, 107, 102, 97, 92, 87, 83, 77, 72, 67, 62, 57,
	52, 46, 41, 36, 30, 25, 20, 14, 9, 4, -2, -7, -13, -18, -23, -29,
	-34, -39, -45, -50, -55, -60, -66, -71, -76, -81, -86, -91, -96, -101, -105, -110,
	-112, -113, -114, -116, -117, -118, -119, -120, -121, -122, -123, -124, -124, -125, -125, -126,
	-126, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -123,
	-122, -121, -120, -119, -118, -117, -115, -114, -112, -111, -111, -112, -113, -115, -116, -117,
	-118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127, -127,
	-127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118, -117,
	-116, -115, -113, -112, -111, -107, -102, -97, -92, -87, -83, -77, -72, -67, -62, -57,
	-52, -46, -41, -36, -30, -25, -20, -14, -9, -4, 2, 7, 13, 18, 23, 29,
	34, 39, 45, 50, 55, 60, 66, 71, 76, 81, 86, 91, 96, 101, 105, 110,
	112, 113, 114, 116, 117, 118, 119, 120, 121, 122, 123, 124, 124, 125, 125, 126,
	126, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 123,
	122, 121, 120, 119, 118, 117, 115, 114, 112, 111, 111, 112, 113, 115, 116, 117,
	118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127
};

const int8_t svpwmW[] = {
	-127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118, -117,
	-116, -115, -113, -112, -111, -111, -112, -114, -115, -117, -118, -119, -120, -121, -122, -123,
	-123, -124, -125, -125, -126, -126, -126, -127, -127, -127, -127, -127, -127, -127, -126, -126,
	-125, -125, -124, -124, -123, -122, -121, -120, -119, -118, -117, -116, -114, -113, -112, -110,
	-105, -101, -96, -91, -86, -81, -76, -71, -66, -60, -55, -50, -45, -39, -34, -29,
	-23, -18, -13, -7, -2, 4, 9, 14, 20, 25, 30, 36, 41, 46, 52, 57,
	62, 67, 72, 77, 83, 87, 92, 97, 102, 107, 111, 112, 113, 115, 116, 117,
	118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127,
	127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118, 117,
	116, 115, 113, 112, 111, 111, 112, 114, 115, 117, 118, 119, 120, 121, 122, 123,
	123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 126, 126,
	125, 125, 124, 124, 123, 122, 121, 120, 119, 118, 117, 116, 114, 113, 112, 110,
	105, 101, 96, 91, 86, 81, 76, 71, 66, 60, 55, 50, 45, 39, 34, 29,
	23, 18, 13, 7, 2, -4, -9, -14, -20, -25, -30, -36, -41, -46, -52, -57,
	-62, -67, -72, -77, -83, -87, -92, -97, -102, -107, -111, -112, -113, -115, -116, -117,
	-118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127, -127
};

// xtal clock
uint16_t clockWait = 65535;
uint16_t startTCWD, startTCSRWD;

// menu
const char eepSign[] = "Wilo MT6";

struct sParamDef {
	uint8_t eepAddr;
	char name[17];
	char unit[5];
	int8_t f; // fractional digits
	int16_t def, min, max;
};

#define N_PARAM (sizeof(paramDef)/sizeof(paramDef[0]))

const struct sParamDef paramDef[] = {
/*  0 */	{ 0x08, "ON pressure", "bar", 1, 25, 5, 45 },
/*  1 */	{ 0x0a, "OFF pressure", "bar", 1, 30, 10, 50 },
/*  2 */	{ 0x0c, "OFF delay", "s", 0, 3, 0, 15 },
/*  3 */	{ 0x0e, "Autorun", "", 0, 1, 0, 1 },
/*  4 */	{ 0x10, "Max frequency", "Hz", 0, 50, 5, 62 },
/*  5 */	{ 0x12, "Base frequency", "Hz", 0, 34, 5, 62 },
/*  6 */	{ 0x14, "Min frequency", "Hz", 0, 30, 5, 62 },
/*  7 */	{ 0x16, "Stop frequency", "Hz", 0, 1, 1, 30 },
/*  8 */	{ 0x18, "Manual frequency", "Hz", 0, 10, 1, 62 },
/*  9 */	{ 0x1a, "Rated frequency", "Hz", 0, 50, 5, 62 },
/* 10 */	{ 0x1c, "Rated voltage", "V", 0, 230, 10, 400 },
/* 11 */	{ 0x1e, "Max current", "A", 1, 50, 5, 80 },
/* 12 */	{ 0x26, "Undervoltage", "V", 0, 120, 50, 200 },
/* 13 */	{ 0x28, "Overvoltage", "V", 0, 390, 200, 400 },
/* 14 */	{ 0x2a, "Max temperature", "\001C", 0, 50, 25, 120 },
/* 15 */	{ 0x24, "Rotation dir.", "", 0, 0, 0, 1 },
/* 16 */	{ 0x20, "External switch", "", 0, 0, 0, 2 },
/* 17 */	{ 0x22, "Ignore faults", "", 0, 0, 0, 1 },
/* 18 */	{ 0x2c, "LED intensity", "", 0, 5, 1, 7 }
};

int16_t param[N_PARAM];
uint8_t autoRun, autoRunStart, extSwConfig, manualRun;
uint8_t menu, menuItem, page;
uint8_t ignFaults, rotDirParam;
int16_t itemValue;

// LCD
const uint8_t degreeSymbol[8] = {0x0c, 0x12, 0x12, 0x0c, 0x00, 0x00, 0x00, 0x00};
const uint8_t arrowSymbol[8] = {0x04, 0x0e, 0x1f, 0x00, 0x04, 0x0e, 0x1f, 0x00};
char lcdLine[2][20];
char statusLine[4][20] = {
	"#.#bar | ##Hz ###",
	" ###V #.#A ###\001C",
	" ####W     ###\001C",
	" = ######"
};
char menuLine[20] = "[######] >######";

uint16_t tDisp;
uint16_t dispFreq, dispVolt, dispCur, dispPres, dispTemp, dispPow;
float tempR;
uint8_t dispStep;

// VFD
uint16_t freq, reqFreq; // 256=62.5Hz
uint16_t maxFreq;
uint16_t baseFreq;
uint16_t minFreq;
uint16_t stopFreq;
uint16_t manualFreq;
float freqToVolt;
uint16_t freqToPwm;
uint16_t fineIndex, svpwmIndex;
uint8_t z0cnt;
int16_t pwmRatio; // 0-251
uint8_t vfdRun; // PWM output enabled
uint8_t rotDir;

// pressure sensor
uint16_t z1highWord;
uint16_t pLowWord, pHighWord;
uint32_t pTckLast;
uint8_t pOvf;
uint16_t tPres;
uint16_t pRaw[3];
int16_t pAct;
int16_t pOn = 625; // 2.5bar
int16_t pOff = 675; // 3bar
uint8_t pNew, pIndex;
uint8_t flow;
uint8_t isNewPres;

// regulator
uint8_t regOn;
uint16_t tReg, tOn, t4ms;
uint16_t vfdStopDelay = 750; // 3s

// keyboard
uint8_t key, lastKey, keyFirst;
uint16_t tKey;
uint8_t tLed, ledIntensity;

// serial port
char serialData[256];

// ADC
uint16_t adcVal[3];
uint8_t adcCnt[3];
uint16_t temp, current, voltage;
uint16_t minVolt;
uint16_t maxVolt;
uint16_t maxCur;
uint16_t maxTemp = 65535; // ???
uint16_t tVoltCalc;

// faults
uint16_t fault, scFault;
uint16_t tPresFault, tUv, tOv, tTemp;

// Modbus
uint8_t modbusReq[8], modbusReqI;
uint16_t tModbus;

// status pages
enum pageTypeEnum { PAGE_STATUS, PAGE_FAULT, PAGE_INT, PAGE_HEX8, PAGE_HEX16 };

struct sPageDef {
	uint8_t type;
	char name[17];
	uint16_t *ptr;
};

#define N_PAGE (sizeof(pageDef)/sizeof(pageDef[0]))
#define PAGE_FIRST_FAULT 2
#define PAGE_LAST_FAULT 8
const struct sPageDef pageDef[] = {
	{ PAGE_STATUS, "", 0 },
	{ PAGE_STATUS, "", 0 },
	
	{ PAGE_FAULT, "Short circuit", 0 },
	{ PAGE_FAULT, "Pressure sensor", 0 },
	{ PAGE_FAULT, "Low voltage", 0 },
	{ PAGE_FAULT, "High voltage", 0 },
	{ PAGE_FAULT, "Overcurrent", 0 },
	{ PAGE_FAULT, "Xtal Oscillator", 0 },
	{ PAGE_FAULT, "IGBT Temperature", 0 },
	
	{ PAGE_INT, "clockWait", &clockWait },
	{ PAGE_HEX16, "startTCWD", &startTCWD },
	{ PAGE_HEX16, "startTCSRWD", &startTCSRWD },
	{ PAGE_INT, "freqToPwm", &freqToPwm },
	
	{ PAGE_HEX16, "pLowWord", &pLowWord },
	{ PAGE_HEX16, "pHighWord", &pHighWord },
	{ PAGE_HEX8, "pOvf", (uint16_t *) &pOvf },
	
	{ PAGE_HEX8, "PDR1", (uint16_t *) &IO.PDR1.BYTE },
	{ PAGE_HEX8, "PDR2", (uint16_t *) &IO.PDR2.BYTE },
	{ PAGE_HEX8, "PDR3", (uint16_t *) &IO.PDR3.BYTE },
	{ PAGE_HEX8, "PDR5", (uint16_t *) &IO.PDR5.BYTE },
	{ PAGE_HEX8, "PDR6", (uint16_t *) &IO.PDR6.BYTE },
	{ PAGE_HEX8, "PDR7", (uint16_t *) &IO.PDR7.BYTE },
	{ PAGE_HEX8, "PDR8", (uint16_t *) &IO.PDR8.BYTE },
	{ PAGE_HEX8, "PDRB", (uint16_t *) &IO.PDRB.BYTE },
	{ PAGE_HEX8, "PDRC", (uint16_t *) &IO.PDRC.BYTE },

	{ PAGE_INT, "TZ0.GRA", (uint16_t *) &TZ0.GRA },
	{ PAGE_INT, "TZ0.GRB", (uint16_t *) &TZ0.GRB },
	{ PAGE_INT, "TZ0.GRC", (uint16_t *) &TZ0.GRC },
	{ PAGE_INT, "TZ0.GRD", (uint16_t *) &TZ0.GRD },
	
	{ PAGE_HEX8, "WDT.TCWD", (uint16_t *) &WDT.TCWD }

};	

void delay(uint32_t ticks) {
	while (--ticks) ;
}

void setParam(uint8_t n) {
	float r1;
	
	switch (n) {
	case 0: pOn = 364.71875 + 10.30594 * param[n]; break;
	case 1: pOff = 364.71875 + 10.30594 * param[n]; break;
	case 2: vfdStopDelay = param[n] * 250; break;
	case 3: autoRunStart = param[n];
	case 4: maxFreq = 4.096 * param[n]; break;
	case 5: baseFreq = 4.096 * param[n]; break;
	case 6: minFreq = 4.096 * param[n]; break;
	case 7: stopFreq = 4.096 * param[n]; break;
	case 8: manualFreq = 4.096 * param[n]; break;
	case 9:
	case 10: freqToVolt = (float) param[n] / param[n - 1]; break;
	case 11: maxCur = 7.7824 * param[n]; break;
	case 12: minVolt = (float) param[n] * 2816 / 1395; break;
	case 13: maxVolt = (float) param[n] * 2816 / 1395; break;
	case 14: 
		r1 = TEMP_R0 * expf(TEMP_B * ((1 / ((float) param[n] + TEMP_K) - 1 / TEMP_0)));
		maxTemp = TEMP_RDIV / (r1 + TEMP_RDIV) * 1024;
		break;
	case 15: rotDirParam = param[n]; break;
	case 16: extSwConfig = param[n]; break;
	case 17: ignFaults = param[n]; break;
	case 18: ledIntensity = 0xff >> (param[n] - 1); break;
	}
}

// printf is too slow
void writeNum(char out[], uint16_t num, uint8_t nI, uint8_t nF) {
	uint8_t o, t;

	if (!nI) return;

	t = nI + (nF ? nF + 1 : 0);
	o = t;

	while (nF--) {
		out[--o] = '0' + num % 10;
		num /= 10;
		if (!nF) out[--o] = '.';
	}
	out[--o] = '0' + num % 10;
	num /= 10;
	while (--nI) {
		out[--o] = num ? '0' + num % 10 : ' ';
		num /= 10;
	}
	if (num) {
		while (t--) out[t] = '?';
	}
}

void serialSend(uint8_t byte) {
	uint16_t timeout;
	
	timeout = 65536;
	while (!SCI3.SSR.BIT.TDRE || !(timeout--));
	SCI3.TDR = byte;
}

// for debugging
void serialPrintf(const char *format, ...) {
	va_list args;
	uint8_t i, j;

	va_start(args, format);
	vsprintf(serialData, format, args);
	for (i = 0; i < 256; i++) {
		if (!serialData[i]) break;
		serialSend(serialData[i]);
	}
	va_end(args);
}

uint8_t spiCmd(uint32_t data, uint8_t out, uint8_t in) {
	IO.PDR5.BIT.B7 = 1; // CS on
	IO.PDR5.BIT.B5 = 1; // start bit
	delay(2);
	IO.PDR5.BIT.B6 = 1;
	delay(4);
	IO.PDR5.BIT.B6 = 0;
	delay(4);
	while (out--) {
		IO.PDR5.BIT.B5 = (data >> out) & 1;
		delay(4);
		IO.PDR5.BIT.B6 = 1;
		delay(4);
		IO.PDR5.BIT.B6 = 0;
	}
	IO.PDR5.BIT.B5 = 0;
	delay(4);

	while (in--) {
		IO.PDR5.BIT.B6 = 1;
		delay(3);
		data <<= 1;
		IO.PDR5.BIT.B6 = 0;
		delay(2);
		data |= IO.PDR5.BIT.B4;
		delay(2);
	}
	delay(2);
	IO.PDR5.BIT.B7 = 0; // CS off
	delay(2);
	return (uint8_t) data;
}

void eepRead(uint8_t *data, uint8_t addr, uint8_t size) {
	while (size--) {
		*data = spiCmd(0x400 | addr, 11, 8);
		addr++;
		data++;
	}
}

void eepWrite(uint8_t *data, uint8_t addr, uint8_t size) {
	uint16_t tWr;
	
	spiCmd(0x180, 11, 0); // write enable
	while (size--) {
		spiCmd(0x20000 | ((uint32_t) addr << 8) | *data, 19, 0);
		addr++;
		data++;
		tWr = t4ms;
		IO.PDR5.BIT.B7 = 1;
		delay(2);
		while ((!IO.PDR5.BIT.B4) && (t4ms - tWr < 3));
		IO.PDR5.BIT.B7 = 0;
		delay(2);
	}
	spiCmd(0x000, 11, 0); // write disable
}

void loadEeprom() {
	uint8_t i, byte;
	int16_t value;
	
	for (i = 0; i < 8; i++) {
		eepRead(&byte, i, 1);
		if (byte != eepSign[i]) goto repairEeprom;
	}
	for (i = 0; i < N_PARAM; i++) {
		eepRead((uint8_t *) &value, paramDef[i].eepAddr, 2);
		if (value >= paramDef[i].min & value <= paramDef[i].max)
			param[i] = value;
		else
			goto repairEeprom;
		WDT.TCWD = 0;
	}
	for (i = 0; i < N_PARAM; i++) {
		setParam(i);
		WDT.TCWD = 0;
	}
	return;
	
repairEeprom:
	for (i = 0; i < N_PARAM; i++) {
		param[i] = paramDef[i].def;
		eepWrite((uint8_t *) &param[i], paramDef[i].eepAddr, 2);
		setParam(i);
		WDT.TCWD = 0;
	}
	eepWrite((uint8_t *) eepSign, 0, 8);
}

uint8_t getKey() {
	uint8_t key, p7, p5b2;
	
	key = 0;
	p7 = IO.PDR7.BYTE;
	p5b2 = IO.PDR5.BIT.B2;
	IO.PDR5.BIT.B2 = 1; // turn off LED supply
	IO.PDR7.BYTE = 0x02; // key down
	delay(5);
	if (!IO.PDR6.BIT.B7) key = KEY_DOWN;
	IO.PDR7.BYTE = 0x04; // key up+auto
	delay(5);
	if (!IO.PDR6.BIT.B6) key = key ? KEY_INVALID : KEY_AUTO;
	if (!IO.PDR6.BIT.B7) key = key ? KEY_INVALID : KEY_UP;
	IO.PDR7.BYTE = 0x10; // key menu+run
	delay(5);
	if (!IO.PDR6.BIT.B6) key = key ? KEY_INVALID : KEY_RUN;
	if (!IO.PDR6.BIT.B7) key = key ? KEY_INVALID : KEY_MENU;
	IO.PDR7.BYTE = 0x20; // key enter
	delay(5);
	if (!IO.PDR6.BIT.B7) key = key ? KEY_INVALID : KEY_ENTER;
	IO.PDR7.BYTE = p7;
	IO.PDR5.BIT.B2 = p5b2;
	
	return key;
}

uint8_t readKey() {
	uint8_t key;
	
	key = getKey();
	if (key == KEY_INVALID) {
		tKey = t4ms;
		return KEY_NONE;
	}
	if (key != lastKey) {
		tKey = t4ms;
		keyFirst = 1;
		lastKey = key;
		return key;
	} else {
		if (t4ms - tKey > (keyFirst ? 200 : 15)) {
			tKey = t4ms;
			keyFirst = 0;
			return key;
		} else {
			return KEY_NONE;
		}
	}
}

void lcdSend(uint8_t data, uint8_t rs) {
	IO.PDR3.BYTE = data;
	IO.PDR1.BIT.B6 = rs;
	delay(50); // min 60ns
	IO.PDR1.BIT.B7 = 1;
	delay(10); // min 450ns
	IO.PDR1.BIT.B7 = 0;
	delay(50); // min 37us
}

void lcdInit() {
	uint8_t i;
	
	delay(150000); // min 40ms after Vcc>2.7V
	lcdSend(0x30, 0);
	delay(15000); // min 4.1ms
	lcdSend(0x30, 0);
	delay(500); // min 100us
	lcdSend(0x30, 0);
	delay(500); // min 37us
	lcdSend(0x38, 0);	// function set
	delay(500); // min 37us
	lcdSend(0x0c, 0);	// display on
	delay(500); // min 37us
	lcdSend(0x01, 0);  // clear display
	delay(5000);
	lcdSend(0x48, 0); // degree symbol
	for (i = 0; i < 8; i++)
		lcdSend(degreeSymbol[i], 1);
	lcdSend(0x50, 0); // arrow up symbol
	for (i = 0; i < 8; i++)
		lcdSend(arrowSymbol[i], 1);
}

void lcdSendLine(uint8_t line, char data[]) {
	uint8_t i, j;
	uint8_t rowOffset[] = { 0, 0x40 };

	lcdSend(rowOffset[line] | 0x80, 0);
	j = 0;
	for (i = 0; i < 16; i++) {
		if (data[j] != 0) {
			lcdSend((uint8_t) data[j++], 1);
		} else {
			lcdSend(0x20, 1);
		}
	}
}

void lcdPrintf(uint8_t line, const char *format, ...) {
	va_list args;

	va_start(args, format);
	vsprintf(lcdLine[line], format, args);
	lcdSendLine(line, lcdLine[line]);
	va_end(args);
}

int clockSetup() {
	
	CKCSR.BIT.PMRC = 3;
	//CKCSR.BIT.OSCBAKE = 1;
	CKCSR.BIT.CKSWIF = 0;
	//CKCSR.BIT.CKSWIE = 1;
	CKCSR.BIT.OSCSEL = 1;
	while (!CKCSR.BIT.CKSTA) {
		if (--clockWait == 0) {
			fault |= FAULT_XTAL;
			return 1;
		}
	}
	return 0;		
}

void startVfd() {
	freq = 0;
	pwmRatio = 0;
	TZ0.GRB = PWM_MAX / 2;
	TZ0.GRC = PWM_MAX / 2;
	TZ0.GRD = PWM_MAX / 2;
	set_imask_ccr(1);
	if (!(fault || scFault)) {
		TZ.TOCR.BYTE = 0;
		TZ.TOER.BYTE = 0xf1; // enable outputs B0, C0, D0
		vfdRun = 1;
	}
	set_imask_ccr(0);
}	

void stopVfd() {
	reqFreq = 0;
	freq = 0;
	regOn = 0;
	TZ.TOCR.BYTE = 0;
	TZ.TOER.BYTE = 0xff; // disable outputs B0, C0, D0
	vfdRun = 0;
}

void regVfd() {
	int16_t tmp;
	
	tReg = t4ms;
	if (fault || scFault) {
		if (vfdRun) stopVfd();
		return;
	}
	if ((pAct < pOn) || (regOn && (tReg - tOn < vfdStopDelay)) || flow) {
		if (!regOn || (pAct < pOff) || flow) {
			tOn = t4ms;
			regOn = 1;
		}
		tmp = ((pOff - pAct) >> 2) + baseFreq;
	} else {
		tmp = 0;
		regOn = 0;
	}
	if (regOn && (tmp < minFreq)) tmp = minFreq;
	if (tmp < 0) tmp = 0;
	if (tmp > maxFreq) tmp = maxFreq;
	reqFreq = tmp;
}

void adcProc() {
	uint8_t adcsr, chan;
	
	adcsr = AD.ADCSR.BYTE;
	if (adcsr & 0x80) {
		chan = adcsr & 7;
		switch (chan) {
			case 3:
				adcVal[0] += AD.ADDRD >> 6;
				adcCnt[0]++;
				if (adcCnt[0] == 0x40) {
					temp = adcVal[0] >> 6;
					adcCnt[0] = 0;
					adcVal[0] = 0;
					chan = 4;
				}
				break;
			case 4:
				adcVal[1] += AD.ADDRA >> 6;
				adcCnt[1]++;
				if (adcCnt[1] == 0x40) {
					current = adcVal[1] >> 6;
					adcCnt[1] = 0;
					adcVal[1] = 0;
					chan = 6;
				}
				break;
			case 6:
				adcVal[2] += AD.ADDRC >> 6;
				adcCnt[2]++;
				if (adcCnt[2] == 0x40) {
					voltage = adcVal[2] >> 6;
					adcCnt[2] = 0;
					adcVal[2] = 0;
					chan = 3;
				}
				break;
			default:
				chan = 3;
		}
		flow = !IO.PDRB.BIT.B2;
		AD.ADCSR.BYTE = chan;
		AD.ADCSR.BYTE = chan | 0x20;
	} else if (!(adcsr & 0x20)) {
		AD.ADCSR.BYTE = 0x03;
		AD.ADCSR.BYTE = 0x23;
	}
}

uint8_t extSw() {
	switch (extSwConfig) {
	case 0: return 1;
	case 1: return IO.PDR2.BIT.B3;
	case 2: return !IO.PDR2.BIT.B3;
	}
}

uint8_t newPressure() {
	uint32_t tmpDiff, tmpNow;
	
	if (pOvf && !(pLowWord & 0x8000)) pHighWord++;
	tmpNow = pLowWord + ((uint32_t) pHighWord << 16);
	tmpDiff = tmpNow - pTckLast;
	pTckLast = tmpNow;
	pNew = 0;
	if ((tmpDiff < 23500) || (tmpDiff > 110000)) return 0;
	pRaw[pIndex] = tmpDiff >> 6;
	pIndex++;
	if (pIndex > 2) pIndex = 0;
	if (((pRaw[0] >= pRaw[1]) && (pRaw[0] <= pRaw[2])) ||
		((pRaw[0] <= pRaw[1]) && (pRaw[0] >= pRaw[2]))) {
		pAct = pRaw[0];
	} else {
		if (((pRaw[1] >= pRaw[0]) && (pRaw[1] <= pRaw[2])) ||
			((pRaw[1] <= pRaw[0]) && (pRaw[1] >= pRaw[2]))) {
			pAct = pRaw[1];
		} else {
			pAct = pRaw[2];
		}
	}
	tPres = t4ms;
	return 1;
}

void checkFaults() {
	if (t4ms - tPres > 50) {
		fault |= FAULT_PRESSURE;
		stopVfd();
		tPresFault = t4ms;
	} else if ((fault & FAULT_PRESSURE) && (t4ms - tPresFault > 1000)) {
		fault &= ~FAULT_PRESSURE;
	}
	if (voltage < minVolt) {
		fault |= FAULT_UV;
		stopVfd();
		tUv = t4ms;
	} else if ((fault & FAULT_UV) && (t4ms - tUv > 1000)) {
		fault &= ~FAULT_UV;
	}
	if (voltage > maxVolt) {
		fault |= FAULT_OV;
		stopVfd();
		tOv = t4ms;
	} else if ((fault & FAULT_OV) && (t4ms - tOv > 1000)) {
		fault &= ~FAULT_OV;
	}
	if (temp > maxTemp) {
		fault |= FAULT_TEMP;
		stopVfd();
		tTemp = t4ms;
	} else if ((fault & FAULT_TEMP) && (t4ms - tTemp > 1000)) {
		fault &= ~FAULT_TEMP;
	}
	if (current > maxCur) {
		fault |= FAULT_OC;
		stopVfd();
	}
	if (!CKCSR.BIT.CKSTA) {
		fault |= FAULT_XTAL;
		stopVfd();
	}
}

void dispProc() {
	uint16_t pageVal;
	
	IO.PDR8.BIT.B6 = 1;
	switch (dispStep++) {
	case 0:
		IO.PDR8.BIT.B5 = 1;
		dispFreq = (float) freq * 62.5 / 256 + 0.5;
		writeNum(&statusLine[0][9], dispFreq, 2, 0);
		break;
	case 1:
		dispVolt = (float) voltage * (1395 / 2816);
		writeNum(&statusLine[1][1], dispVolt, 3, 0);
		break;
	case 2:
		dispCur = (float) current * (1250 / 9728);
		writeNum(&statusLine[1][6], dispCur, 1, 1);
		break;
	case 3:
		if (fault & FAULT_PRESSURE) {
			statusLine[0][0] = ' ';
			statusLine[0][1] = '-';
			statusLine[0][2] = ' ';
		} else {
			dispPres = (float) (pAct - 364) * 0.09703136;
			writeNum(&statusLine[0][0], dispPres, 1, 1);
		}
		statusLine[0][7] = flow ? 0x02 : 0x20;
		break;
	case 4:
		tempR = (float) (1024 - temp) / temp * TEMP_RDIV;
		dispTemp = TEMP_0 * TEMP_B / (TEMP_0 * logf(tempR/TEMP_R0) + TEMP_B) - TEMP_K;
		writeNum(&statusLine[1][11], dispTemp, 3, 0);
		writeNum(&statusLine[2][11], dispTemp, 3, 0);
		break;
	case 5:
		if (fault | scFault) {
			statusLine[0][14] = ((fault | scFault) >> 4) & 0xf;
			statusLine[0][14] += statusLine[0][14] < 10 ? '0' : ('A' - 10);
			statusLine[0][15] = (fault | scFault) & 0xf;
			statusLine[0][15] += statusLine[0][15] < 10 ? '0' : ('A' - 10);
		} else {
			statusLine[0][14] = 'O';
			statusLine[0][15] = 'K';
		}
		break;
	case 6:
		dispPow = (float) voltage * (float) current * (1395 / 2816 * 1250 / 9728 / 10);
		writeNum(&statusLine[2][1], dispPow, 4, 0);

		if ((pageDef[page].type == PAGE_FAULT) &&
			!(((fault | scFault) >> (page - PAGE_FIRST_FAULT)) & 1)) {
			for (page = PAGE_FIRST_FAULT; page <= PAGE_LAST_FAULT; page++)
				if (((fault | scFault) >> (page - PAGE_FIRST_FAULT)) & 1) break;
			if (page > PAGE_LAST_FAULT) page = 0;
		}
		if (pageDef[page].type == PAGE_INT) {
			writeNum(&statusLine[3][3], *pageDef[page].ptr, 5, 0);
			statusLine[3][8] = ' ';
		} else if (pageDef[page].type == PAGE_HEX8) {
			pageVal = *((uint8_t *) pageDef[page].ptr);
			statusLine[3][3] = '0';
			statusLine[3][4] = 'x';
			statusLine[3][5] = (pageVal >> 4) & 0xf;
			statusLine[3][5] += statusLine[3][5] < 10 ? '0' : ('A' - 10);
			statusLine[3][6] = pageVal & 0xf;
			statusLine[3][6] += statusLine[3][6] < 10 ? '0' : ('A' - 10);
			statusLine[3][7] = ' ';
			statusLine[3][8] = ' ';
		} else if (pageDef[page].type == PAGE_HEX16) {
			pageVal = *pageDef[page].ptr;
			statusLine[3][3] = '0';
			statusLine[3][4] = 'x';
			statusLine[3][5] = (pageVal >> 12) & 0xf;
			statusLine[3][5] += statusLine[3][5] < 10 ? '0' : ('A' - 10);
			statusLine[3][6] = (pageVal >> 8) & 0xf;
			statusLine[3][6] += statusLine[3][6] < 10 ? '0' : ('A' - 10);
			statusLine[3][7] = (pageVal >> 4) & 0xf;
			statusLine[3][7] += statusLine[3][7] < 10 ? '0' : ('A' - 10);
			statusLine[3][8] = pageVal & 0xf;
			statusLine[3][8] += statusLine[3][8] < 10 ? '0' : ('A' - 10);
		}
		break;
	case 7:
		if (!menu) {
			if (page < PAGE_FIRST_FAULT) {
				lcdSendLine(0, statusLine[0]);
			} else if (page <= PAGE_LAST_FAULT) {
				lcdSendLine(0, "FAULT:");
			} else {
				lcdSendLine(0, pageDef[page].name);
			}
		}
		break;
	default:
		if (!menu) {
			if (page == 0) lcdSendLine(1, statusLine[1]);
			else if (page == 1) lcdSendLine(1, statusLine[2]);
			else if (page <= PAGE_LAST_FAULT) lcdSendLine(1, pageDef[page].name);
			else lcdSendLine(1, statusLine[3]);
		}
		dispStep = 0;
	}
	tDisp = t4ms;
	IO.PDR8.BIT.B6 = 0;
	IO.PDR8.BIT.B5 = 0;
}

void voltCalc() {
	uint16_t tmp;
	float voltNow;
	
	if (voltage < minVolt || voltage > maxVolt) return;
	voltNow = (float) voltage * 1395 / 2816 / 1.414214;
	tmp = freqToVolt / voltNow * 252 * 62.5 / 4;
	if (tmp > 255) tmp = 255;
	freqToPwm = tmp;
}

void writeMenuLine() {
	uint8_t nI, nF, nU, i;

	nU = 0;
	while (paramDef[menuItem].unit[nU++]);
	nU--;
	for (i = 0; i < nU; i++) {
		menuLine[i + 7 - nU] = paramDef[menuItem].unit[i];
		menuLine[i + 16 - nU] = paramDef[menuItem].unit[i];
	}
	nF = paramDef[menuItem].f;
	nI = nF ? 5 - nU - nF : 6 - nU;
	writeNum(&menuLine[1], paramDef[menuItem].def, nI, nF);
	writeNum(&menuLine[10], itemValue, nI, nF);
}

void menuProc() {
	switch (key) {
	case KEY_MENU:
		if (menu) {
			menu--;
		} else {
			menu++;
		}
		itemValue = param[menuItem];
		break;
	case KEY_UP:
		switch (menu) {
		case 0:
			if (page > 0) {
				page--;
				if (pageDef[page].type == PAGE_FAULT) {
					while ((page >= PAGE_FIRST_FAULT) &&
						!((fault | scFault) >> (page - PAGE_FIRST_FAULT)) & 1) page--;
				}
			}
			break;
		case 1:
			if (menuItem > 0) menuItem--;
			itemValue = param[menuItem];
			break;
		case 2:
			if (itemValue < paramDef[menuItem].max) itemValue++;
			break;
		}
		break;
	case KEY_DOWN:
		switch (menu) {
		case 0:
			if (page < N_PAGE - 1) {
				page++;
				if (pageDef[page].type == PAGE_FAULT) {
					while ((page <= PAGE_LAST_FAULT) &&
						!((fault | scFault) >> (page - PAGE_FIRST_FAULT)) & 1) page++;
				}
			}
			break;
		case 1:
			if (menuItem < N_PARAM - 1) menuItem++;
			itemValue = param[menuItem];
			break;
		case 2:
			if (itemValue > paramDef[menuItem].min) itemValue--;
			break;
		}
		break;
	case KEY_ENTER:
		switch (menu) {
		case 1:
			itemValue = param[menuItem];
			menu++;
			break;
		case 2:
			param[menuItem] = itemValue;
			setParam(menuItem);
			eepWrite((uint8_t *) &param[menuItem], paramDef[menuItem].eepAddr, 2);
			menu--;
			break;
		}
		break;
	case KEY_AUTO:
		autoRun = !autoRun;
		break;
	case KEY_RUN:
		manualRun = !manualRun;
		break;
	}
	switch (menu) {
	case 0:
		break;
	case 1:
		lcdSendLine(0, paramDef[menuItem].name);
		writeMenuLine();
		menuLine[9] = ' ';
		lcdSendLine(1, menuLine);
		break;
	case 2:
		lcdSendLine(0, paramDef[menuItem].name);
		writeMenuLine();
		menuLine[9] = '>';
		lcdSendLine(1, menuLine);
		break;
	}
}

void modbusProc() {
	if (t4ms - tModbus > 2) modbusReqI = 0;
	if (SCI3.SSR.BIT.RDRF) {
		modbusReq[modbusReqI++] = SCI3.RDR;
		SCI3.SSR.BIT.RDRF = 0;
		tModbus = t4ms;
	}
	if (modbusReqI == 8) modbusReqI = 0;
}
	
void setLeds() {
	tLed++;
	
	switch (tLed & ledIntensity) {
	case 0: 
		IO.PDR7.BIT.B5 = 0;
		IO.PDR7.BIT.B6 = 1;
		break;
	case 1:
		IO.PDR7.BIT.B6 = 0;
		IO.PDR7.BIT.B4 = autoRun ? 1 : 0;
		break;
	case 2:
		IO.PDR7.BIT.B4 = 0;
		IO.PDR7.BIT.B2 = vfdRun ? 1 : 0;
		break;
	case 3:
		IO.PDR7.BIT.B2 = 0;
		if (ignFaults)
			IO.PDR7.BIT.B5 = ((t4ms & 0xf0) == 0) ? 1 : 0;
		else
			IO.PDR7.BIT.B5 = (fault | scFault) ? 1 : 0;
		break;
	case 4:
		IO.PDR7.BIT.B5 = 0;
		break;
	}
}

void main(void)
{
	uint8_t i, j, buf;
	uint16_t tSerial;
	
	startTCWD = WDT.TCWD;
	startTCSRWD = WDT.TCSRWD.BYTE;
	WDT.TCSRWD.BYTE = 0x54; // disable watchdog
	WDT.TCSRWD.BYTE = 0x54;
	WDT.TCWD = 0; // watchdog reset

	clockSetup();
	WDT.TCWD = 0; // watchdog reset
	
	MSTCR1.BYTE = 0x43; // module standby: RTC, Timer V, I2C
	MSTCR2.BYTE = 0x91; // module standby: PWM, Timer B1, SCI3_2

	IO.PDR6.BIT.B1 = 0; // FTIOB0 = 0
	IO.PDR6.BIT.B2 = 0; // FTIOC0 = 0
	IO.PDR6.BIT.B3 = 0; // FTIOD0 = 0

	IO.PMR1.BYTE = 0x32; // IRQ0, IRQ1, TXD
	IO.PCR1 = 0xc2; // relay, LCD RS, E
	IO.PCR2 = 0x00;	
	IO.PCR3 = 0xff; // LCD data
	IO.PCR5 = 0xe4; // EEPROM CS, SK, DI, LED supply
	IO.PCR6 = 0x0e; // FTIOB0, FTIOC0, FTIOD0 = out
	IO.PCR7 = 0x76;	// LEDs, keys
	IO.PCR8 = 0xff; // debug p85, p86, p87

	SCI3.SCR3.BYTE = 0x00; // UART
	SCI3.SMR.BYTE = 0x00; // 8N1
	SCI3.BRR = 51; // 9600baud @ 16MHz
	SCI3.SCR3.BYTE = 0x30; // UART RX/TX enable

	IEGR1.BYTE = 0x70; // bit0 = irq0 edge (pressure), bit1 = irq1 (fault)
	IENR1.BYTE = 0x13; // bit0 = irq0 enable (pressure), bit1 = irq1 (fault)

	TZ.TPMR.BYTE = 0x8f; // timer Z PWM mode B0, C0, D0
	TZ0.TCR.BYTE = 0x20; // clear TCNT on GRA compare match
	TZ1.TCR.BYTE = 0x03; // 16MHz / 8
	TZ0.TCNT = 0;
	TZ0.GRA = PWM_MAX;
	TZ0.TIER.BYTE = 0x0f; // enable TZ0.GRA match interrupt
	TZ1.TIER.BYTE = 0x10; // enable TZ1 overflow interrupt
	TZ.TSTR.BYTE = 0x03; // timer Z0, Z1 start
	
	lcdInit();

	WDT.TCWD = 0;

	lcdPrintf(0, "Wilo EMHIL505EM");
	lcdPrintf(1, " open firmware");

	WDT.TCWD = 0;

	tDisp = t4ms;
	loadEeprom();

	while (t4ms - tDisp < 250) WDT.TCWD = 0;
	IO.PDR1.BIT.B1 = 1; // switch on relay
	lcdPrintf(0, " Jakub Strnad");
	lcdPrintf(1, " v0.9 02/2025");
	tDisp = t4ms;
	while (t4ms - tDisp < 250) {
		newPressure();
		adcProc();
		WDT.TCWD = 0;
	}

	autoRun = autoRunStart;
	
	while (1) {
		adcProc();
		if (ignFaults) {
			fault = 0;
		} else {
			checkFaults();
		}
		if (!vfdRun && (t4ms - tVoltCalc > 25)) {
			voltCalc();
			rotDir = rotDirParam;
		}
		
		isNewPres = pNew ? newPressure() : 0;

		if (manualRun) {
			reqFreq = manualFreq;
		} else if (autoRun && extSw()) {
			if (isNewPres) regVfd();
		} else {
			reqFreq = 0;
		}
		if (fault || scFault) reqFreq = 0;
		if (!vfdRun && (reqFreq > stopFreq)) startVfd();
		else if (vfdRun && (reqFreq <= stopFreq) && (freq <= stopFreq)) stopVfd();

		setLeds();
		if (t4ms - tDisp > 10) dispProc();
		if (key = readKey()) menuProc();
		modbusProc();

		WDT.TCWD = 0;
	}
}

#pragma section IntPRG
//  vector 1 Reserved

//  vector 2 Reserved

//  vector 3 Reserved

//  vector 4 Reserved

//  vector 5 Reserved

//  vector 6 Reserved

//  vector 7 NMI
__interrupt(vect=7) void INT_NMI(void) {/* sleep(); */}
//  vector 8 TRAP #0
__interrupt(vect=8) void INT_TRAP0(void) {/* sleep(); */}
//  vector 9 TRAP #1
__interrupt(vect=9) void INT_TRAP1(void) {/* sleep(); */}
//  vector 10 TRAP #2
__interrupt(vect=10) void INT_TRAP2(void) {/* sleep(); */}
//  vector 11 TRAP #3
__interrupt(vect=11) void INT_TRAP3(void) {/* sleep(); */}
//  vector 12 Address break
__interrupt(vect=12) void INT_ABRK(void) {/* sleep(); */}
//  vector 13 SLEEP
__interrupt(vect=13) void INT_SLEEP(void) {/* sleep(); */}
//  vector 14 IRQ0
__interrupt(vect=14) void INT_IRQ0(void) { //irq0(); }
	pLowWord = TZ1.TCNT;
	pHighWord = z1highWord;
	pOvf = TZ1.TSR.BIT.OVF; // if OVF flag is set, we may need to add 1 to pHighWord
	pNew = 1;
	IRR1.BIT.IRRI0 = 0;
}

//  vector 15 IRQ1
__interrupt(vect=15) void INT_IRQ1(void) { //irq1(); }
	IRR1.BIT.IRRI1 = 0;
	scFault = FAULT_SHORT;
	TZ.TOER.BYTE = 0xff; // disable outputs B0, C0, D0
	TZ.TOCR.BYTE = 0;
}

//  vector 16 IRQ2
__interrupt(vect=16) void INT_IRQ2(void) {/* sleep(); */}
//  vector 17 IRQ3
__interrupt(vect=17) void INT_IRQ3(void) {/* sleep(); */}
//  vector 18 WKP
__interrupt(vect=18) void INT_WKP(void) {/* sleep(); */}
//  vector 19 RTC
__interrupt(vect=19) void INT_RTC(void) {/* sleep(); */}
//  vector 20 Reserved

//  vector 21 Reserved

//  vector 22 Timer V
__interrupt(vect=22) void INT_TimerV(void) {/* sleep(); */}
//  vector 23 SCI3
__interrupt(vect=23) void INT_SCI3(void) {/* sleep(); */}
//  vector 24 IIC2
__interrupt(vect=24) void INT_IIC2(void) {/* sleep(); */}
//  vector 25 ADI
__interrupt(vect=25) void INT_ADI(void) {/* sleep(); */}

//  vector 26 Timer Z0
__interrupt(vect=26) void INT_TimerZ0(void) { //irqZ0(); }
	uint8_t int4ms;
	
	IO.PDR8.BIT.B7 = 1;

	if (TZ0.TSR.BIT.IMFA) {
		TZ0.TSR.BIT.IMFA = 0;
		z0cnt++;
		if ((z0cnt & 0x1f) == 0) {
			t4ms++;
			if (freq < reqFreq)	freq++;
			else if ((freq > reqFreq) && (freq != 0)) freq--;
		}
		if (rotDir)
			fineIndex -= freq;
		else
			fineIndex += freq;
		svpwmIndex = (fineIndex >> 7) & 0xff;
	
		pwmRatio = freq * freqToPwm >> 6;
		if (pwmRatio > 251) pwmRatio = 251;
	}
	
	// we have to write each register right after its compare match because this MCU has no preload buffer
	// and writing them at wrong time will cause the pulse to not turn off in that cycle
	if (TZ0.TSR.BIT.IMFD) {
		TZ0.TSR.BIT.IMFD = 0;
		TZ0.GRD = ((int16_t) svpwmU[svpwmIndex] * pwmRatio >> 5) + PWM_MAX / 2;
	}
	if (TZ0.TSR.BIT.IMFC) {
		TZ0.TSR.BIT.IMFC = 0;
		TZ0.GRC = ((int16_t) svpwmV[svpwmIndex] * pwmRatio >> 5) + PWM_MAX / 2;
	}
	if (TZ0.TSR.BIT.IMFB) {
		TZ0.TSR.BIT.IMFB = 0;
		TZ0.GRB = ((int16_t) svpwmW[svpwmIndex] * pwmRatio >> 5) + PWM_MAX / 2;
	}
	IO.PDR8.BIT.B7 = 0;
}

//  vector 27 Timer Z1
__interrupt(vect=27) void INT_TimerZ1(void) {
	z1highWord++;
	TZ1.TSR.BYTE;
	TZ1.TSR.BIT.OVF = 0;
}

//  vector 28 Reserved

//  vector 29 Timer B1
__interrupt(vect=29) void INT_TimerB1(void) {/* sleep(); */}
//  vector 30 Reserved

//  vector 31 Reserved

//  vector 32 SCI3_2
__interrupt(vect=32) void INT_SCI3_2(void) {/* sleep(); */}
//  vector 33 Reserved

//  vector 34 OSCI
__interrupt(vect=34) void INT_OSCI(void) {/* sleep(); */}

