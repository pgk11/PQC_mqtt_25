#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials (Wokwi simulates connection, these values don't matter)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";  // Public MQTT broker
const char* msg = "iot/pgk";  // Message to publish

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    while (!client.connected()) {
      Serial.print("Connecting to MQTT...");
      if (client.connect("ESP32Client")) {
        Serial.println("connected");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        delay(2000);
      }
    }
  }

  // Publish a test message every 5 seconds
  client.publish(msg, "Prashant Dabeli");
  Serial.println("Published: Aloo croissant chaat");
  delay(5000);
}


//JnU9NDM2MDEzMTE4MzM1ODEzNjMzJm49UHJpeWFuK0tvdHdhbCZlPXByaXlhbi5rb3R3YWwlNDBnbWFpbC5jb20meD0yMDI1MDgxNABzbpBOApq5BJYlUXrbxfXcvSo6VtQmAX3SrumEY8Ec0PT8b1KZtBVfX_SwSYW_P9zGVUfqGRKPdBx_P_P1BJYfWqN03zC2mlKd_PVVaIMIMs4uYWRQDMHtgqDZjBstOq9mohWK0Hw0BPWBZGEVLSROeIBLvPlUIL1CxV6iuKDM2ZAA_Sm1vrc4wbptYKAVTjaU5WPSCOQbu_PzulVN_P5UP2JCJXDZKQk9p9Jz0fWiBWRpmhiJgJRcLYKI5fXoRXPUeFa7OdCWbzUBOuMtMer_S80IlAABkaKViC632tDPExJQod4h1QoC_S3zDkfO9cPtYHrhrAUqA4AvMy7EUd6MOMMbf4khCV