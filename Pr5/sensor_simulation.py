import paho.mqtt.client as mqtt
import time
import random

BROKER = "broker.hivemq.com"

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.connect(BROKER, 1883)

try:
    while True:
        client.publish("crop_weather/sensor/Sunlight_Hours", round(random.uniform(3.5, 9.5), 2))
        client.publish("crop_weather/sensor/Precipitation", random.randint(300, 600))
        client.publish("crop_weather/sensor/Average_Temperature", random.randint(10, 40))
        client.publish("crop_weather/sensor/Soil_pH", round(random.uniform(5.5, 7.5), 2))
        client.publish("crop_weather/sensor/Soil_Moisture", random.randint(0, 100))
        client.publish("crop_weather/sensor/CO2_Concentration", random.randint(400, 500))

        print("Data packet sent to broker.")
        time.sleep(15)
except KeyboardInterrupt:
    print("Stopped.")