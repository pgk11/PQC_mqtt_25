#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_timer.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";
const char* topic_public_key = "kyber/init/esp32";
const char* topic_cipher = "kyber/response/pi";

#define KYBER_PUBLICKEYBYTES 800
#define KYBER_SECRETKEYBYTES 1632
#define KYBER_CIPHERTEXTBYTES 768
#define KYBER_SHARED_SECRET_BYTES 32

uint8_t public_key[KYBER_PUBLICKEYBYTES];
uint8_t private_key[KYBER_SECRETKEYBYTES];
uint8_t shared_secret[KYBER_SHARED_SECRET_BYTES];

WiFiClient espClient;
PubSubClient client(espClient);

static inline uint64_t now_us() {

  return (uint64_t)esp_timer_get_time();
}
static void print_duration(const char* label, uint64_t start_us, uint64_t end_us) {

  uint64_t dur = end_us - start_us;
  Serial.printf("%s: %llu us (%.3f ms)\n", label,
                (unsigned long long)dur, dur / 1000.0);
}

// Generating Kyber key pair
void generateKyberKeyPair() {
  uint64_t t1 = now_us();
  for (int i = 0; i < KYBER_PUBLICKEYBYTES; i++) {
    public_key[i] = random(0, 256);
  }
  uint64_t t2 = now_us();
  for (int i = 0; i < KYBER_SECRETKEYBYTES; i++) {
    private_key[i] = random(0, 256);
  }
  uint64_t t3 = now_us();
  Serial.println("Kyber keypair generated");
  print_duration("Public key generation time: ", t1, t2);
  Serial.println(" ");
  print_duration("Private key generation time: ", t2, t3);
}

// decrypting iphertext function 
void kyberDecapsulate(uint8_t* ciphertext) {
  uint64_t t1 = now_us();
  // Simulate shared key derivation
  for (int i = 0; i < KYBER_SHARED_SECRET_BYTES; i++) {
    shared_secret[i] = (ciphertext[i] ^ private_key[i]) & 0xFF;  // Mock logic
  }
  uint64_t t2 = now_us();
  Serial.println("Shared secret derived: ");
  for (int i = 0; i < KYBER_SHARED_SECRET_BYTES; i++) {
    Serial.printf("%02X", shared_secret[i]);
  }
  print_duration("Decapsulation time: ", t1, t2);
}

// Receiving secret key from python code on rpi
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("Message arrived on topic: %s, length: %u\n", topic, length);

  if (strcmp(topic, topic_cipher) == 0 && length == KYBER_CIPHERTEXTBYTES) {
    kyberDecapsulate(payload);
  } else {
    Serial.println("Unexpected topic or payload size.");
  }
}

//funtion to connect WiFi
void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected.");
}

//connect to mqtt server
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected.");
      client.subscribe(topic_cipher);  // Subscribe to receive ciphertext
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  setupWiFi();
  client.setServer(mqtt_server, 1883);
  client.setBufferSize(2048);
  client.setCallback(callback);

  generateKyberKeyPair();
  reconnectMQTT();
  Serial.print("Public key: ");
  for (int i = 0; i < KYBER_PUBLICKEYBYTES; i++) {
    Serial.print(public_key[i]);
  }
  Serial.println("\nPrivate key: ");
  for (int i = 0; i < KYBER_SECRETKEYBYTES; i++) {
    Serial.print(private_key[i]);
  }
 
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  bool ok = client.publish(topic_public_key, public_key, KYBER_PUBLICKEYBYTES, true);
Serial.printf("Publish result: %s (%d bytes)\n", ok ? "OK" : "FAIL", KYBER_PUBLICKEYBYTES);
  client.loop();
  
  
  delay(5000);
}
