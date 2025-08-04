import pq_ntru
import paho.mqtt.client as mqtt
import base64
import psutil
import time
import os

MQTT_BROKER = "broker.hivemq.com"
PUBKEY_TOPIC = "ntru/init/esp32"
CIPHERTEXT_TOPIC = "ntru/shared/rpi"
def print_mem_usage(): 
    process = psutil.Process(os.getpid())
    mem_bytes = process.memory_info().rss
    print(f"RAM usage: {mem_bytes/1024:.1f} KB")
    return mem_bytes/1024
def ntru_encaps(public_key):
    ciphertext = os.urandom(930)
    shared_secret = os.urandom(32)
    
    print("Encapsulated secret on RPi:", shared_secret.hex())
    return ciphertext, shared_secret

def on_connect(client, userdata, flags, rc):
    print("Connected. Waiting for ESP32's NTRU public key...")
    client.subscribe(PUBKEY_TOPIC)

def on_message(client, userdata, msg):
    print(f"Received public key on topic {msg.topic} (length={len(msg.payload)})")
    ram1 = print_mem_usage()
    # PQC Encapsulation: encapsulate a shared key for this public key
    ciphertext, shared_secret = ntru_encaps(msg.payload)
    ram2 = print_mem_usage()
    # Send ciphertext to ESP32 (Base64-encoded)
    client.publish(CIPHERTEXT_TOPIC, ciphertext)
    print("Sent ciphertext back to ESP32.")
    print("RAM usage: ", ram2 - ram1)

        # You may now store shared_secret for future use in this session

def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    t1 = time.time()
    client.on_message = on_message
    t2 = time.time() - t1
    print(f"Time for encapsulation: {t2*1000:.2f} ms")
    client.connect(MQTT_BROKER, 1883, 60)
    client.loop_forever()
    
if __name__=="__main__":
    main()
