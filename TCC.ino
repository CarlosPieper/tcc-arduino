#include <SoftwareSerial.h>
#include <WiFiEsp.h>

SoftwareSerial GPSModule(4, 3);
SoftwareSerial wifiModule(7,6);
#define echoSensor 8
#define trigSensor 9

const char rede[] = "ap-102";
const char senha[] = "cincopila";

bool wifiConectado = false;

WiFiEspServer server(80);
 
RingBuffer buf(8)

int updates;
int failedUpdates;
int pos;
int stringplace = 0;

String timeUp;
String nmea[15];
float latitude = 0, longitude = 0;

bool vagaOcupada() {
  bool ocupada = false;

  digitalWrite(trigSensor, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigSensor, LOW);

  double distancia = pulseIn(echoSensor, HIGH);
  Serial.print("distancia pega ");
  Serial.print(distancia);

  distancia *=340;
  distancia /= 2;
  distancia /= 10000; 

  Serial.print("distancia: ");
  Serial.print(distancia);

  if(distancia < 100){
    ocupada = true;
  }

  return ocupada;
}

void updateLocation(){
  Serial.flush();
  GPSModule.flush();
  while (GPSModule.available() > 0)
  {
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
  }
  else {
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
  for (int i = 0; i < sizeof(charVal); i++)
  {
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
  for (int i = 0; i < sizeof(charVal); i++)
  {
    CalcLng += charVal[i];
  }
  lngfirst += CalcLng.substring(1);
  lngfirst = posneg += lngfirst;
  return lngfirst;
}


void setup() {
  Serial.begin(57600);
  GPSModule.begin(9600);
  wifiModule.begin(115200);
  WiFi.init(&wifiModule);
  WiFi.config(IPAddress(192,168,0,110));

  if(WiFi.status() == WL_NO_SHIELD){
    while (true);
  }
  while(status != WL_CONNECTED){
    status = WiFi.begin(ssid, pass);
  }
  server.begin();

  pinMode(trigSensor, OUTPUT);
  pinMode(echoSensor, INPUT);
}

void loop() {
  WiFiEspClient client = server.available();
  
  Serial.println("---------------------");
  Serial.println("Latitude");
  Serial.println(latitude);
  Serial.println("Longitude");
  Serial.println(longitude);
  
  bool ocupada = vagaOcupada();

  if(ocupada){
    updateLocation();
  }
  
  if(wifiModule.available())       /* Confere se a comunicação está acessível */
  {
    while(wifiModule.available()) {  /* Enquanto estiver acessível */
      char c = wifiModule.read();      /* Le o caractere. */
      Serial.write(c);              /* Escreve no monitor serial */
    }
  }

  delay(1000);
}