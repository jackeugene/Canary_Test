//https://github.com/arduino-libraries/ArduinoMqttClient/blob/master/examples/WiFiSimpleSender/WiFiSimpleSender.ino

#include <ArduinoMqttClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFiNINA.h>

#define DHTPIN 7
#define DHTTYPE DHT22   // DHT 22  (AM2302)

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "Archimedes";        // your network SSID (name)
char pass[] = "f29070930f";    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

DHT dht = DHT(DHTPIN, DHTTYPE);

const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic[]  = "house/porch/temp";

const long interval = 2000;
unsigned long previousMillis = 0;

int count = 0;
float h;
float t;
float f;
String envData;

void initWifi()
{
   // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();
  
}

void initMqtt()
{
   // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  
}

void getEnv()
{
  // Read the humidity in %:
  int h = (int) dht.readHumidity();
  // Read the temperature as Celsius:
  int t = (int) dht.readTemperature();
  // Read the temperature as Fahrenheit:
  int f = (int) dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again):
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

   // Compute heat index in Fahrenheit (default):
  int hif = (int) dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius:
  int hic = (int) dht.computeHeatIndex(t, h, false);

  

  String humidity =  String(h, DEC);  
  String tempC =  String(t,DEC);
  String tempF = String(f,DEC);
  String fIndex = String(hif,DEC);
  String cIndex = String(hic,DEC);

  envData = humidity + "," + tempC + "," + tempF + "," + fIndex + "," + cIndex;
  Serial.println(envData);

  

  

  //Serial.print("Humidity: ");
  //Serial.print(h);
  //Serial.print(" % ");
  //Serial.print("Temperature: ");
  //Serial.print(t);
  //Serial.print(" \xC2\xB0");
  //Serial.print("C | ");
  //Serial.print(f);
  //Serial.print(" \xC2\xB0");
  //Serial.print("F ");
  //Serial.print("Heat index: ");
  //Serial.print(hic);
  //Serial.print(" \xC2\xB0");
  //Serial.print("C | ");
  //Serial.print(hif);
  //Serial.print(" \xC2\xB0");
  //Serial.println("F");



}

void mqttPub()
{
  Serial.print("Sending message to topic: ");
  Serial.println(topic);
  Serial.print("hello ");
  Serial.println(count);

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic);
  mqttClient.print(envData);
  mqttClient.endMessage();

  Serial.println();
  
}


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  initWifi();
  initMqtt();

  dht.begin();
  delay(2000);
}
  
void loop() {
   // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    getEnv();
    mqttPub();

    count++;
  }

}
