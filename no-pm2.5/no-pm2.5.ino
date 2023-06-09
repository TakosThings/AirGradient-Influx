/*
This is the code for the AirGradient DIY PRO Air Quality Sensor with an ESP8266 Microcontroller.

It is a high quality sensor showing PM2.5, CO2, Temperature and Humidity on a small display and can send data over Wifi.

Build Instructions: https://www.airgradient.com/open-airgradient/instructions/diy-pro/

Kits (including a pre-soldered version) are available: https://www.airgradient.com/open-airgradient/kits/

The codes needs the following libraries installed:
“WifiManager by tzapu, tablatronix” tested with version 2.0.11-beta
“U8g2” by oliver tested with version 2.32.15

Configuration:
Please set in the code below the configuration parameters.

If you have any questions please visit our forum at https://forum.airgradient.com/

If you are a school or university contact us for a free trial on the AirGradient platform.
https://www.airgradient.com/

MIT License

*/

#include <AirGradient.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <U8g2lib.h>
#include <InfluxDbClient.h>
#include "arduino_secrets.h"

AirGradient ag = AirGradient();
InfluxDBClient influxClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point influxSensor("airgradient");

// Display bottom right
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// CONFIGURATION START

// set to true to switch from Celcius to Fahrenheit
boolean inF = false;

// set to true if you want to connect to wifi. You have 60 seconds to connect. Then it will go into an offline mode.
boolean connectWIFI=true;

// Set Timezone
#define TZ_INFO "GMT0"

// CONFIGURATION END

// INTERVALS START

unsigned long currentMillis = 0;

const int oledInterval = 5000;
unsigned long previousOled = 0;

const int sendToServerInterval = 10000;
unsigned long previoussendToServer = 0;

const int co2Interval = 5000;
unsigned long previousCo2 = 0;
int Co2 = 0;

const int tempHumInterval = 5000;
unsigned long previousTempHum = 0;
float temp = 0;
int hum = 0;

// INTERVALS END

void setup() {
  Serial.begin(115200);

  u8g2.begin();
  u8g2.setContrast(50);
  updateOLED();

  if (connectWIFI) {
    connectToWifi();
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  }

  updateOLED2("Warming up the", "sensors.", "");

  ag.CO2_Init();
  ag.TMP_RH_Init(0x44);
}


void loop() {
  currentMillis = millis();
  updateOLED();
  updateCo2();
  updateTempHum();
  sendToServer();
}

void updateCo2() {
  if (currentMillis - previousCo2 >= co2Interval) {
    previousCo2 += co2Interval;
    Co2 = ag.getCO2_Raw();
    Serial.println();
    Serial.print("Co2 :");
    Serial.println(String(Co2));
  }
}

void updateTempHum(){
  if (currentMillis - previousTempHum >= tempHumInterval) {
    previousTempHum += tempHumInterval;
    TMP_RH result = ag.periodicFetchData();
    temp = result.t;
    hum = result.rh;
    Serial.print("Temp: ");
    Serial.println(String(temp));
    Serial.print("Hum : ");
    Serial.println(String(hum));
  }
}

void updateOLED() {
  if (currentMillis - previousOled >= oledInterval) {
    previousOled += oledInterval;

    String ln3;
    String ln1 = "";
    String ln2 = "CO2:" + String(Co2);

    if (inF) {
      ln3 = "F:" + String((temp* 9 / 5) + 32) + " H:" + String(hum)+"%";
    } else {
      ln3 = "C:" + String(temp) + " H:" + String(hum)+"%";
    }
    updateOLED2(ln1, ln2, ln3);
  }
}

void updateOLED2(String ln1, String ln2, String ln3) {
  char buf[9];
  u8g2.firstPage();
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_t0_16_tf);
    u8g2.drawStr(1, 10, String(ln1).c_str());
    u8g2.drawStr(1, 30, String(ln2).c_str());
    u8g2.drawStr(1, 50, String(ln3).c_str());
  } while ( u8g2.nextPage() );
}

void sendToServer() {
  if (currentMillis - previoussendToServer >= sendToServerInterval) {
    previoussendToServer += sendToServerInterval;

    if(WiFi.status()== WL_CONNECTED) {
      // Clear fields
      influxSensor.clearFields();

      influxSensor.addField("co2", Co2);
      influxSensor.addField("temp", temp);
      influxSensor.addField("hum", hum);

      // Print what will be written
      Serial.print("Writing: ");
      Serial.println(influxSensor.toLineProtocol());

      // Write
      if (!influxClient.writePoint(influxSensor)) {
        Serial.print("InfluxDB write failed: ");
        Serial.println(influxClient.getLastErrorMessage());
      }
    } else {
      Serial.println("WiFi Disconnected");
    }
  }
}

// Wifi Manager
void connectToWifi() {
  WiFiManager wifiManager;
  //WiFi.disconnect(); //to delete previous saved hotspot
  String HOTSPOT = "AG-" + String(ESP.getChipId(), HEX);
  updateOLED2("60s to connect", "to Wifi Hotspot", HOTSPOT);
  wifiManager.setTimeout(60);
  if (!wifiManager.autoConnect((const char * ) HOTSPOT.c_str())) {
    updateOLED2("booting into", "offline mode", "");
    Serial.println("failed to connect and hit timeout");
    delay(6000);
  }
}

// Influx connection
void connectToInfluxDb() {
  if (influxClient.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(influxClient.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(influxClient.getLastErrorMessage());
    updateOLED2("Influx connection", "failed", "");
  }
}
