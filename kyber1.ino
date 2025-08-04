#include <WiFi.h>
#include <PubSubClient.h>

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

// Generating Kyber key pair
void generateKyberKeyPair() {
  for (int i = 0; i < KYBER_PUBLICKEYBYTES; i++) {
    public_key[i] = random(0, 256);
  }
  for (int i = 0; i < KYBER_SECRETKEYBYTES; i++) {
    private_key[i] = random(0, 256);
  }
  Serial.println("Kyber keypair generated (simulated).");
}

// decrypting iphertext function 
void kyberDecapsulate(uint8_t* ciphertext) {
  // Simulate shared key derivation
  for (int i = 0; i < KYBER_SHARED_SECRET_BYTES; i++) {
    shared_secret[i] = (ciphertext[i] ^ private_key[i]) & 0xFF;  // Mock logic
  }
  Serial.print("Shared secret derived: ");
  for (int i = 0; i < KYBER_SHARED_SECRET_BYTES; i++) {
    Serial.printf("%02X", shared_secret[i]);
  }
  Serial.println();
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
  bool ok = client.publish(topic_public_key, public_key, KYBER_PUBLICKEYBYTES);
Serial.printf("Publish result: %s (%d bytes)\n", ok ? "OK" : "FAIL", KYBER_PUBLICKEYBYTES);
  client.loop();
  
  
  delay(5000);
}
