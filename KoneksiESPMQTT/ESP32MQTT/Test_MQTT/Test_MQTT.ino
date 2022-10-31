/*
================================================================================================
Test MQTT ESP-32
------------------------------------------------------------------------------------------------
Test koneksi ESP32 ke MQTT Broker menggunakan HIVEMQ
Menampilkan hasil pengukuran sensor PZEM-004T pada HIVEMQ
Data Sensor : Voltage, Current, Power, Energy, Frequency, Power Factor
================================================================================================
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>

// Update these with values suitable for your network.

const char* ssid = "aufarquinsa";
const char* password = "FAR291296";
const char* mqtt_server = "broker.mqtt-dashboard.com";
//const char* mqtt_server = "rmq2.pptik.id";
//const char* mqtt_user = "TMDG2022";
//const char* mqtt_pass = "TMDG2022";
byte mac[6]; //array temp mac address
String MACAddress;
const char* deviceGuid = "ESP32Client";
int loop_count = 0 ;
//==============================================================================================
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif


#if defined(ESP32)
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
#elif defined(ESP8266)

//PZEM004Tv30 pzem(Serial1);
#else

PZEM004Tv30 pzem(PZEM_SERIAL);
#endif
//===================================================================================

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
byte LEDPin = 13;

void setup() {
  pinMode(LEDPin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
//==============================================================================================
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//==============================================================================================
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LEDPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(LEDPin, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
//==============================================================================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
    //if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("/topik/mqtt/outTopic", "SHA.ME");
      //client.publish("Sensor_PZEM004T", "SHA.ME");
      // ... and resubscribe
      //client.subscribe("/topik/mqtt/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//==============================================================================================

void loop() {
// Read the data from the sensor
float voltage = pzem.voltage();
float current = pzem.current();
float power = pzem.power();
float energy = pzem.energy();
float frequency = pzem.frequency();
float pf = pzem.pf();
String dataSendtoMqtt;
String convertDeviceGuid = String(deviceGuid);

for (int i = 0; i <= loop_count; i++) {
    if (!client.connected()) {
    reconnect();
    }

  // Check if the data is valid
    if (isnan(voltage)) {
      Serial.println("Error reading voltage");
    } else if (isnan(current)) {
      Serial.println("Error reading current");
    } else if (isnan(power)) {
      Serial.println("Error reading power");
    } else if (isnan(energy)) {
      Serial.println("Error reading energy");
    } else if (isnan(frequency)) {
      Serial.println("Error reading frequency");
    } else if (isnan(pf)) {
      Serial.println("Error reading power factor");
    }
}
  
  dataSendtoMqtt = String(convertDeviceGuid + " #Voltage : " + voltage + " #Current : " + current + " #Power : " + power + " #Energy : " + energy + " #Frequency : " + frequency + " #Power Factor : " + pf);
  client.publish("/topik/mqtt/outTopic", dataSendtoMqtt.c_str());
  
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "Hasil Pengukuran Ke #%ld", value);
    //snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("/topik/mqtt/outTopic", msg);
  }
 
  client.loop();
  delay(5000);

 
}
