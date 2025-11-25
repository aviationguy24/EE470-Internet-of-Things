# ============================================================================
# File        : relay_node_mqtt_db.py
# Author      : Aaron John Estrada
# Date        : November 24, 2025
# System      : Relay Node (Windows / Laptop / Raspberry Pi)
#
# Description :
#   Relay node that:
#     1. Subscribes to an ESP8266 MQTT topic on the public HiveMQ broker
#     2. Receives potentiometer values and other sensor data
#     3. Inserts received values into a Hostinger MySQL database table
#     4. Prints all received MQTT messages for debugging purposes
#
# ============================================================================

import paho.mqtt.client as mqtt
import mysql.connector


# MQTT SETTINGS
BROKER_URL = "broker.hivemq.com"
BROKER_PORT = 1883
TOPIC = "testtopic/temp/outTopic/estrada123"  

# HOSTINGER DATABASE SETTINGS
DB_HOST = "151.106.97.51"                 # Hostinger database host
DB_USER = "u515418176_dbAaronEstrada"     
DB_PASS = "Mechanical2003!"               
DB_NAME = "u515418176_AaronEstrada"      

# FUNCTION TO INSERT INTO DATABASE
def push_value_to_db(sensor_value):
    try:
        conn = mysql.connector.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASS,
            database=DB_NAME
        )
        cur = conn.cursor()
        sql = "INSERT INTO pot_values (value) VALUES (%s)"
        cur.execute(sql, (sensor_value,))
        conn.commit()
        print(f"[DB] Inserted value: {sensor_value}")
    except mysql.connector.Error as err:
        print("[DB ERROR]", err)
    finally:
        try:
            cur.close()
            conn.close()
        except:
            pass



# MQTT CALLBACKS
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[MQTT] Connected to HiveMQ!")
        client.subscribe(TOPIC)
        print("[MQTT] Subscribed to:", TOPIC)
    else:
        print("[MQTT] Connection failed. RC:", rc)


def on_message(client, userdata, msg):
    payload = msg.payload.decode().strip()
    print("[MQTT] Received:", payload)

    try:
        sensor_value = float(payload)
        push_value_to_db(sensor_value)
    except ValueError:
        print("[WARN] Payload not numeric → Skipped")

# MAIN PROGRAM
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

print("Connecting to HiveMQ broker…")
client.connect(BROKER_URL, BROKER_PORT, 60)

print("Relay node running → press CTRL+C to stop")
client.loop_forever()
