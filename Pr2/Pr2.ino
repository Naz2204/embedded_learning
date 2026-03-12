#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include "DHT.h"

#define WIFI_SSID "your_wifi"
#define WIFI_PASSWORD "your_pass"
#define WIFI_CHANNEL 6

#define MOTOR_CON1 4
#define MOTOR_CON2 15
#define DHT_DATA 32
#define DHT_TYPE DHT11

WebServer server(80);
DHT dht(DHT_DATA, DHT_TYPE);

const int freq = 500;
const int resolution = 8;

float temp_limit = 20;
float hum_limit = 45;


void sendHtml() {
    String response = R"(
    <!DOCTYPE html>
    <html>
      <head>
        <title>Variant 16</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
          body{
            font-family: Arial;
            text-align: center;
            background: url(https://images.steamusercontent.com/ugc/1689398119794154468/147682ED4598124EF81740BF07DDD34DE2B62781/?imw=637&imh=358&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true);
            background-repeat: no-repeat;
            background-size: cover;
            margin-top:40px;
          }

          .card{
            background:white;
            padding:25px;
            margin:auto;
            width:300px;
            border-radius:10px;
            box-shadow:0 0 10px rgba(0,0,0,0.2);
          }

          .value{
            font-size:35px;
            margin:10px;
          }

          input{
            width:80px;
            padding:5px;
            font-size:18px;
            text-align:center;
          }

          button{
            padding:10px 20px;
            font-size:16px;
            margin-top:10px;
          }
        </style>
      </head>

      <body>

        <h1>Variant 16</h1>

        <div class="card">
          <h3>Current temperature</h3>
          <div class="value" id="temp"></div>

          <h3>Current humidity</h3>
          <div class="value" id="hum"></div>
        </div>

        <br>

        <div class="card">
          <h3>Max allowed values</h3>

          <form id="desiredForm">

            <label>Temperature</label><br>
            <input type="number" name="temp" id="targetTemp" placeholder=%temp_limit% step="0.1"><br><br>

            <label>Humidity</label><br>
            <input type="number" name="hum" id="targetHum" placeholder=%hum_limit% step="0.1"><br><br>

            <button type="submit">Save</button>

          </form>

          <div id="status" style="margin-top:10px;color:green;"></div>
        </div>

        <script>
          function fetchSensorData() {
            fetch('/data')
              .then(response => response.json())
              .then(data => {
                document.getElementById('temp').innerText = data.temp.toFixed(1) + ' °C';
                document.getElementById('hum').innerText = data.hum.toFixed(1) + ' %';
              })
              .catch(err => console.log('Error fetching data', err));
          }

          setInterval(fetchSensorData, 2000);
          fetchSensorData();

          document.getElementById('desiredForm').addEventListener('submit', function(e){
            e.preventDefault();
            let temp = parseFloat(document.getElementById('targetTemp').value);
            let hum = parseFloat(document.getElementById('targetHum').value);

            fetch(`/set?temp=${temp}&hum=${hum}`)
              .then(resp => resp.text())
              .then(msg => {
                document.getElementById('status').innerText = 'Saved!';
                setTimeout(()=>document.getElementById('status').innerText='', 2000);
              })
              .catch(err => console.log('Error sending desired values', err));
          });
        </script>

      </body>
    </html>
  )";

  response.replace("%temp_limit%", String(temp_limit));
  response.replace("%hum_limit%", String(hum_limit));

  server.send(200, "text/html", response);
}

void change_speed(int dutyCycle) {
  ledcWrite(MOTOR_CON1, dutyCycle);
  ledcWrite(MOTOR_CON2, 0);
}

void calc_fan_speed(float tc, float hu) {
  if (temp_limit*2 < tc || hum_limit*1.5 < hu) {
    change_speed(255);
  }
  else if (temp_limit < tc || hum_limit < hu) {
    change_speed(255 / 4);
  }
  else {
    change_speed(0);
  }
}

void data_path() {
  server.on("/data", [](){
    float tc = dht.readTemperature();
    float hu = dht.readHumidity(); 

    Serial.printf("Temperature: %.2f C\n", tc);
    Serial.printf("Humidity: %.2f %\n", hu);

    calc_fan_speed(tc, hu);

    String json = "{\"temp\":"+String(tc,1)+",\"hum\":"+String(hu,1)+"}";
    server.send(200, "application/json", json);
  });
}

void set_path() {
  server.on("/set", [](){
    float tc = server.arg("temp").toFloat();
    float hu = server.arg("hum").toFloat();

    if (tc == tc) {
      temp_limit = tc;
    }

    if (hu == hu) {
      hum_limit = hu;
    }

    Serial.printf("Target temperature: %.2f C\n", temp_limit);
    Serial.printf("Target humidity: %.2f %\n", hum_limit);

    server.send(200, "text/plain", "OK");
  });
}

void connect_to_WiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {

  Serial.begin(9600);

  ledcAttach(MOTOR_CON1, freq, resolution);
  ledcAttach(MOTOR_CON2, freq, resolution);

  connect_to_WiFi();

  server.on("/", sendHtml);

  data_path();

  set_path();

  dht.begin();

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(2);
}
