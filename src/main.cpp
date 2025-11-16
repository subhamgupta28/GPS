#include "Automata.h"
#include "ArduinoJson.h"
#include <TinyGPS++.h>
#include <Wire.h>
#include "TimeLib.h"
#define USE_HTTPS 1

#define RXD2 17
#define TXD2 18

const char *HOST = "automata.realsubhamgupta.in";
int PORT = 443;

// const char *HOST = "raspberry.local";
// int PORT = 8010;

// Automata automata("GPS", HOST, PORT);
Automata automata("GPS", HOST, PORT, "0.tcp.in.ngrok.io", 14730);
Preferences preferences;
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;
long start2 = millis();
JsonDocument doc;

void action(const Action action)
{
  int sp = action.data["speed"];

  String jsonString;
  serializeJson(action.data, jsonString);
  Serial.println(jsonString);
}

void sendData()
{
  Serial.println("sent data");
  automata.sendData(doc);
}

void setup()
{
  Serial.begin(115200);
  gpsSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  automata.begin();
  automata.addAttribute("SATS", "Satellite", "", "DATA|MAIN");
  automata.addAttribute("LAT", "Latitude", "", "DATA|MAP");
  automata.addAttribute("LONG", "Longitude", "", "DATA|MAP");

  automata.addAttribute("SPEED", "Speed", "Km/h", "DATA|MAIN");
  // automata.addAttribute("DATE", "Date", "", "DATA|AUX");
  // automata.addAttribute("TIME", "Time", "", "DATA|AUX");

  // automata.addAttribute("HOUR", "Hour", "", "DATA|AUX");
  // automata.addAttribute("MIN", "Minutes", "", "DATA|AUX");
  // automata.addAttribute("SEC", "Seconds", "", "DATA|AUX");

  automata.addAttribute("ALT", "Altitude", "", "DATA|MAIN");
  automata.addAttribute("FIX", "Fix", "", "DATA|MAIN");
  automata.addAttribute("COURSE", "Course", "", "DATA|AUX");

  automata.addAttribute("HDOP", "HDOP", "", "DATA|AUX");

  automata.registerDevice();
  automata.onActionReceived(action);
  automata.delayedUpdate(sendData);
  Serial.println("Device booted");
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  while (gpsSerial.available())
    gps.encode(gpsSerial.read());
  // do
  // {
  //   while (gpsSerial.available())
  //     gps.encode(gpsSerial.read());
  // } while ((millis() - start) > ms);
}

void loop()
{
  // Serial.println("loop");
  smartDelay(20);
  // unsigned long timestamp = now();

  doc["SATS"] = String(gps.satellites.value());
  doc["LAT"] = String(gps.location.lat(), 8);
  doc["LONG"] = String(gps.location.lng(), 8);
  doc["SPEED"] = String(gps.speed.kmph());
  // doc["DATE"] = String(String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year()));
  // doc["TIME"] = String(timestamp);
  // doc["HOUR"] = String(gps.time.hour());
  // doc["MIN"] = String(gps.time.minute());
  // doc["SEC"] = String(gps.time.second());
  doc["ALT"] = String(gps.altitude.meters());
  doc["FIX"] = gps.location.age();
  doc["COURSE"] = String(gps.course.deg(), 2);
  doc["HDOP"] = String(gps.hdop.hdop(), 2);

  Serial.println("loop");

  if ((millis() - start2) > 1000 && gps.location.isValid())
  {
    automata.sendLive(doc);
    Serial.println("sent live");
    Serial.println(gps.location.lat());
    Serial.println(gps.location.lng());
    start2 = millis();
  }
  delay(100);
}
