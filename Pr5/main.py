import json

import pandas as pd
import joblib
from pathlib import Path

import paho.mqtt.client as mqtt
from sklearn.linear_model import LinearRegression

regressor: LinearRegression | None = None

sensor_buffer = {
    "Sunlight_Hours": None,
    "Precipitation": None,
    "Average_Temperature": None,
    "Soil_pH": None,
    "Soil_Moisture": None,
    "CO2_Concentration": None
}

def load_csv_data():
    df = pd.read_csv('cotton_growth_cycle_data.csv')
    x = df[["Sunlight_Hours", "Precipitation", "Average_Temperature", "Soil_pH", "Soil_Moisture", "CO2_Concentration"]]
    y = df["Yield"]
    return x, y

def prep_train_data(x:pd.DataFrame, y:pd.DataFrame):
    from sklearn.model_selection import train_test_split

    x_train, x_test, y_train, y_test = train_test_split(x, y, test_size = 0.2, random_state = 42)
    return x_train, x_test, y_train, y_test

def train_model():
    global regressor
    x, y = load_csv_data()
    x_train, x_test, y_train, y_test = prep_train_data(x, y)
    regressor = LinearRegression(n_jobs=-1)
    regressor.fit(x_train, y_train)

    joblib.dump(regressor, 'model.pkl')


def load_model():
    global regressor
    regressor = joblib.load('model.pkl')

def mqtt_receiver():
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.on_message = on_message
    client.connect("broker.hivemq.com", 1883)
    client.subscribe("crop_weather/sensor/#")
    print("Receiver online")
    client.loop_forever()

def on_message(client, userdata, msg):
    payload = msg.payload.decode()

    if "status" in payload:
        print("Status:", payload)
        return

    key = msg.topic.split('/')[-1]
    val = float(payload)

    sensor_buffer[key] = val

    if all(v is not None for v in sensor_buffer.values()):
        df = pd.DataFrame([sensor_buffer])

        if regressor is not None:
            prediction = regressor.predict(df)
            print(df.to_string(index=False))
            print(f"Predicted Yield: {prediction[0]:.2f}")
        for k in sensor_buffer: sensor_buffer[k] = None


def main():
    model_file = Path("model.pkl")
    if model_file.is_file():
        load_model()
    else:
        train_model()

    mqtt_receiver()


if __name__ == '__main__':
    main()