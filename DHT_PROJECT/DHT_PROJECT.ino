#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "in.pool.ntp.org");
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int today = 1;

#define FIREBASE_HOST "Your Firebase Database Link"
#define FIREBASE_AUTH "Your Firebase Database Authentication Key"

#define WIFI_SSID "Your WiFi SSID"
#define WIFI_PASSWORD "Your WiFi Password"

#define DHTPIN D1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(19800);
  timeClient.update();
  today =  timeClient.getDay() + 1;
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  dht.begin();
}

void loop() {
  
  // put your main code here, to run repeatedly:

  timeClient.update();
  int day = checkDay();
  int hour = checkHours();
  int minutes = checkMinutes();
  int seconds = checkSeconds();
  
  float humid = dht.readHumidity();
  float temp = dht.readTemperature();
  if (isnan(humid) || isnan(temp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humid);
  String fireHumid = String(humid);
  Serial.print("% Temperature: ");
  Serial.print(temp);
  Serial.println("°C");
  String fireTemp = String(temp);
  delay(7000);
  Firebase.pushString("/DHT11/Humidity", fireHumid);
  Firebase.pushString("/DHT11/Temperature", fireTemp);
  
  if (hour == 13 && minutes == 13 && day != today) {
      today = day;
      int f = 0;
      float sumTemp = 0;
      float sumHumid = 0;
      while (f < 10) {
         float temp = dht.readTemperature();
         float humid = dht.readTemperature();
         sumTemp += temp;
         sumHumid += humid;
         f++; 
      }
      float avgTemp = sumTemp/10;
      float avgHumid = sumHumid/10;
      String fireAvgTemp = String(avgTemp);
      String fireAvgHumid = String(avgHumid);
      String fireDay = weekDays[timeClient.getDay()];
      Firebase.pushString("/DHT11/AvgTemp", fireAvgTemp);
      Firebase.pushString("/DHT11/AvgHumidity", fireAvgHumid);
      Firebase.pushString("/DHT11/Day", fireDay);
  }
  
  if (Firebase.failed()) {
      Serial.print("pushing /logs failed:");
      Serial.println(Firebase.error()); 
      return;
  }
}

int checkDay() {
  return timeClient.getDay();
}

int checkHours(){
  return timeClient.getHours();
}

int checkMinutes() {
  return timeClient.getMinutes();
}

int checkSeconds() {
  return timeClient.getSeconds();
}
