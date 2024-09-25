#include <SoftwareSerial.h>
#include <WiFi.h>

SoftwareSerial GPSModule(4, 3);  // RX, TX
int updates;
int failedUpdates;
int pos;
int stringplace = 0;

String timeUp;
String nmea[15];
float latitude = 0, longitude = 0;

void setup() {
  Serial.begin(57600);
  GPSModule.begin(9600);
  char* ssid = "ap102";
  char* password = "cincopila";
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("tentando conectar");
  }

  
    Serial.println(WiFi.localIP());
}

void loop() {
  Serial.println("---------------------");
  Serial.println("Latitude");
  Serial.println(latitude);
  Serial.println("Longitude");
  Serial.println(longitude);

  UpdateLocation();

  delay(1000);
}

void UpdateLocation() {
  Serial.flush();
  GPSModule.flush();
  while (GPSModule.available() > 0) {
    GPSModule.read();
  }
  if (GPSModule.find("$GPRMC,")) {
    String tempMsg = GPSModule.readStringUntil('\n');
    for (int i = 0; i < tempMsg.length(); i++) {
      if (tempMsg.substring(i, i + 1) == ",") {
        nmea[pos] = tempMsg.substring(stringplace, i);
        stringplace = i + 1;
        pos++;
      }
      if (i == tempMsg.length() - 1) {
        nmea[pos] = tempMsg.substring(stringplace, i);
      }
    }
    updates++;
    nmea[2] = ConvertLat();
    nmea[4] = ConvertLng();
    latitude = nmea[2].toDouble();
    longitude = nmea[4].toDouble();
  } else {
    failedUpdates++;
  }
  stringplace = 0;
  pos = 0;
}

String ConvertLat() {
  String posneg = "";
  if (nmea[3] == "S") {
    posneg = "-";
  }
  String latfirst;
  float latsecond;
  for (int i = 0; i < nmea[2].length(); i++) {
    if (nmea[2].substring(i, i + 1) == ".") {
      latfirst = nmea[2].substring(0, i - 2);
      latsecond = nmea[2].substring(i - 2).toFloat();
    }
  }
  latsecond = latsecond / 60;
  String CalcLat = "";

  char charVal[9];
  dtostrf(latsecond, 4, 6, charVal);
  for (int i = 0; i < sizeof(charVal); i++) {
    CalcLat += charVal[i];
  }
  latfirst += CalcLat.substring(1);
  latfirst = posneg += latfirst;
  return latfirst;
}

String ConvertLng() {
  String posneg = "";
  if (nmea[5] == "W") {
    posneg = "-";
  }

  String lngfirst;
  float lngsecond;
  for (int i = 0; i < nmea[4].length(); i++) {
    if (nmea[4].substring(i, i + 1) == ".") {
      lngfirst = nmea[4].substring(0, i - 2);
      //Serial.println(lngfirst);
      lngsecond = nmea[4].substring(i - 2).toFloat();
      //Serial.println(lngsecond);
    }
  }
  lngsecond = lngsecond / 60;
  String CalcLng = "";
  char charVal[9];
  dtostrf(lngsecond, 4, 6, charVal);
  for (int i = 0; i < sizeof(charVal); i++) {
    CalcLng += charVal[i];
  }
  lngfirst += CalcLng.substring(1);
  lngfirst = posneg += lngfirst;
  return lngfirst;
}