#include <SD.h>
#include <TMRpcm.h>
#include <SPI.h>

#define STOP_COUNT 6
#define record_time 10000

#define SD_ChipSelectPin 53
#define echoPin 23
#define trigPin 10
#define threshold 910
#define record_pin 32
#define record_button 3
#define play_button 2
#define record_led 22
#define play_led 24

const int sound_module_pins[] = {34, 36, 38, 40, 42, 44, 46};
int sensorPins[12] = {A5, A4, A3, A2, A1, A0, A11, A10, A9, A8, A7, A6};
const int dir_pins[2] = {28, 30};
const int pwm_pins[2] = {8, 9};


int record_button_count = 0;
int station_count = 0;
unsigned long stopped_time;
unsigned long lastSwitchDetectedMillis_record, lastSwitchDetectedMillis_play;
long duration;
int distance;
int statusSensor[12] = {0};
bool reached_destination = false;
bool isRecording = false;
bool is_playing = false;

TMRpcm tmrpcm;

void open_circuit(int n) {
  pinMode (n, INPUT) ;
}

void make_low(int n) {
  pinMode (n, OUTPUT);
  digitalWrite (n, LOW) ;
}

void play_message(int n) {
  open_circuit(n);
  delay(1000);
  make_low(n);
  delay(1000);
  open_circuit(n);
  delay(record_time - 2000); // 2500
  make_low(n);
  delay(500);
  open_circuit(n);
}

void recording_audio_in_message(int message_pin) {
  Serial.print("Recording audio in message ");
  Serial.print(record_button_count);
  Serial.print(" \n");
  make_low(sound_module_pins[message_pin]);
}

void stop_recording_in_message(int message_pin) {
  Serial.print("Stoping recording in message ");
  Serial.print(record_button_count - 1);
  Serial.print(" \n");
  open_circuit(sound_module_pins[message_pin]);
}

void record_interupt() {
  if (millis() - lastSwitchDetectedMillis_record > 1000) {
    isRecording = true;
  }
  lastSwitchDetectedMillis_record = millis(); 
}

void play_interupt() {
  if (millis() - lastSwitchDetectedMillis_play > 1000) {
    is_playing = true;
  }
  lastSwitchDetectedMillis_play = millis(); 
}


void record() {
  if(!reached_destination && record_button_count < 7) {
    make_low(record_pin);
    recording_audio_in_message(record_button_count);
  }

  else if(record_button_count > 6) {
    Serial.print("Sorry! Can't record any more messages\n");
  }
  record_button_count++;
}

void stop_record() {
    make_low(record_pin);
    stop_recording_in_message(record_button_count - 1);
}

void playing() {
  digitalWrite(play_led, HIGH);
  open_circuit(record_pin);
  for (int i = 0; i < record_button_count; i++) {
    Serial.print("Playing recorded audio in message ");
    Serial.print(i);
    Serial.print(" \n");
    play_message(sound_module_pins[i]);
  }
  is_playing = false;
}


int check(int a[], int b[]) {
  int ar_count = 0;
  for (int i = 0; i < 12; i++) {
    if (a[i] == b[i]) ar_count++;
  }
  if (ar_count == 12) return 1;
  return 0;
}

void print_sensor_values() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print("\n");

  for (int i = 0; i < 12; i++) {
    Serial.print(statusSensor[i]);
    Serial.print(" ");
  }
  Serial.print(" \n");
}

int IR_status(int ar[]) {

  int stop[12]      =        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  int noPath[12]    =         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  int forward1[12]  =        {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0};
  int forward2[12]  =        {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0};
  int forward3[12]  =        {0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0};
  int forward4[12]  =        {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0};
  int forward5[12]  =        {0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0};
  int forward6[12]  =        {0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0};

  int right1[12]    =        {0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0};
  int right2[12]    =        {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
  int right3[12]    =        {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
  int right4[12]    =        {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0};
  int right5[12]    =        {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
  int right6[12]    =        {0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
  int right7[12]    =        {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0};

  int left1[12]     =        {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0};
  int left2[12]     =        {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0};
  int left3[12]     =        {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1};
  int left4[12]     =        {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0};
  int left5[12]     =        {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1};
  int left6[12]     =        {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0};
  int left7[12]     =        {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1};


  if (check(ar, stop)) return 0;

  if (check(ar, forward1)) return 1;
  if (check(ar, forward2)) return 1;
  if (check(ar, forward3)) return 1;
  if (check(ar, forward4)) return 1;
  if (check(ar, forward5)) return 1;
  if (check(ar, forward6)) return 1;

  if (check(ar, left1)) return 2;
  if (check(ar, left2)) return 2;
  if (check(ar, left3)) return 2;
  if (check(ar, left4)) return 2;
  if (check(ar, left5)) return 2;
  if (check(ar, left6)) return 2;
  if (check(ar, left7)) return 2;

  if (check(ar, right1)) return 3;
  if (check(ar, right2)) return 3;
  if (check(ar, right3)) return 3;
  if (check(ar, right4)) return 3;
  if (check(ar, right5)) return 3;
  if (check(ar, right6)) return 3;
  if (check(ar, right7)) return 3;

  if (check(ar, noPath)) return 4;

  return -1;
}


void setup()
{
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode (dir_pins[0], OUTPUT);
  pinMode (dir_pins[1], OUTPUT);
  pinMode(dir_pins[0], OUTPUT);
  pinMode(dir_pins[1], OUTPUT);
  pinMode(record_led, OUTPUT);
  pinMode(play_led, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(record_button), record_interupt, RISING);
  attachInterrupt(digitalPinToInterrupt(play_button), play_interupt, RISING);
  for (int i = 0; i < 12; i++) pinMode(sensorPins[i], INPUT);
  tmrpcm.speakerPin = 11;
  if(!SD.begin(SD_ChipSelectPin)) {
    Serial.println("SD fail");
    return;
  }
  else Serial.println("SD card initialized");
}

void loop()
{
  for (int i = 0; i < 12; i++) statusSensor[i] = (analogRead(sensorPins[i]) >= threshold);
  print_sensor_values();

  if (distance >= 20) {
    if (IR_status(statusSensor) == 0) {
      station_count++;
      if (station_count < STOP_COUNT) {
        digitalWrite(dir_pins[0], LOW);
        analogWrite(pwm_pins[0], 0);
        digitalWrite(dir_pins[1], LOW);
        analogWrite(pwm_pins[1], 0);
        Serial.print("Stoping here for the next ");
        Serial.print("15");
        Serial.print(" seconds\n");
        Serial.print("If you want some thing, then press the record button\n");
        // if(stopFlag) stopped_time = millis();
        isRecording = false;
        tmrpcm.play("stop_audio.wav");
        stopped_time = millis();
        while(millis() - stopped_time < 10000 && !isRecording);
        tmrpcm.disable();
        if(isRecording) {
          digitalWrite(record_led, HIGH);
          record();
          delay(10000); 
          isRecording = false;
          digitalWrite(record_led, LOW);
          stop_record();
        }
        //else delay(15000);
        digitalWrite(dir_pins[0], LOW);
        analogWrite(pwm_pins[0], 80);
        digitalWrite(dir_pins[1], LOW);
        analogWrite(pwm_pins[1], 80);
        delay(1000);
        //stopFlag = true;
        isRecording = false;
      }

      else {
        digitalWrite(dir_pins[0], LOW);
        analogWrite(pwm_pins[0], 0);
        digitalWrite(dir_pins[1], LOW);
        analogWrite(pwm_pins[1], 0);
        is_playing = false;
        Serial.print("Reached Destination\n");
        //tmrpcm.play("End_audio.wav");
        while(!is_playing){
          Serial.print("Waiting for play interupt\n");
        }
        //tmrpcm.disable();
        if(is_playing) playing();    
    }
  }
    else if (IR_status(statusSensor) == 1) {
      digitalWrite(dir_pins[0], LOW);
      analogWrite(pwm_pins[0], 160);//220
      digitalWrite(dir_pins[1], LOW);
      analogWrite(pwm_pins[1], 160);
      Serial.print("Moving Forward\n");
    }

    else if (IR_status(statusSensor) == 2) {
      digitalWrite(dir_pins[0], LOW);
      analogWrite(pwm_pins[0], 100);//150
      digitalWrite(dir_pins[1], LOW);
      analogWrite(pwm_pins[1], 0);
      Serial.print("Moving Left\n");
    }

    else if (IR_status(statusSensor) == 3) {
      digitalWrite(dir_pins[0], LOW);
      analogWrite(pwm_pins[0], 0);
      digitalWrite(dir_pins[1], LOW);
      analogWrite(pwm_pins[1], 100);
      Serial.print("Moving Right\n");
    }

    else if (IR_status(statusSensor) == 4) {
      digitalWrite(dir_pins[0], LOW);
      analogWrite(pwm_pins[0], 0);
      digitalWrite(dir_pins[1], LOW);
      analogWrite(pwm_pins[1], 0);
      Serial.print("Out of Track\n");
    }
    else {
      digitalWrite(dir_pins[0], LOW);
      analogWrite(pwm_pins[0], 80);//200
      digitalWrite(dir_pins[1], LOW);
      analogWrite(pwm_pins[1], 80);
      Serial.print("Invalid path\n");
      tmrpcm.play("invalid_path_audio.wav");
    }
  }

  else {
    Serial.print("Obstacle detected\n");
    digitalWrite(dir_pins[0], LOW);
    analogWrite(pwm_pins[0], 0);
    digitalWrite(dir_pins[1], LOW);
    analogWrite(pwm_pins[1], 0);
    tmrpcm.play("obstacle_detected_audio.wav");
  }
}
