# wilo-vfd
Open Source Firmware for Wilo EMHIL 505 EM water pump

## Menu parameters description

- **ON pressure:** pressure at which the pump will start regardless of flow sensor input
- **OFF pressure:** pressure at which the pump can stop if there is no flow for specified amount of time
- **OFF delay:** time before stopping the pump when there is no flow and OFF pressure has been reached
- **Autorun:** power-on autorun enable/disable
- **Max frequency:** maximum output frequency in autorun mode
- **Base frequency:** output frequency when pressure is equal to OFF pressure
  (increases when pressure is lower, decreases when pressure is higher);
  make sure this is set to a value that can achieve the OFF pressure with zero flow, otherwise the pump will never turn off
- **Min frequency:** minimum steady output frequency in autorun mode
- **Stop frequency:** below this frequency, PWM is turned off and output phases are shorted to the negative pole
- **Manual frequency:** frequency preset for manual run mode
- **Rated frequency:** rated frequency of the motor (like 50Hz or 60Hz), used for output V/f ratio calculation
- **Rated voltage:** rated RMS voltage of the motor (like 230V), used for output V/f ratio calculation;
  HINT: you can decrease this to reduce the power consumption, in my case, 130V works pretty well
- **Max current:** current from the DC rail; if this value is exceeded, fault is set, resets when both auto and manual run is disabled
- **Undervoltage:** minimum voltage on the DC rail; if voltage drops below this value, temporary fault is set, automatically resets in 4 seconds
- **Overvoltage:** maximum voltage on the DC rail; if voltage exceeds this value, temporary fault is set, automatically resets in 4 seconds
- **Max temperature:** maximum temperature of the IGBT module; if temperature exceeds this value, temporary fault is set, automatically resets in 4 seconds
- **No flow timeout:** maximum time the pump can continuously run without detecting water flow; fault resets when both auto and manual run is disabled
- **Rotation dir.:** 0 = "original" rotation direction; 1 = the opposite
- **External switch:** 0 = disabled; 1 = autorun when closed; 2 = autorun when open
- **Ignore faults:** disable fault detection, except short-circuit fault from IGBT module
- **LED intensity:** sets the PWM period for LED outputs; 1 = darkest (longest); 7 = brightest (shortest period)
- **Modbus ID:** Modbus ID for reading the holding registers through serial port

Pinouts of internal connections
===============================

J200 serial port
----------------
- 1 GND
- 2 RxD (from PC to MCU)
- 3 TxD (from MCU to PC)
- 4 +15V
- this port is optically isolated and powered from an isolated supply (+15VA and GND2)
- seems to work only up to 9600 baud (not verified)
- this port is used for programming in boot mode;
  J302 pin 7 must be connected to GND - non-isolated side !

14pin IDC header
----------------
- 1 +15v
- 2 +15v
- 3 +5v for temp sensor
- 4 NTC temp sensor, possibly 100k/3950K (around 145k@13C, 118k@16C, 17k@50C)
- 5 IGBT fault (1k R -> Fo pin)
- 6 current sense (0.02R x19 gain op-amp)
- 7 Un
- 8 Up
- 9 Vn
- 10 Vp
- 11 Wn
- 12 Wp
- 13 GND
- 14 GND

14pin IDC socket - debug
------------------------
- 1 p87 (pin 43)
- 2 gnd
- 3 nc?
- 4 gnd
- 5 p86 (pin 42)
- 6 gnd
- 7 /nmi
- 8 +5v
- 9 nc?
- 10 gnd
- 11 p85 (pin 41)
- 12 gnd
- 13 /reset
- 14 gnd

MCU signals (Renesas/Hitachi H8 36077GFZV)
------------------------------------------
- p11 relay
- pb6/an6/extd v300 [221k / 2k2 (2232/22)]

- p76 pwr led
- p72 run led + key up+auto
- p75 fault led + key enter
- p74 auto led + key menu+run
- p71 key down

- p67 menu+enter+down+up input
- p66 run+auto input

- p16 lcd RS
- gnd lcd RW
- p17 lcd E
- p30 lcd D0
- ...
- p37 lcd D7

- p14/irq0 pressure sensor
- pb2/an2 flow sensor (1=no flow)
- p23 external float sensor

- pb3/an3 temp sensor
- p15/irq1 fault
- pb4/an4 current sense [0.02r x19 amp]

- p63/ftiod0 U
- p62/ftioc0 V
- p61/ftiob0 W

- p57/scl eeprom CS
- p56 eeprom SK
- p55 eeprom DI
- p54 eeprom DO
- p52 LED supply
