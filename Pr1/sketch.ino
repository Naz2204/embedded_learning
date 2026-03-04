#define LED_POWER 22
#define MOTION_SENSOR_IN 15
#define TIME_BTN_1000 19
#define TIME_BTN_60000 18
#define TIME_BTN_90000 5
#define SENSOR_STATE_BTN 17

unsigned long previous_time = 0;
byte sensor_state = 1;
byte last_read_state = 1;
byte last_motion_state = 0;
int timeout = 90000;

void on_detected() {
  Serial.print("Detected. ");
  Serial.printf("Time %d \n", previous_time);
  digitalWrite(LED_POWER, HIGH);
}

void on_timeout() {
  Serial.print("Timeout. ");
  Serial.printf("Time %d \n", previous_time);
  digitalWrite(LED_POWER, LOW);
}

void sensor_state_change() {
  byte read_sensor_state = digitalRead(SENSOR_STATE_BTN);
  delay(50);
  if (!read_sensor_state && last_read_state != read_sensor_state) {
    sensor_state = !sensor_state;
    last_read_state = 0;
    Serial.print(sensor_state);
  }
  else if (last_read_state == 0 && read_sensor_state == 1) {
    last_read_state = 1;
  }
}

void read_sensor() {
  unsigned long current_time = millis();
  if (sensor_state) {
    byte motion_status = digitalRead(MOTION_SENSOR_IN);

    if (motion_status == 1 && last_motion_state == 0) {
      previous_time = current_time;
      last_motion_state = 1;
      on_detected();
    }
    else if (motion_status == 0 && last_motion_state == 1) {
      last_motion_state = 0;
    }
  }
  
  if (current_time - previous_time >= timeout) {
    previous_time = current_time;
    on_timeout();
  }
}

void setup() {
  pinMode(LED_POWER, OUTPUT);
  pinMode(MOTION_SENSOR_IN, INPUT);
  pinMode(TIME_BTN_1000, INPUT_PULLUP);
  pinMode(TIME_BTN_60000, INPUT_PULLUP);
  pinMode(TIME_BTN_90000, INPUT_PULLUP);
  pinMode(SENSOR_STATE_BTN, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  
  sensor_state_change();

  if (!digitalRead(TIME_BTN_1000)) {
    timeout = 1000;
    Serial.printf("New timeout %d \n", timeout);
  }

  if (!digitalRead(TIME_BTN_60000)) {
    timeout = 60000;
    Serial.printf("New timeout %d \n", timeout);
  }

  if (!digitalRead(TIME_BTN_90000)) {
    timeout = 10000;
    Serial.printf("New timeout %d \n", timeout);
  }

  read_sensor();
}