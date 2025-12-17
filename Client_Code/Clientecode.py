import ssl
import time
import json
import paho.mqtt.client as mqtt
import mysql.connector

# -----------------------------
# 1. AWS IoT (MQTT) SETTINGS
# -----------------------------

# Your AWS IoT endpoint (from AWS IoT Core → Settings)
AWS_ENDPOINT = "a27hbj2v8bh0z3-ats.iot.us-east-1.amazonaws.com"

# Name for this MQTT client (can be the Thing name)
CLIENT_ID = "serverClient01"

# Topic where the ESP32 publishes card data
TOPIC = "iticbcn/espnode01/pub"

# Paths to certificate files on the EC2 instance
CA_PATH = "certs/AmazonRootCA1.pem"   # Root CA from AWS
CERT_PATH = "certs/device-cert.pem"   # Certificate for this client
KEY_PATH = "certs/private-key.pem"    # Private key for this client


# -----------------------------
# 2. DATABASE SETTINGS (MariaDB)
# -----------------------------

DB_CONFIG = {
    "host": "localhost",
    "user": "assist_admin",        # DB user created for the project
    "password": "Assistencia123!", # DB password
    "database": "control_assistencia",
}

# ID of the device (reader) in table 'dispositiu'
DEVICE_ID = 1

# Global DB connection object (reused)
db_conn = None


def connect_db():
    """
    Open a connection to the database (or reuse it if already open).
    """
    global db_conn
    if db_conn is None or not db_conn.is_connected():
        db_conn = mysql.connector.connect(**DB_CONFIG)
    return db_conn


# -----------------------------
# 3. MQTT CALLBACK FUNCTIONS
# -----------------------------

def on_connect(client, userdata, flags, rc):
    """
    Called when the client connects to AWS IoT.
    """
    print("Connected with result code", rc)
    # Subscribe to the topic after connecting
    client.subscribe(TOPIC)
    print(f"Subscribed to topic: {TOPIC}")


def on_message(client, userdata, msg):
    """
    Called when a new MQTT message is received from AWS IoT.
    Here we:
      1. Parse the JSON payload
      2. Find the user with this card UID
      3. Insert a row into 'fixatge' table
    """
    global db_conn

    # Decode the bytes payload to string
    payload = msg.payload.decode("utf-8")
    print(f"Message received on topic {msg.topic}: {payload}")

    try:
        # Convert JSON string to Python dict
        data = json.loads(payload)

        # Card UID and device name from the message
        uid = data.get("uid")
        device_name = data.get("device")  # not used now, but could be used later

        if not uid:
            print("No UID in message, skipping")
            return

        # Make sure we have a DB connection
        conn = connect_db()
        cursor = conn.cursor()

        # 1) Find user ID by card UID in table 'usuari'
        cursor.execute(
            "SELECT id FROM usuari WHERE targeta = %s",
            (uid,),
        )
        row = cursor.fetchone()

        if row is None:
            # No user with this card UID
            print(f"No user found for card UID {uid}, skipping insert")
            return

        id_usuari = row[0]

        # 2) Insert clock-in event into 'fixatge'
        #    Use current date and time from the DB server
        insert_sql = """
            INSERT INTO fixatge (id_usuari, id_dispositiu, data, hora)
            VALUES (%s, %s, CURDATE(), CURTIME())
        """
        cursor.execute(insert_sql, (id_usuari, DEVICE_ID))
        conn.commit()

        print(f"Inserted fixatge: user_id={id_usuari}, device_id={DEVICE_ID}")

    except json.JSONDecodeError:
        print("Error decoding JSON, skipping message")
    except mysql.connector.Error as e:
        # Handle database errors (connection lost, etc.)
        print(f"MySQL error: {e}")
        db_conn = None  # Force reconnect next time
    except Exception as e:
        # Catch any other unexpected errors
        print(f"Unexpected error: {e}")


# -----------------------------
# 4. MAIN FUNCTION
# -----------------------------

def main():
    # Create MQTT client with the given client ID
    client = mqtt.Client(client_id=CLIENT_ID)

    # Attach callback functions
    client.on_connect = on_connect
    client.on_message = on_message

    # Configure TLS (SSL) with our certificates
    client.tls_set(
        ca_certs=CA_PATH,
        certfile=CERT_PATH,
        keyfile=KEY_PATH,
        tls_version=ssl.PROTOCOL_TLS,
    )

    print("Connecting to AWS IoT...")
    # Port 8883 is the default for MQTT over TLS
    client.connect(AWS_ENDPOINT, port=8883)

    # Start the network loop in a background thread
    client.loop_start()

    try:
        # Keep the script running forever
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        # Allow manual stop with Ctrl+C
        print("Exiting...")
    finally:
        # Clean shutdown
        client.loop_stop()
        client.disconnect()


# -----------------------------
# 5. ENTRY POINT
# -----------------------------

if __name__ == "__main__":
    main()

