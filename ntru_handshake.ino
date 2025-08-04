#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_timer.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";
const char* topic_pub = "ntru/init/esp32";
const char* topic_sub = "ntru/shared/rpi";

WiFiClient espClient;
PubSubClient client(espClient);

// === NTRU-HPS2048677 sizes ===
#define PK_SIZE 930
#define SK_SIZE 1234
#define CT_SIZE 930
#define SS_SIZE 32

uint8_t public_key[PK_SIZE];
uint8_t secret_key[SK_SIZE];
uint8_t received_cipher[CT_SIZE];
uint8_t shared_secret[SS_SIZE];
bool cipher_received = false;

static inline uint64_t now_us() {

  return (uint64_t)esp_timer_get_time();
}
static void print_duration(const char* label, uint64_t start_us, uint64_t end_us) {

  uint64_t dur = end_us - start_us;
  Serial.printf("%s: %llu us (%.3f ms)\n", label,
                (unsigned long long)dur, dur / 1000.0);
}

// === Placeholder for keypair generation ===
void ntru_keypair() {
  for (int i = 0; i < PK_SIZE; i++) {
    public_key[i] = esp_random() & 0xFF;
  }
  for (int i = 0; i < SK_SIZE; i++) {
    secret_key[i] = esp_random() & 0xFF;
  }
  Serial.println("[NTRU] Simulated keypair generated.");
}

// === Placeholder for decapsulation ===
void ntru_decaps(uint8_t* ss, const uint8_t* ct, const uint8_t* sk) {
  for (int i = 0; i < SS_SIZE; i++) ss[i] = ct[i] ^ sk[i];
  Serial.println("[NTRU] Simulated decapsulation complete.");
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  Serial.print("[WiFi] Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected.");
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, topic_sub) == 0 && length == CT_SIZE) {
    memcpy(received_cipher, payload, CT_SIZE);
    cipher_received = true;
    Serial.println("[MQTT] Ciphertext received.");
  } else {
    Serial.printf("[MQTT] Message received on topic '%s', length %u\n", topic, length);
  }
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("[MQTT] Connecting...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected.");
      client.subscribe(topic_sub);
    } else {
      Serial.print(" failed, rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setBufferSize(2048);
  client.setCallback(callback);

  reconnect_mqtt();

  // Generate keypair
  uint64_t t1 = now_us();
  ntru_keypair();
  uint64_t t2 = now_us();
  print_duration("Key generation time: ", t1, t2);
  Serial.println("Public key: ");
  for (int i = 0; i < PK_SIZE; i++) {
    Serial.print(public_key[i]);
  }
  Serial.println("\nPrivate key: ");
  for (int i = 0; i < SK_SIZE; i++) {
    Serial.print(secret_key[i]);
  }
  // Publish public key
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }

  bool success = client.publish(topic_pub, public_key, PK_SIZE);
  Serial.println(success ? "[MQTT] Public key published." : "[MQTT] Publish failed.");
  client.loop();

  if (cipher_received) {
    cipher_received = false;

    ntru_decaps(shared_secret, received_cipher, secret_key);

    Serial.print("[NTRU] Shared secret: ");
    for (int i = 0; i < SS_SIZE; i++) {
      Serial.printf("%02X", shared_secret[i]);
    }
    Serial.println();
  }

  delay(3500);
}
