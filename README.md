# wilo-vfd
Open Source Firmware for Wilo EMHIL 505 EM water pump

### Pinouts of internal connections ###

14pin IDC header
================
1 +15v
2 +15v
3 +5v for temp sensor
4 NTC temp sensor, possibly 100k/3950K (around 145k@13C, 118k@16C, 17k@50C)
5 IGBT fault (1k R -> Fo pin)
6 current sense (0.02R x19 gain op-amp)
7 Un
8 Up
9 Vn
10 Vp
11 Wn
12 Wp
13 GND
14 GND

14pin IDC socket - debug
========================
1 p87 (pin 43)
2 gnd
3 nc?
4 gnd
5 p86 (pin 42)
6 gnd
7 /nmi
8 +5v
9 nc?
10 gnd
11 p85 (pin 41)
12 gnd
13 /reset
14 gnd

MCU signals (Renesas/Hitachi H8 36077GFZV)
==========================================
p11 relay
pb6/an6/extd v300 [221k / 2k2 (2232/22)]

p76 pwr led
p72 run led + key up+auto
p75 fault led + key enter
p74 auto led + key menu+run
p71 key down

p67 menu+enter+down+up input
p66 run+auto input

p16 lcd RS
gnd lcd RW
p17 lcd E
p30 lcd D0
...
p37 lcd D7

p14/irq0 pressure sensor
pb2/an2 flow sensor (1=no flow)
p23 external float sensor

pb3/an3 temp sensor
p15/irq1 fault
pb4/an4 current sense [0.02r x19 amp]

p63/ftiod0 U
p62/ftioc0 V
p61/ftiob0 W

p57/scl eeprom CS
p56 eeprom SK
p55 eeprom DI
p54 eeprom DO
p52 LED supply
