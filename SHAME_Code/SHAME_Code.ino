/*
SHA.ME : Smart Home Assistant for Monitoring Energy
=================================================================
Platform Smart Home yang digunakan untuk monitoring dan efisiensi 
pemakaian energi suatu rumah

Tubes Desain Aplikasi Interaktif 2022
Kelompok : Kendali-2
           Joni Juliansah
           Muhammad Fadlan

Refference : Jakub Mandula 2021
-----------------------------------------------------------------
*/

#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <PubSubClient.h>

//Definisi WIFI
#define WIFI_NETWORK "aufarquinsa"
#define WIFI_PASSWORD "FAR291296"
#define WIFI_TIMEOUT_MS 20000

const char* mqtt_server = "rmq2.pptik.id";
const char* mqtt_user = "TMDG2022";
const char* mqtt_pass = "TMDG2022";
const char* mqtt_pub_topic = "Sensor";


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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  //printMACAddress();
  const char* CL;
  //CL = MACAddress.c_str();
  //Serial.println(CL);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(CL, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      ESP.restart();
      delay(5000);

    }
  }
}

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

void setup() {
    // Debugging Serial port
    Serial.begin(115200);
    ConnectToWIFI();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    // Reset Energy Internal Counter
    //pzem.resetEnergy();
}

void loop() {
    // Print the custom address of the PZEM
    Serial.print("Custom Address:");
    Serial.println(pzem.readAddress(), HEX);

    // Read the data from the sensor
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

if (!client.connected()) {
    reconnect();
  }
  client.loop();

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
    } else {

        // Print the values to the Serial console
        Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
        Serial.print("PF: ");           Serial.println(pf);

    }


    Serial.println();
    delay(2000);
}
