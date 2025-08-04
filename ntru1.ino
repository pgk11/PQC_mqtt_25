#include <WiFi.h>
#include <PubSubClient.h>
#include "ntru.h"  // Simulated NTRU implementation

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqttServer = "broker.hivemq.com";  // or your local broker
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

uint8_t pk[NTRU_PUBLICKEYBYTES];
uint8_t sk[NTRU_SECRETKEYBYTES];
uint8_t shared_secret[NTRU_SHAREDKEYBYTES];

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n[WiFi] Failed to connect.");
  }
}

void connectMQTT() {
  Serial.print("[MQTT] Connecting");
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println(" connected.");
      client.subscribe("ntru/response/rpi");
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n[ESP32] Received ciphertext from Raspberry Pi:");

  for (int i = 0; i < length; i++) Serial.printf("%02X ", payload[i]);
  Serial.println();

  uint64_t start = micros();
  ntru_decapsulate(shared_secret, payload, sk);
  uint64_t end = micros();

  Serial.printf("[ESP32] Decapsulation time: %llu µs\n", end - start);

  Serial.print("[ESP32] Shared Secret: ");
  for (int i = 0; i < NTRU_SHAREDKEYBYTES; i++) Serial.printf("%02X", shared_secret[i]);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWiFi();

  client.setServer(mqttServer, mqttPort);
  client.setBufferSize(2048);
  client.setCallback(callback);
  connectMQTT();

  // Generate Keypair and measure time
  uint64_t start = micros();
  ntru_keypair(pk, sk);
  uint64_t end = micros();

  Serial.printf("[ESP32] NTRU Key Generation Time: %llu µs\n", end - start);

  // Publish Public Key
  bool success = client.publish("ntru/init/esp32", pk, NTRU_PUBLICKEYBYTES);
  if (success)
    Serial.println("[ESP32] Public key published successfully.");
  else
    Serial.println("[ESP32] Failed to publish public key.");
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();
}
