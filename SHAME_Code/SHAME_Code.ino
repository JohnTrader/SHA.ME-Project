/*
SHA.ME : Smart Home Assistant for Monitoring Energy
===============================================================================================
Platform Smart Home yang digunakan untuk monitoring dan efisiensi 
pemakaian energi suatu rumah

Tubes Desain Aplikasi Interaktif 2022
Kelompok : Kendali-2
           Joni Juliansah
           Muhammad Fadlan

Refference : Jakub Mandula, Fahmi Nurfadilah, Nurman Hariyanto
=================================================================================================
*/

#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <PubSubClient.h>

//Definisi WIFI
#define WIFI_NETWORK "aufarquinsa"
#define WIFI_PASSWORD "FAR291296"
//#define WIFI_NETWORK "OPPO A5 2020"
//#define WIFI_PASSWORD "796cad6039d3"
#define WIFI_TIMEOUT_MS 20000

byte mac[6]; //array temp mac address
String MACAddress;
const char* deviceGuid = "d15016e1-9411-4a87-92c0-618e49465bfd";
const char* mqtt_server = "rmq2.pptik.id";
const char* mqtt_user = "TMDG2022";
const char* mqtt_pass = "TMDG2022";
const char* mqttQueuePublish = "Sensor_PZEM004T";
const char* mqttQueueSubscribe    = "Aktuator";

int loop_count = 0 ;


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

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

//============================================================================================
//Function for Get message payload from MQTT rabbit mq

char sPayload[100];
char message [40] ;
char address[40];
void callback(char* topic, byte* payload, unsigned int length) {
  memcpy(sPayload, payload, length);
  memcpy(address, payload, 36);
  memcpy(message, &payload[37], length - 37);
  if (String((char *)address) == String((char *)deviceGuid))
  {
    Serial.println("address sama");
  }
  else
  {
    Serial.println("address berbeda");
    return;
  }
  Serial.println(message);
/*
  if (message[0] == '1') {
    digitalWrite(relayControlPin, HIGH);
    Serial.println("relay 1 on");
    statusDevice[0] = "1";

  }
  if (message[0] == '0') {
    digitalWrite(relayControlPin, LOW);
    Serial.println("relay 1 off");
    statusDevice[0] = "0";
  } */
} 

//============================================================================================
// Fungsi untuk koneksi dan rekoneksi ke RabbitMQ
void reconnect() {
  // Loop until we're reconnected
  printMACAddress();
  const char* CL;
  CL = MACAddress.c_str();
  Serial.println(CL);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(CL, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe(mqttQueueSubscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      ESP.restart();
      delay(5000);

    }
  }
}
//============================================================================================
//Koneksi WiFi
void ConnectToWIFI(){
  Serial.print("Menghubungkan WIFI");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

  unsigned long startAttempTime = millis();

  while(WiFi.status() != WL_CONNECTED && millis()-startAttempTime < WIFI_TIMEOUT_MS){
    Serial.print(".");
    delay(100);
  }
  if(WiFi.status() != WL_CONNECTED){
    Serial.println ("Gagal!");
  }
  else{
    Serial.print("Tersambung IP ");
    Serial.println(WiFi.localIP());
  }
}

//============================================================================================
//Fungsi macaddress to String
String mac2String(byte ar[]) {
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%2X", ar[i]);
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}
//============================================================================================
//Print MAC Address
void printMACAddress() {
  WiFi.macAddress(mac);
  MACAddress = mac2String(mac);
  Serial.println(MACAddress);
}
//============================================================================================
//void watchdogSetup(void) {
//  ESP.wdtDisable();
//}
//============================================================================================
//Fungsi Setup
void setup() {
    // Debugging Serial port
    Serial.begin(115200);
    ConnectToWIFI();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    // Reset Energy Internal Counter
    //pzem.resetEnergy();
    delay(1500);
    //watchdogSetup();
}
//============================================================================================
//Main Program
void loop() {
     
    // Print the custom address of the PZEM
    //Serial.print("Custom Address:");
    //Serial.println(pzem.readAddress(), HEX);

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

    // Cek jika data valid
    if(isnan(voltage)){
      Serial.println("Kesalahan Pembacaan Voltage");
    } else if (isnan(current)) {
      Serial.println("Kesalahan Pembacaan Current");
    } else if (isnan(power)) {
      Serial.println("Kesalahan Pembacaan Power");
    } else if (isnan(energy)) {
      Serial.println("Kesalahan Pembacaan Energy");
    } else if (isnan(frequency)) {
      Serial.println("Kesalahan Pembacaan Frequency");
    } else if (isnan(pf)) {
      Serial.println("Kesalahan Pembacaan Power Factor");
    } /*else {

        // Print the values to the Serial console
        Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
        Serial.print("PF: ");           Serial.println(pf);
      } */
    
    dataSendtoMqtt = String(convertDeviceGuid + " #Voltage : " + voltage + "#Current : " + current + "#Power : " + power + "#Energy : " + energy + "#Frequency : " + frequency + "#Power Factor : " + pf);
    client.publish(mqttQueuePublish, dataSendtoMqtt.c_str());
    //loop_count++;
    //ESP.wdtFeed();
    //Serial.print(loop_count);
    //Serial.print(". Watchdog fed in approx. ");
    //Serial.print(loop_count * 5000);
    //Serial.println(" milliseconds.");
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "Hasil Pengukuran Ke #%ld", value);
    //snprintf (msg, 75, "Hasil Pengukuran Ke #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("Sensor_PZEM004T", msg);
  }
    client.loop();
    delay(5000); 
  }
}
//============================================================================================