/* IO Expander sketch optimized
 *  
 * Irrigation System v1.1
 * 
 */
#include <math.h>
#include <time.h> // File located \Program Files (x86)\Arduino\hardware\tools\avr\avr\include\time.h
#include <util/crc16.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include "IOExpander.h"

#define FAHRENHEIT
#define INIT_BOARD              "g5w1;g11w1;g11d0,75;g12w1;g12d0,75;rsf"
#define ONEWIRE_TO_I2C_ROM1     "i4scc"
#define ONEWIRE_TO_I2C_ROM2     "i6s8f"
#define ONEWIRE_TEMPERATURE     "t6s0300"
#define RTC_SENSOR              "s4te"
#define I2C_EEPROM              "s4tf"
#define I2C_OLED                "s4t10"
#define I2C_LIGHT               "s3t9;sc0"
#define OPTICAL_SENSOR          "g5a"
#define BUTTON1                 "g11d"
#define BUTTON2                 "g12d"

#define WATER_TIME_BEFORE_SUNRISE 60
#define SUNRISE_LUX             100
#define RAIN_DETECT_LEVEL       4.0
#define DO_NOT_WATER_TEMP       4.4444 // 40F

#define MAX_ZONES               4

#define HOUR_IN_DAY             24L
#define MIN_IN_HOUR             60L
#define SEC_IN_MIN              60L
#define SEC_IN_HOUR             (MIN_IN_HOUR * SEC_IN_MIN)
#define SEC_IN_DAY              (HOUR_IN_DAY * SEC_IN_HOUR)
#define DAYS_IN_WEEK            7
#define SEC_IN_WEEK             (SEC_IN_DAY * DAYS_IN_WEEK)

#define SUN                     0x01
#define MON                     0x02
#define TUE                     0x04
#define WED                     0x08
#define THR                     0x10
#define FRI                     0x20
#define SAT                     0x40
#define EVERYDAY                (SUN | MON | TUE | WED | THR | FRI | SAT)

#define SUNRISE                 0x80

#define MENU_OPTIONS            9
#define MENU_TIME               30

#define OFF                     0
#define ON                      1

#define STATE_ON_OFF            0x01

//#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
SoftwareSerial swSerial(8,7);
#endif

char weekday[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

char menu[][13] = {"Next",
                   "Water",
                   "Reset",
                   "Clock Min +",
                   "Clock Min -",
                   "Clock Hour +",
                   "Clock Hour -",
                   "Sunrise",
                   "ON/OFF"};

enum {
  MENU_NEXT,
  MENU_WATER,
  MENU_RESET,
  MENU_CLOCK_MIN_PLUS,
  MENU_CLOCK_MIN_MINUS,
  MENU_CLOCK_HOUR_PLUS,
  MENU_CLOCK_HOUR_MINUS,
  MENU_SUNRISE,
  MENU_ON_OFF
};

typedef struct {
  char description[16];
  uint8_t relay;
} ZONE;

typedef struct {
  uint8_t zone;
  uint8_t days;
  int8_t hour;
  int8_t min;
  uint8_t duration;
} SCHEDULE;

typedef struct {
  time_t sunrise_time;
  time_t last_water_time;
  uint8_t water_schedule;
  uint8_t water_duration;
  uint8_t rain[MAX_ZONES];
  uint8_t state;
  uint8_t crc;
} NVRAM;

enum {
  ZONE1,
  ZONE2,
  ZONE3,
  ZONE4
};

enum {
  RELAY1 = 1,
  RELAY2,
  RELAY3,
  RELAY4
};

ZONE zone[] = {
  {"Front Right", RELAY1},
  {"Front Left",  RELAY2},
  {"Bushes",      RELAY3},
  {"Left Side",   RELAY4},
};

SCHEDULE schedule[] = {
  {ZONE1, SUNRISE | EVERYDAY, -1, 0,  4},
  {ZONE2, EVERYDAY,            6, 15, 5},
  {ZONE3, EVERYDAY,            6, 0,  10},
  {ZONE4, EVERYDAY,            6, 10, 6},
}; 

NVRAM nvram;
bool update_nvram = false;

uint8_t crc8(uint8_t* data, uint16_t length)
{
  uint8_t crc = 0;

  while (length--) {
    crc = _crc8_ccitt_update(crc, *data++);
  }
  return crc;
}

int led = 13;
bool init_oled = true;
bool update_oled = true;
bool init_board = true;

#ifdef FAHRENHEIT
#define C2F(temp)   CelsiusToFahrenheit(temp)
float CelsiusToFahrenheit(float celsius)
{
  return ((celsius * 9) / 5) + 32;
}
#else
#define C2F(temp)   (temp)
#endif

void SerialPrint(const char* str, float decimal, char error)
{
  Serial.print(str);
  if (error) Serial.print(F("NA"));
  else Serial.print(decimal, 1);
}

time_t NextScheduleTime(time_t last_time, uint8_t* next_schedule)
{
  time_t next_time = -1;
  time_t clk_time;
  uint8_t i;
  tm clk;
  uint8_t wday;
 
  for (i = 0; i < sizeof(schedule) / sizeof(SCHEDULE); i++) {
    if (schedule[i].days & SUNRISE) {
      clk_time = nvram.sunrise_time; 
      clk_time += schedule[i].hour * SEC_IN_HOUR;
      clk_time += schedule[i].min * SEC_IN_MIN;
      localtime_r(&clk_time, &clk);
    } 
    else {
      localtime_r(&last_time, &clk);
      clk.tm_hour = schedule[i].hour;
      clk.tm_min = schedule[i].min;
      clk.tm_sec = 0;
      clk_time = mktime(&clk);
    }
    wday = clk.tm_wday;
    while (clk_time <= last_time || !(schedule[i].days & (1 << wday)))
    {
      clk_time += SEC_IN_DAY;
      if (++wday > SATURDAY) wday = SUNDAY;
      if (wday == clk.tm_wday) break; // Only check one week
    }
    if (clk_time < next_time) {
      next_time = clk_time;
      *next_schedule = i;
    }
  }
  return next_time;
}

void StartScheduleTime(time_t start_time, uint8_t start_schedule)
{
  uint8_t i;
 
  nvram.last_water_time = start_time;
  nvram.water_schedule = start_schedule;
  nvram.water_duration = schedule[start_schedule].duration+1;
  update_nvram = true;
  // Check if it rained
  i = schedule[start_schedule].zone;
  if (i < MAX_ZONES && nvram.rain[i] > 0) {
    if (nvram.rain[i] > nvram.water_duration) nvram.water_duration = 0;
    else nvram.water_duration -= nvram.rain[i];
    nvram.rain[i] = 0;
  }
}

void WaterScheduleTime(void) 
{
  uint8_t i;
 
  nvram.water_duration--;
  update_nvram = true;
  i = schedule[nvram.water_schedule].zone;
  if (i < MAX_ZONES) {
    Serial.print("r");
    Serial.print(zone[i].relay);
    if (nvram.water_duration > 0) Serial.println("o");
    else Serial.println("f");
    SerialReadUntilDone();
  }
}

void setup() {
  Serial.begin(115200);
#ifdef SERIAL_DEBUG
  swSerial.begin(115200);
#endif  
  pinMode(led, OUTPUT);
  //delay(1000);
  wdt_enable(WDTO_8S);
}

void loop() {
  static tm rtc;
  tm clk, sunrise_clk;
  time_t rtc_time;
  time_t clk_time;
  static time_t next_time;
  static uint8_t last_sec;
  static uint8_t last_min;
  bool error_rtc;
  bool error_light;
  bool error_temp;
  static long lux = 0;
  static float temp, rain;
  static uint8_t sunrise_counter = MIN_IN_HOUR;
  static bool check_sunrise = false;
  uint8_t i;
  static bool read_nvram = true;
  static time_t water_time;
  static uint8_t water_schedule;
  uint8_t sz;
  uint8_t wday;
  long n;
  bool button1, button2;
  static int8_t menu_select = -1;
  static time_t menu_time = 0;
 
  Serial.println();
  if (SerialReadUntilDone()) {
    if (init_board) {
      SerialCmdDone(INIT_BOARD);
      init_board = false;
    }
 
    if (init_oled) {
      if (SerialCmdNoError(ONEWIRE_TO_I2C_ROM1)) {
        SerialCmdDone(I2C_OLED ";si;sc;sd");
        init_oled = false;
      }
    }
 
    if (SerialCmdDone(RTC_SENSOR)) {
      error_rtc = !SerialReadTime(&rtc);
      if (!error_rtc) {
        clk = rtc; // mktime() can change struct tm
        rtc_time = mktime(&clk);
        localtime_r(&rtc_time, &rtc);  // Get wday.
      }

      if (read_nvram) {
        if (SerialCmdNoError(I2C_EEPROM)) {
          SerialReadEEPROM((uint8_t*)&nvram, 0, sizeof(nvram));
          if (nvram.crc != crc8((uint8_t*)&nvram, sizeof(nvram)-sizeof(uint8_t))) {
            //swSerial.println("CRC8 Failure!");
            // Initialize nvram
            memset(&nvram, 0, sizeof(nvram));
            clk = rtc;
            clk.tm_hour = 6;
            clk.tm_min = 0;
            clk.tm_sec = 0;
            nvram.sunrise_time = mktime(&clk);
            if (nvram.sunrise_time < rtc_time) nvram.sunrise_time + SEC_IN_DAY;
            update_nvram = true;
          }
          // Check last water time no less than one week
          if (rtc_time - nvram.last_water_time > SEC_IN_WEEK) nvram.last_water_time = rtc_time - SEC_IN_WEEK;
          // Check sunrise time
          if (rtc_time > nvram.sunrise_time) {
            localtime_r(&nvram.sunrise_time, &sunrise_clk);
            clk = rtc;
            clk.tm_hour = sunrise_clk.tm_hour;
            clk.tm_min = sunrise_clk.tm_min;
            clk.tm_sec = sunrise_clk.tm_sec;
            nvram.sunrise_time = mktime(&clk);
            if (nvram.sunrise_time < rtc_time) nvram.sunrise_time + SEC_IN_DAY;                      
          }
          if (nvram.water_duration) {
            nvram.water_duration++;
            water_time = nvram.last_water_time;
          }
          else {
            clk_time = (nvram.last_water_time) ? nvram.last_water_time : rtc_time;
            water_time = NextScheduleTime(clk_time, &water_schedule);
          }
          read_nvram = false;
        }
      }
    }

    // Process only once every minute
    if (rtc.tm_min != last_min)
    {
      // Request a 1-Wire temperature measurement.  Read it later.
      error_temp = !SerialCmdNoError(ONEWIRE_TEMPERATURE);
      if (!error_temp) SerialCmdDone("tt");
 
      error_light = !SerialCmdNoError(ONEWIRE_TO_I2C_ROM2 ";oo0");
      if (!error_light) {
        SerialCmdDone(I2C_LIGHT); // Do not use overdrive
        SerialCmd("sr");
        SerialReadInt(&lux);
        SerialReadUntilDone();
      }
 
      if (SerialCmd(OPTICAL_SENSOR)) {
        SerialReadFloat(&rain);
        SerialReadUntilDone();
      }

      error_temp = !SerialCmdNoError(ONEWIRE_TEMPERATURE);
      if (!error_temp) {
        SerialCmd("tr");
        SerialReadFloat(&temp);
        SerialReadUntilDone();
      }
 
      // Is it sunrise?
      if (lux < SUNRISE_LUX) {
        if (sunrise_counter > 0) sunrise_counter--;
        else check_sunrise = true;
      }
      else {
        if (sunrise_counter < MIN_IN_HOUR) {
          sunrise_counter++;
          if (check_sunrise && sunrise_counter == MIN_IN_HOUR) {
            nvram.sunrise_time = rtc_time + (SEC_IN_DAY - SEC_IN_HOUR);
            check_sunrise = false;
            update_nvram = true;
          }
        }
      }

      // Is it raining?
      if (rain <= RAIN_DETECT_LEVEL) {
        for (i = 0; i < MAX_ZONES; i++) {
          if (nvram.rain[i] < -1) nvram.rain[i]++;
        }
        update_nvram = true;
      }

      // Check schedule
      if (menu_select == -1 && !nvram.water_duration) {
        while (water_time + (schedule[water_schedule].duration * SEC_IN_MIN) < rtc_time) {
          water_time = NextScheduleTime(water_time, &water_schedule);
        }
        if (water_time <= rtc_time) {
          StartScheduleTime(water_time, water_schedule);
          if (temp <= DO_NOT_WATER_TEMP || nvram.state & STATE_ON_OFF == OFF)
             nvram.water_duration = 0;
        }
      }

      // Do we need to water?
      if (nvram.water_duration) {
        WaterScheduleTime();
        if (!nvram.water_duration)
          water_time = NextScheduleTime(water_time, &water_schedule);
      }

      last_min = rtc.tm_min;
      update_oled = true;
    }

    // Check buttons
    button1 = SerialReadButton(BUTTON1);
    if (button1) {
      if (menu_select == -1) menu_select = 0;
      else {
        if (++menu_select >= MENU_OPTIONS)
          menu_select = 0;
      }
      menu_time = rtc_time;
      update_oled = true;
    }
    if (menu_select >= 0) {
      button2 = SerialReadButton(BUTTON2);
      if (button2) {
        clk_time = rtc_time;
        switch(menu_select) {
          case MENU_NEXT:
          case MENU_RESET:
            if (nvram.water_duration) {
              nvram.water_duration = 1;
              WaterScheduleTime();
            }
            water_time = NextScheduleTime((menu_select == MENU_NEXT) ? water_time : rtc_time, &water_schedule);
            break;
          case MENU_WATER:
            StartScheduleTime(water_time, water_schedule);
            WaterScheduleTime();
            break;
          case MENU_CLOCK_MIN_PLUS:
            clk_time += SEC_IN_MIN;
            break;
          case MENU_CLOCK_MIN_MINUS:
            clk_time -= SEC_IN_MIN;
            break;
          case MENU_CLOCK_HOUR_PLUS:
            clk_time += SEC_IN_HOUR;
            break;
          case MENU_CLOCK_HOUR_MINUS:
            clk_time -= SEC_IN_HOUR;
            break;      
          case MENU_ON_OFF:
            nvram.state ^= STATE_ON_OFF;
            update_nvram = true;
            break;  
        }
        if (clk_time != rtc_time) {
          if (SerialCmdDone(RTC_SENSOR)) {
            localtime_r(&clk_time, &clk);
            SerialWriteTime(&clk);
            rtc_time = clk_time;
          }
        }
        menu_time = rtc_time;
        update_oled = true;
      }
    }
    if (menu_select >= 0 && rtc_time - menu_time > MENU_TIME) {
      menu_select = -1;
      update_oled = true;
    }

    if (update_oled) {
      if (SerialCmdNoError(ONEWIRE_TO_I2C_ROM1)) {
        Serial.print("st10;so1;sc;sf0;sa0;sd0,0,\"");
        if (nvram.water_duration) Serial.print(nvram.water_duration);
        else {
          if ((nvram.state & STATE_ON_OFF) == OFF) Serial.print("OFF");
          else if (rain <= RAIN_DETECT_LEVEL) Serial.print("Rain");
          else if (temp <= DO_NOT_WATER_TEMP) Serial.print("Cold");
          else Serial.print("v1.1");
        }
        Serial.print("\";sf2;sa1;sd75,0,\"");
        if (menu_select == 7) { // Sunrise
          clk_time = nvram.sunrise_time;
          localtime_r(&clk_time, &clk);
        }
        else clk = rtc;
        Serial.print(clk.tm_hour-((clk.tm_hour>12)?12:0));
        Serial.print(":");
        if (clk.tm_min < 10) Serial.print("0");
        Serial.print(clk.tm_min);
        Serial.println("\"");
        SerialReadUntilDone();
 
        Serial.print("sf1;sa0;sd79,8,\"");
        Serial.print((clk.tm_hour>12)?"PM":"AM");
        Serial.print("\";sf0;sa1;sd127,1,\"");
        Serial.print(weekday[clk.tm_wday]);
        Serial.print("\";sd127,13,\"");
        Serial.print(clk.tm_mon+1);
        Serial.print("/");
        Serial.print(clk.tm_mday);
        Serial.println("\"");
        SerialReadUntilDone();
 
        Serial.print("sf0;sa0;sd1,36,\"");
        i = schedule[water_schedule].zone;
        if (i < MAX_ZONES) Serial.print(zone[i].description);
        localtime_r(&water_time, &clk);
        if (water_time - rtc_time > SEC_IN_DAY) {
          Serial.print("\";sa1;sd126,36,\"");
          Serial.print(clk.tm_mon+1);
          Serial.print("/");
          Serial.print(clk.tm_mday);
          Serial.print(" ");
          Serial.print(clk.tm_hour-((clk.tm_hour>12)?12:0));
          Serial.print(":");
          if (clk.tm_min < 10) Serial.print("0");
          Serial.print(clk.tm_min); 
          Serial.print(" ");
        }
        else {
          Serial.print("\";sf1;sa1;sd111,30,\"");
          Serial.print(clk.tm_hour-((clk.tm_hour>12)?12:0));
          Serial.print(":");
          if (clk.tm_min < 10) Serial.print("0");
          Serial.print(clk.tm_min); 
          Serial.print("\";sf0;sd126,36,\"");
        }
        Serial.print((clk.tm_hour>12)?"PM":"AM");     
        if (nvram.water_duration) Serial.print("\";so2;sc0,29,128,19");
        Serial.println();
        SerialReadUntilDone();
 
        if (menu_select == -1) {
          //Serial.print("\";sa0;sd0,52,\"");
          //Serial.print(rain);
          SerialPrint("\";so1;sa2;sd63,52,\"", C2F(temp), error_temp);
          if (!error_temp) Serial.print("\",248,\""
  #ifdef FAHRENHEIT
            "F"
  #else
            "C"
  #endif
            );
          Serial.print(" / ");  
          Serial.print(lux);  
        }
        else {
          Serial.print("\";so0;sc0,51,128,14;sf0;sa2;sd63,52,\"");
          if (menu_select == MENU_ON_OFF) {
            Serial.print((nvram.state & STATE_ON_OFF) ? "OFF" : "ON");
          }
          else Serial.print(menu[menu_select]);
        }
        Serial.println("\";sd");
        SerialReadUntilDone();
        update_oled = false;
      }
      else init_oled = true;
    }

    if (update_nvram) {
      if (SerialCmdNoError(I2C_EEPROM)) {
        nvram.crc = crc8((uint8_t*)&nvram, sizeof(nvram)-sizeof(uint8_t));
        //swSerial.println(nvram.crc, HEX);
        SerialWriteEEPROM((uint8_t*)&nvram, 0, sizeof(nvram));
        update_nvram = false;
      }
    }

    delay(50);
  }
  else {
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    delay(500);
    init_board = true;
    init_oled = true;
  }
  wdt_reset();
}
