#! usr/bin/env python
import paho.mqtt.client as mqtt
import os
import time

# MQTT broker settings
BROKER = "broker.hivemq.com"
TOPIC_PUBKEY = "kyber/init/esp"
TOPIC_CIPHERTEXT = "kyber/response/pgk"

def kyber_encapsulate(public_key):
    print("Encapsulating the key")
    ciphertext = os.urandom(768)         # Simulated ciphertext
    shared_secret = os.urandom(32)       # Simulated shared key
    print("Shared key generated.")
    return ciphertext, shared_secret

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker")
    client.subscribe(TOPIC_PUBKEY)
    print(f"Subscribed to topic: {TOPIC_PUBKEY}")

def on_message(client, userdata, msg):
    print(f"Received public key on topic {msg.topic} (length={len(msg.payload)})")
    if len(msg.payload) == 800:
    	print("Public key received")
    	ciphertext, shared_secret = kyber_encapsulate(msg.payload)
    	print(ciphertext)
    	client.publish(TOPIC_CIPHERTEXT, ciphertext)
    	print(f"Sent ciphertext to topic: {TOPIC_CIPHERTEXT}")
    else:
    	print("unexpected key length")

def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    print("Connected! Waiting for message")
    client.on_message = on_message
    time.sleep(3)
    print("Received!")
    client.connect(BROKER, 1883, 60)
    client.loop_forever()

if __name__ == "__main__":
    main()
