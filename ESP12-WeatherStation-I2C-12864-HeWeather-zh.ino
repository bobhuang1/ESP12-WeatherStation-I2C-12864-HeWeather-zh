#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESPHTTPClient.h>
#include <JsonListener.h>
#include <stdio.h>
#include <time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()
#include <Timezone.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <WiFiManager.h>
#include "HeWeatherCurrent.h"
#include "HeWeatherForecast.h"
#include "WeatherStationImages.h"
#include "WeatherStationFonts.h"

//#define USE_WIFI_MANAGER     // disable to NOT use WiFi manager, enable to use
//#define SHOW_US_CITIES  // disable to NOT to show Fremont and NY, enable to show
#define USE_HIGH_ALARM       // disable - LOW alarm sounds, enable - HIGH alarm sounds
#define USE_LED              // diable to NOT use LEDs, enable to use LEDs
//#define USE_OLD_LED          // disable to use new type 3mm red-blue LED, enable to use old type 5mm red-green LED

#define DHTTYPE  DHT11       // Sensor type DHT11/21/22/AM2301/AM2302
#define SMOKEPIN   2

#ifdef USE_LED
#define DHTPIN   4
#define ALARMPIN 5
#else
#define DHTPIN   5
#define ALARMPIN 4
#endif

#ifdef USE_LED
#define LEDRED 15
#define LEDGREEN 0
#endif

const char CompileDate[] = __DATE__;
const char* WIFI_SSID[] = {"ibehome", "ibetest", "ibehomen", "TYCP", "Tenda_301"};
const char* WIFI_PWD[] = {"tianwanggaidihu", "tianwanggaidihu", "tianwanggaidihu", "5107458970", "5107458970"};
#define numWIFIs (sizeof(WIFI_SSID)/sizeof(char *))
#define WIFI_TRY 30

#define TZ              8       // (utc+) TZ in hours
#define DST_MN          0      // use 60mn for summer time in some countries
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)
#define UPDATE_INTERVAL_SECS 20 * 60 // Update every 20 minutes

#if (DHTPIN >= 0)
DHT dht(DHTPIN, DHTTYPE);
#endif

// Japan, Tokyo
TimeChangeRule japanRule = {"Japan", Last, Sun, Mar, 1, 540};     // Japan
Timezone Japan(japanRule);
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);
// United Kingdom (London, Belfast)
TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        // British Summer Time
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         // Standard Time
Timezone UK(BST, GMT);
// UTC
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};     // UTC
Timezone UTC(utcRule);
// US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);
// US Central Time Zone (Chicago, Houston)
TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};
TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};
Timezone usCT(usCDT, usCST);
// US Mountain Time Zone (Denver, Salt Lake City)
TimeChangeRule usMDT = {"MDT", Second, Sun, Mar, 2, -360};
TimeChangeRule usMST = {"MST", First, Sun, Nov, 2, -420};
Timezone usMT(usMDT, usMST);
// Arizona is US Mountain Time Zone but does not use DST
Timezone usAZ(usMST);
// US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
Timezone usPT(usPDT, usPST);

String HEWEATHER_APP_ID = "d72b42bcfc994cfe9099eddc9647c6f2";
String HEWEATHER_LANGUAGE = "zh"; // zh for Chinese, en for English
#ifdef USE_WIFI_MANAGER
String HEWEATHER_LOCATION = "auto_ip"; // Get location from IP address
#else
String HEWEATHER_LOCATION = "CN101210202"; // Changxing
#endif
String HEWEATHER_LOCATION1 = "US3290117";
String HEWEATHER_LOCATION2 = "US5392171";
const uint8_t MAX_FORECASTS = 5; // do not change this

const String WDAY_NAMES[] = {"星期天", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};

HeWeatherCurrentData currentWeather;
HeWeatherCurrent currentWeatherClient;

#ifdef SHOW_US_CITIES
HeWeatherCurrentData currentWeather1;
HeWeatherCurrentData currentWeather2;
HeWeatherCurrent currentWeatherClient1;
HeWeatherCurrent currentWeatherClient2;
#endif

HeWeatherForecastData forecasts[MAX_FORECASTS];
HeWeatherForecast forecastClient;

// 4 CS, 5 SID(data), 6 CLK, 17 RST(Reset)
U8G2_ST7920_128X64_F_SW_SPI display(U8G2_R0, /* clo  ck=*/ 14 /* A4 */ , /* data=*/ 12 /* A2 */, /* CS=*/ 13 /* A3 */, /* reset=*/ 16); // 16, U8X8_PIN_NONE

time_t nowTime;
const String degree = String((char)176);
bool readyForWeatherUpdate = false;
long timeSinceLastWUpdate = 0;
uint8_t draw_state = 0;
float previousTemp = 0;
float previousHumidity = 0;

void smokeHandler() {
  int smokeValue = digitalRead(SMOKEPIN);
  //  Serial.print("Smoke interrupt: ");
  //  Serial.println(smokeValue);
  if (smokeValue == 1)
  {
#ifdef USE_HIGH_ALARM
    digitalWrite(ALARMPIN, LOW);
#else
    digitalWrite(ALARMPIN, HIGH);
#endif
#ifdef USE_LED
    ledoff();
#endif
    //    Serial.println("Turn off alarm");
  }
  else
  {
#ifdef USE_HIGH_ALARM
    digitalWrite(ALARMPIN, HIGH);
#else
    digitalWrite(ALARMPIN, LOW);
#endif
#ifdef USE_LED
    ledred();
#endif
    //    Serial.println("Turn on alarm");
  }
}

#ifdef USE_LED
void ledoff () {
  digitalWrite(LEDGREEN, LOW);
  digitalWrite(LEDRED, LOW);
}

void ledred () {
  ledoff();
#ifdef USE_OLD_LED
  analogWrite(LEDRED, 128);
#else
  analogWrite(LEDRED, 100);
#endif
}

void ledgreen () {
  ledoff();
#ifdef USE_OLD_LED
  digitalWrite(LEDGREEN, HIGH);
#else
  analogWrite(LEDGREEN, 100);
#endif
}

void ledyellow () {
  ledoff();
#ifdef USE_OLD_LED
  analogWrite(LEDRED, 128);
  digitalWrite(LEDGREEN, HIGH);
#else
  analogWrite(LEDRED, 50);
  analogWrite(LEDGREEN, 100);
#endif
}
#endif

void setup() {
  delay(100);
  Serial.begin(115200);
  Serial.println("Begin");

#if (DHTPIN >= 0)
  dht.begin();
#endif

  pinMode(SMOKEPIN, INPUT);
  pinMode(ALARMPIN, OUTPUT);
#ifdef USE_HIGH_ALARM
  digitalWrite(ALARMPIN, LOW); // Turn off alarm
#else
  digitalWrite(ALARMPIN, HIGH); // Turn off alarm
#endif

#ifdef USE_LED
  pinMode(LEDRED, OUTPUT);

  pinMode(LEDGREEN, OUTPUT);
  ledred(); // red led test
  delay(500);
  ledoff();
  ledgreen(); // green led test
  delay(500);
  ledoff();
  ledyellow(); // yellow led test
  delay(500);
  ledoff();
#endif

  display.begin();
  display.setFontPosTop();

  display.clearBuffer();
  display.drawXBM(31, 0, 66, 64, garfield);
  display.sendBuffer();
  shortBeep();
  delay(1000);

#ifdef USE_WIFI_MANAGER
  WiFi.persistent(true);
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(600);
  wifiManager.autoConnect("IBEClock12864-HW");
  Serial.println("Please connect WiFi IBEClock12864-HW");
  drawProgress("请用手机设置本机WIFI", "SSID IBEClock12864-HW");
#else
  Serial.println("Scan WIFI");
  drawProgress("正在扫描WIFI...", "");
  int intPreferredWIFI = 0;
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  int n = WiFi.scanNetworks();
  if (n == 0)
  {
  }
  else
  {
    for (int i = 0; i < n; ++i)
    {
      for (int j = 0; j < numWIFIs; j++)
      {
        if (strcmp(WIFI_SSID[j], string2char(WiFi.SSID(i))) == 0)
        {
          intPreferredWIFI = j;
          break;
        }
      }
    }
  }
  Serial.println("Connect WIFI");

  WiFi.persistent(true);
  WiFi.begin(WIFI_SSID[intPreferredWIFI], WIFI_PWD[intPreferredWIFI]);
  drawProgress("正在连接WIFI...", WIFI_SSID[intPreferredWIFI]);
  int WIFIcounter = intPreferredWIFI;
  while (WiFi.status() != WL_CONNECTED) {
    int counter = 0;
    while (counter < WIFI_TRY && WiFi.status() != WL_CONNECTED)
    {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(500);
      if (WiFi.status() == WL_CONNECTED) break;
      counter++;
    }
    if (WiFi.status() == WL_CONNECTED) break;
    WIFIcounter++;
    if (WIFIcounter >= numWIFIs) WIFIcounter = 0;
    WiFi.begin(WIFI_SSID[WIFIcounter], WIFI_PWD[WIFIcounter]);
    drawProgress("正在连接WIFI...", WIFI_SSID[WIFIcounter]);
  }
#endif

  if (WiFi.status() != WL_CONNECTED) ESP.restart();

  // Get time from network time service
  Serial.println("WIFI Connected");
  drawProgress("连接WIFI成功,", "正在同步时间...");
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");
  drawProgress("同步时间成功,", "正在更新天气数据...");
  updateData(true);
  timeSinceLastWUpdate = millis();
  attachInterrupt(digitalPinToInterrupt(SMOKEPIN), smokeHandler, CHANGE);
}

void loop() {
  display.firstPage();
  do {
    draw();
    //    delay(100);
  } while ( display.nextPage() );
  draw_state++;
  if (draw_state >= 12) draw_state = 0;
  delay(2000);

  if (millis() - timeSinceLastWUpdate > (1000 * UPDATE_INTERVAL_SECS)) {
    setReadyForWeatherUpdate();
    timeSinceLastWUpdate = millis();
  }

#if (DHTPIN >= 0)
  if (dht.read())
  {
    float fltHumidity = dht.readHumidity();
    Serial.print("Humidity: ");
    Serial.println(fltHumidity);
    float fltCTemp = dht.readTemperature() - 1.5;
    Serial.print("CTemp: ");
    Serial.println(fltCTemp);
    if (isnan(fltCTemp) || isnan(fltHumidity))
    {
    }
    else
    {
      previousTemp = fltCTemp;
      previousHumidity = fltHumidity;
    }
  }
#endif

  if (readyForWeatherUpdate) {
    updateData(false);
  }
}

void draw(void) {
  // We get local date and time and compare to first forecast
  int forecastBase = 0;
  nowTime = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&nowTime);

  String strTempDate = String(forecasts[0].date);
  strTempDate.trim(); // 2018-08-09
  int day = (strTempDate.substring(8)).toInt();

  if (timeInfo->tm_mday != day)
  {
    // current date is different from forecast
    forecastBase = 1;
  }
  else if (timeInfo->tm_mday == day && timeInfo->tm_hour > 19)
  {
    // We are at current day, but already after 8PM
    forecastBase = 1;
  }

  //    display.drawXBM(31, 0, 66, 64, garfield);
  if (draw_state >= 0 && draw_state < 2)
  {
    drawLocal();
  }
  else if (draw_state >= 2 && draw_state < 4)
  {
#ifdef SHOW_US_CITIES
    drawWorldLocation("纽约", usET, currentWeather1);
#else
    drawLocal();
#endif
  }
  else if (draw_state >= 4 && draw_state < 6)
  {
#ifdef SHOW_US_CITIES
    drawWorldLocation("弗利蒙", usPT, currentWeather2);
#else
    drawLocal();
#endif
  }
  else if (draw_state >= 6 && draw_state < 8)
  {
    drawForecastDetails(0 + forecastBase);
  }
  else if (draw_state >= 8 && draw_state < 10)
  {
    drawForecastDetails(1 + forecastBase);
  }
  else if (draw_state >= 10 && draw_state < 12)
  {
    drawForecastDetails(2 + forecastBase);
  }
  else if (draw_state >= 12)
  {
    draw_state = 0;
  }
  else
  {
    drawLocal();
  }
}

void updateData(bool isInitialBoot) {
  nowTime = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&nowTime);
  if (isInitialBoot)
  {
    drawProgress("正在更新...", "本地天气实况...");
  }
  currentWeatherClient.updateCurrent(&currentWeather, HEWEATHER_APP_ID, HEWEATHER_LOCATION, HEWEATHER_LANGUAGE);

#ifdef SHOW_US_CITIES
  delay(300);
  if (isInitialBoot)
  {
    drawProgress("正在更新...", "纽约天气实况...");
  }
  currentWeatherClient1.updateCurrent(&currentWeather1, HEWEATHER_APP_ID, HEWEATHER_LOCATION1, HEWEATHER_LANGUAGE);
  delay(300);
  if (isInitialBoot)
  {
    drawProgress("正在更新...", "弗利蒙天气实况...");
  }
  currentWeatherClient2.updateCurrent(&currentWeather2, HEWEATHER_APP_ID, HEWEATHER_LOCATION2, HEWEATHER_LANGUAGE);
#endif
  
  if (isInitialBoot || timeInfo->tm_hour == 0 || timeInfo->tm_hour == 8 || timeInfo->tm_hour == 11 || timeInfo->tm_hour == 18)
  {
    delay(300);
    if (isInitialBoot)
    {
      drawProgress("正在更新...", "本地天气预报...");
    }
    int result = forecastClient.updateForecast(forecasts, HEWEATHER_APP_ID, HEWEATHER_LOCATION, HEWEATHER_LANGUAGE);
  }
  readyForWeatherUpdate = false;
}

String chooseMeteocon(String stringInput) {
  nowTime = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&nowTime);

  if (timeInfo->tm_hour > 6 && timeInfo->tm_hour < 18)
  {
    return stringInput.substring(0, 1);
  }
  else
  {
    return stringInput.substring(1, 2);
  }
}

#ifdef USE_LED
void processWeatherText(String inputText) {
  String returnText = inputText;
  returnText.trim();
  //  Serial.println(returnText);
  if  ((returnText.indexOf("暴") >= 0) || (returnText.indexOf("雹") >= 0) || (returnText.indexOf("雾") >= 0) || (returnText.indexOf("台风") >= 0) || (returnText.indexOf("冻") >= 0))
  {
    ledred();
  }
  else if ((returnText.indexOf("中雨") >= 0) || (returnText.indexOf("大雨") >= 0) || (returnText.indexOf("极端") >= 0) || (returnText.indexOf("雷") >= 0))
  {
    ledred();
  }
  else if ((returnText.indexOf("小雨") >= 0) ||  (returnText.indexOf("小雪") >= 0) || (returnText.indexOf("阵雨") >= 0) || (returnText.indexOf("毛雨") >= 0) || (returnText.indexOf("细雨") >= 0) || (returnText.indexOf("雨") >= 0))
  {
    ledyellow();
  }
  else if ((returnText.indexOf("多云") >= 0) || (returnText.indexOf("晴") >= 0) || (returnText.indexOf("少云") >= 0) || (returnText.indexOf("阴") >= 0))
  {
    ledgreen();
  }
  else if ((returnText.indexOf("有风") >= 0) || (returnText.indexOf("平静") >= 0) || (returnText.indexOf("微风") >= 0) || (returnText.indexOf("和风") >= 0))
  {
    ledgreen();
  }
  else if ((returnText.indexOf("强风") >= 0) || (returnText.indexOf("疾风") >= 0) || (returnText.indexOf("大风") >= 0))
  {
    ledyellow();
  }
  else if ((returnText.indexOf("烈风") >= 0) || (returnText.indexOf("风暴") >= 0))
  {
    ledred();
  }
  else if ((returnText.indexOf("狂爆风") >= 0) || (returnText.indexOf("飓") >= 0) || (returnText.indexOf("龙卷") >= 0))
  {
    ledred();
  }
  else if ((returnText.indexOf("小雪") >= 0))
  {
    ledyellow();
  }
  else if ((returnText.indexOf("中雪") >= 0) || (returnText.indexOf("大雪") >= 0) || (returnText.indexOf("夹雪") >= 0) || (returnText.indexOf("雨雪") >= 0) || (returnText.indexOf("阵雪") >= 0))
  {
    ledred();
  }
  else if ((returnText.indexOf("雪") >= 0))
  {
    ledyellow();
  }
  else if ((returnText.indexOf("尘暴") >= 0))
  {
    ledred();
  }
  else if ((returnText.indexOf("霾") >= 0) || (returnText.indexOf("热") >= 0) || (returnText.indexOf("冷") >= 0))
  {
    ledyellow();
  }
  else
  {
    ledgreen();
  }
}
#endif

void drawProgress(String labelLine1, String labelLine2) {
  display.clearBuffer();
  display.enableUTF8Print();
  display.setFont(u8g2_font_wqy12_t_gb2312); // u8g2_font_wqy12_t_gb2312, u8g2_font_helvB08_tf
  int stringWidth = 1;
  if (labelLine1 != "")
  {
    stringWidth = display.getUTF8Width(string2char(labelLine1));
    display.setCursor((128 - stringWidth) / 2, 13);
    display.print(labelLine1);
  }
  if (labelLine2 != "")
  {
    stringWidth = display.getUTF8Width(string2char(labelLine2));
    display.setCursor((128 - stringWidth) / 2, 36);
    display.print(labelLine2);
  }
  display.disableUTF8Print();
  display.sendBuffer();
}

void drawLocal() {
  nowTime = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&nowTime);
  char buff[20];

  display.enableUTF8Print();
  display.setFont(u8g2_font_wqy12_t_gb2312); // u8g2_font_wqy12_t_gb2312, u8g2_font_helvB08_tf
  String stringText = String(timeInfo->tm_year + 1900) + "年" + String(timeInfo->tm_mon + 1) + "月" + String(timeInfo->tm_mday) + "日 " + WDAY_NAMES[timeInfo->tm_wday].c_str();
  int stringWidth = display.getUTF8Width(string2char(stringText));
  display.setCursor((128 - stringWidth) / 2, 1);
  display.print(stringText);
  stringWidth = display.getUTF8Width(string2char(String(currentWeather.cond_txt)));
  display.setCursor((128 - stringWidth) / 2, 40);
  display.print(String(currentWeather.cond_txt));
  String WindDirectionAndSpeed = windDirectionTranslate(currentWeather.wind_dir) + String(currentWeather.wind_sc) + "级";
  stringWidth = display.getUTF8Width(string2char(WindDirectionAndSpeed));
  display.setCursor((128 - stringWidth) / 2, 54);
  display.print(WindDirectionAndSpeed);
  display.disableUTF8Print();
#ifdef USE_LED
  processWeatherText(String(currentWeather.cond_txt));
#endif
  display.setFont(u8g2_font_helvR24_tn); // u8g2_font_inb21_ mf, u8g2_font_helvR24_tn
  //  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);
  stringWidth = display.getStrWidth(buff);
  display.drawStr((128 - 30 - stringWidth) / 2, 11, buff);

  display.setFont(Meteocon21);
  display.drawStr(98, 17, string2char(chooseMeteocon(currentWeather.iconMeteoCon)));

  display.setFont(u8g2_font_helvR08_tf);
  String temp = String(currentWeather.tmp) + degree + "C";
  display.drawStr(0, 53, string2char(temp));

  display.setFont(u8g2_font_helvR08_tf);
  stringWidth = display.getStrWidth(string2char((String(currentWeather.hum) + "%")));
  display.drawStr(127 - stringWidth, 53, string2char((String(currentWeather.hum) + "%")));

  display.setFont(u8g2_font_helvB08_tf);
  if (previousTemp != 0 && previousHumidity != 0)
  {
    display.drawStr(0, 39, string2char(String(previousTemp, 0) + degree + "C"));
  }
  else
  {
    //    display.drawStr(0, 39, string2char("..."));
  }

  if (previousTemp != 0 && previousHumidity != 0)
  {
    int stringWidth = display.getStrWidth(string2char(String(previousHumidity, 0) + "%"));
    display.drawStr(128 - stringWidth, 39, string2char(String(previousHumidity, 0) + "%"));
  }
  else
  {
    //    int stringWidth = display.getStrWidth(string2char("..."));
    //    display.drawStr(128 - stringWidth, 39, string2char("..."));
  }
  display.drawHLine(0, 51, 128);
}

#ifdef SHOW_US_CITIES
void drawWorldLocation(String stringText, Timezone tztTimeZone, HeWeatherCurrentData currentWeather) {
  time_t utc = time(nullptr) - TZ_SEC;
  TimeChangeRule *tcr;        // pointer to the time change rule, use to get the TZ abbrev
  time_t t = tztTimeZone.toLocal(utc, &tcr);
  char buff[5];
  sprintf(buff, "%02d:%02d", hour(t), minute(t));
  display.enableUTF8Print();
  display.setFont(u8g2_font_wqy12_t_gb2312); // u8g2_font_wqy12_t_gb2312, u8g2_font_helvB08_tf
  String stringTemp = stringText + String(month(t)) + "月" + String(day(t)) + "日" + " " + WDAY_NAMES[weekday(t) - 1].c_str();
  int stringWidth = display.getUTF8Width(string2char(stringTemp));
  display.setCursor((128 - stringWidth) / 2, 1);
  display.print(stringTemp);
  stringWidth = display.getUTF8Width(string2char(String(currentWeather.cond_txt)));
  display.setCursor((128 - stringWidth) / 2, 40);
  display.print(String(currentWeather.cond_txt));
  String WindDirectionAndSpeed = windDirectionTranslate(currentWeather.wind_dir) + String(currentWeather.wind_sc) + "级";
  stringWidth = display.getUTF8Width(string2char(WindDirectionAndSpeed));
  display.setCursor((128 - stringWidth) / 2, 54);
  display.print(WindDirectionAndSpeed);
  display.disableUTF8Print();

  display.setFont(u8g2_font_helvR24_tn);
  //  stringTemp = String(hour(t)) + ":" + String(minute(t));
  stringWidth = display.getStrWidth(buff);
  display.drawStr((128 - 30 - stringWidth) / 2, 11, buff);
  String temp = String(currentWeather.tmp) + degree + "C";
  display.setFont(u8g2_font_helvR08_tf);
  display.drawStr(0, 53, string2char(temp));
  String tempHumidity = String(currentWeather.hum) + "%";
  stringWidth = display.getStrWidth(string2char(tempHumidity));
  display.setFont(u8g2_font_helvR08_tf);
  display.drawStr(128 - stringWidth, 53, string2char(tempHumidity));
  display.drawHLine(0, 51, 128);

  display.setFont(Meteocon21);
  display.drawStr(98, 17, string2char(String(currentWeather.iconMeteoCon).substring(0, 1)));
}
#endif

String windDirectionTranslate(String stringInput) {
  String stringReturn = stringInput;
  stringReturn.replace("N", "北");
  stringReturn.replace("S", "南");
  stringReturn.replace("E", "东");
  stringReturn.replace("W", "西");
  stringReturn.replace("无持续", "无");
  return stringReturn;
}

void drawForecast1() {
  drawForecastDetails(0);
}

void drawForecast2() {
  drawForecastDetails(1);
}

void drawForecastDetails(int dayIndex) {
  String strTempDate = String(forecasts[dayIndex].date);
  strTempDate.trim(); // 2018-08-09
  int year = (strTempDate.substring(0, 4)).toInt();
  int month = (strTempDate.substring(5, 7)).toInt();
  int day = (strTempDate.substring(8)).toInt();
  struct tm *idotmpstruct, timetmps32;
  time_t observationTimestamp;
  observationTimestamp = 0;
  idotmpstruct = &timetmps32;
  idotmpstruct->tm_year = year - 1900;
  idotmpstruct->tm_mon = month - 1;
  idotmpstruct->tm_mday = day;
  idotmpstruct->tm_hour = 1;
  idotmpstruct->tm_min = 0;
  idotmpstruct->tm_sec = 0;
  idotmpstruct->tm_wday = 0;  /*/dummy */
  observationTimestamp = mktime(idotmpstruct);   /*/elllit timestamp */
  struct tm* timeInfo;
  timeInfo = localtime(&observationTimestamp);

  display.enableUTF8Print();
  display.setFont(u8g2_font_wqy12_t_gb2312); // u8g2_font_wqy12_t_gb2312, u8g2_font_helvB08_tf
  String stringText = " " + String(timeInfo->tm_mon + 1) + "月" + String(timeInfo->tm_mday) + "日 " + String(WDAY_NAMES[timeInfo->tm_wday].c_str());
  int stringWidth = display.getUTF8Width(string2char(stringText));
  display.setCursor(0, 1);
  display.print(stringText);

  // each Chinese character's length is 3 in UTF-8
  stringText = String("日:" + forecasts[dayIndex].cond_txt_d);
  stringText.replace("\"", "");
  stringText.trim();
  if (stringText.length() > 21)
  {
    stringText = stringText.substring(0, 21);
    stringText.trim();
  }
  display.setCursor(26, 18);
  display.print(stringText);

  // each Chinese character's length is 3 in UTF-8
  stringText = String("夜:" + forecasts[dayIndex].cond_txt_n);
  stringText.replace("\"", "");
  stringText.trim();
  if (stringText.length() > 21)
  {
    stringText = stringText.substring(0, 21);
    stringText.trim();
  }
  display.setCursor(26, 36);
  display.print(stringText);

  stringText = windDirectionTranslate(String(forecasts[dayIndex].wind_dir)) + String(forecasts[dayIndex].wind_sc) + "级";
  stringWidth = display.getUTF8Width(string2char(stringText));
  display.setCursor(0, 54);
  display.print(stringText);
  display.disableUTF8Print();

  display.setFont(u8g2_font_helvR08_tf);
  stringText = String(forecasts[dayIndex].hum) + "%";
  stringWidth = display.getStrWidth(string2char(stringText));
  display.drawStr(128 - stringWidth, 1, string2char(stringText));

  stringText = String(forecasts[dayIndex].tmp_max) + degree + "C";
  stringWidth = display.getStrWidth(string2char(stringText));
  display.drawStr(128 - stringWidth, 18, string2char(stringText));

  stringText = String(forecasts[dayIndex].tmp_min) + degree + "C";
  stringWidth = display.getStrWidth(string2char(stringText));
  display.drawStr(128 - stringWidth, 35, string2char(stringText));

  stringText = String(String(forecasts[dayIndex].pcpn, 1) + "mm") + "  " + String(forecasts[dayIndex].pop) + "%";
  stringWidth = display.getStrWidth(string2char(stringText));
  display.drawStr(128 - stringWidth, 53, string2char(stringText));

  display.setFont(Meteocon21);
  display.drawStr(0, 19, string2char(String(forecasts[dayIndex].iconMeteoCon).substring(0, 1)));
}

void setReadyForWeatherUpdate() {
  readyForWeatherUpdate = true;
}

char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

void shortBeep() {
#ifdef USE_HIGH_ALARM
  digitalWrite(ALARMPIN, HIGH);
  delay(150);
  digitalWrite(ALARMPIN, LOW);
#else
  digitalWrite(ALARMPIN, LOW);
  delay(150);
  digitalWrite(ALARMPIN, HIGH);
#endif
}

void longBeep() {
#ifdef USE_HIGH_ALARM
  digitalWrite(ALARMPIN, HIGH);
  delay(2000);
  digitalWrite(ALARMPIN, LOW);
#else
  digitalWrite(ALARMPIN, LOW);
  delay(2000);
  digitalWrite(ALARMPIN, HIGH);
#endif
}


