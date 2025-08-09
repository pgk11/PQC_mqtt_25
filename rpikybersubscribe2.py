#! usr/bin/env python3
import paho.mqtt.client as mqtt
import os
import pqcrypto.kem.ml_kem_512 as kyber
import time
import psutil

# MQTT broker settings
BROKER = "broker.hivemq.com"
TOPIC_PUBKEY = "kyber/init/pgk1"
TOPIC_CIPHERTEXT = "kyber/response/pgk1"
def print_mem_usage():
    process = psutil.Process(os.getpid())
    mem_bytes = process.memory_info().rss
    print(f"RAM usage: {mem_bytes/1024:.1f} KB")
    return mem_bytes/1024
def secret_key_generate():
    secret_key = os.urandom(1632)
    return secret_key
def kyber_encapsulate(public_key):
    alg = 'ML-KEM-512'
    ram1 = print_mem_usage()
    print("Encapsulating the key")
    #ciphertext = os.urandom(768)        
    #shared_secret = os.urandom(32)
    t1 =  time.perf_counter_ns()    
    ciphertext, shared_secret = kyber.encrypt(public_key)
    t2 =  time.perf_counter_ns()
    ram2 = print_mem_usage()
    print("Shared key generated.")
    time_enc = (t2 - t1) / 1000000.0
    print("Time for encapsulation: ", time_enc)
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
    secret_key = secret_key_generate()
    print(len(shared_secret))
    shared_secret_new = kyber.decrypt(secret_key, ciphertext)
    client.publish(TOPIC_CIPHERTEXT, shared_secret_new)
    print(f"Sent ciphertext to topic: {TOPIC_CIPHERTEXT}")
    print(len(shared_secret_new))
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