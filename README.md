# AirGradient-Influx
Save data from the AirGradient DIY Pro air quality monitor to a locally hosted Influx DB 2 instance.

## Setup
1. Install libraries
    * ESP8266 by ESP8266 Community v2.7.4 (in Board Manager) *
    * AirGradient Air Quality Sensor by AirGradient v2.2.0
    * WifiManager by tzapu, tablatronix v2.0.11-beta
    * U8g2 by oliver v2.32.12
1. `mv` or rename `arduino_secrets.h.example` to `arduino_secrets.h`
1. In Influx DB: create a new org and bucket for your AirGradient
1. Go to Load Data > Sources > Arduino
1. Copy `INFLUXDB_URL`, `INFLUXDB_TOKEN`, `INFLUXDB_ORG`, `INFLUXDB_BUCKET` to `arduino_secrets.h`
1. Find `TZ_INFO` in the sketch file and set your timezone [from this list](https://github.com/openwrt/luci/blob/master/modules/luci-lua-runtime/luasrc/sys/zoneinfo/tzdata.lua). The default is `GMT0`
1. Verify and upload

## Displaying the data in Grafana
1. In Influx DB: create a new read-only API token for Grafana
2. Add a new data source in Grafana using the API token (Change Query Language to `Flux`)
3. Create a new panel in your dashboard and select the AirGradient data source
4. Use the InfluxDB Data Explorer to create the Flux queries for you

## Other
* Tested with PCB v3.3
* OLED brightness can be adjusted by setting `u8g2.setContrast(value)` in `setup()`
  * Default is `50`
  * `value`: 0 to 255

\* I experienced frequent crashing using anything newer than ESP8266 v2.7.4, YMMV
