/*
 Basic MQTT ESP32
=====================================================================================
Koneksi MQTT ke ESP32
===================================================================================== 
*/
#include <WiFi.h>
#include <PubSubClient.h>

//Definisi WIFI
//#define WIFI_NETWORK "aufarquinsa"
//#define WIFI_PASSWORD "FAR291296"
#define WIFI_NETWORK "OPPO A5 2020"
#define WIFI_PASSWORD "796cad6039d3"
#define WIFI_TIMEOUT_MS 20000

const char* mqtt_server = "rmq2.pptik.id";
const char* mqtt_user = "TMDG2022";
const char* mqtt_pass = "TMDG2022";

WiFiClient espClient;
PubSubClient client(espClient);

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//============================================================================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPClient",mqtt_user,mqtt_pass)) {
      Serial.print("connected to ");
      Serial.println(mqtt_server);
      
    } else {
     
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  ConnectToWIFI();
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);

  //Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
