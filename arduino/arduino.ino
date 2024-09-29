#include <SoftwareSerial.h>

SoftwareSerial GPSModule(4, 3);
#define echoSensor 8
#define trigSensor 9

int updates;
int failedUpdates;
int pos;
int stringplace = 0;

String timeUp;
String nmea[15];
float latitude = 0, longitude = 0;

// Função para verificar se a vaga está ocupada
bool vagaOcupada() {
  bool ocupada = false;

  digitalWrite(trigSensor, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigSensor, LOW);

  double distancia = pulseIn(echoSensor, HIGH);

  distancia = distancia * 0.34 / 2;

  Serial.println("Distância: " + String(distancia));

  if (distancia < 100) {
    ocupada = true;
  }

  return ocupada;
}

// Função para atualizar as coordenadas do GPS
void updateLocation() {
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
    Serial.println("Atualização falhou");
    failedUpdates++;
  }
  stringplace = 0;
  pos = 0;
}

// Conversão da latitude do formato NMEA para decimal
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

// Conversão da longitude do formato NMEA para decimal
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
      lngsecond = nmea[4].substring(i - 2).toFloat();
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

void setup() {
  Serial.begin(115200);   // Comunicação com o ESP8266
  GPSModule.begin(9600);  // Comunicação com o módulo GPS

  pinMode(trigSensor, OUTPUT);
  pinMode(echoSensor, INPUT);
}

void loop() {
  updateLocation();  // Atualiza a localização
  Serial.println("---------------------");
  Serial.println("Latitude: " + String(latitude, 6));
  Serial.println("Longitude: " + String(longitude, 6));

  bool ocupada = vagaOcupada();

  if (ocupada) {
    // Envia dados via serial para o ESP8266 quando a vaga estiver ocupada
    String data = "OCUPADO," + String(latitude, 6) + "," + String(longitude, 6);
    Serial.println(data);

    if (Serial.available()) {
      String receivedData = Serial.readStringUntil('\n');
      Serial.println("Recebido: " + receivedData);
    }
    else {
      Serial.println("Sem resposta do WIFI");
    }
  }

  delay(1000);  // Pequeno atraso antes de repetir o loop
}
