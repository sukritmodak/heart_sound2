#include <Arduino.h>
#include "BluetoothSerial.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BluetoothSerial SerialBT;

// =========================
// PINS
// =========================
#define ANALOG_PIN 34
#define DIGITAL_PIN 27
#define BEAT_STATE LOW

// =========================
// HEART-SOUND TIMING
// =========================
#define MIN_SOUND_GAP 150
#define MIN_S1_S2_GAP 100
#define MAX_S1_S2_GAP 500
#define BPM_WINDOW 5000

// =========================
// USB AUDIO
// =========================
#define SAMPLE_RATE 4000
const unsigned long SAMPLE_INTERVAL = 1000000UL / SAMPLE_RATE;
unsigned long nextSampleTime = 0;

// =========================
// DIGITAL S1/S2 MONITOR
// =========================
int previousDigitalState = HIGH;
unsigned long lastSoundTime = 0;
unsigned long s1Time = 0;
bool waitingForS2 = false;

unsigned long totalSoundNumber = 0;
int soundCount5sec = 0;
int heartBeatCount5sec = 0;
unsigned long windowStartTime = 0;

// =========================
// BLE TERMINAL
// =========================
#define BLE_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define BLE_TX_UUID      "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

BLECharacteristic *bleTx = nullptr;

void sendBLEText(const String &message) {
  if (bleTx == nullptr) return;

  // Default BLE MTU safely supports about 20 data bytes.
  const int chunkSize = 20;
  for (int start = 0; start < message.length(); start += chunkSize) {
    String chunk = message.substring(start, start + chunkSize);
    bleTx->setValue((uint8_t*)chunk.c_str(), chunk.length());
    bleTx->notify();
    delay(3);
  }

  // Send newline separately so the browser terminal can reconstruct lines.
  const uint8_t newline = '\n';
  bleTx->setValue(&newline, 1);
  bleTx->notify();
  delay(3);
}

// Send the same diagnostic message to the existing Bluetooth Serial Terminal
// and to the web application's BLE terminal.
void BT(const String &message) {
  SerialBT.println(message);
  sendBLEText(message);
}

// =========================
// AUDIO PACKET FORMAT
// A5 5A | 128 | 128 samples | checksum
// Total = 132 bytes
// =========================
#define AUDIO_BLOCK 128
uint8_t audioBuffer[AUDIO_BLOCK];
int audioIndex = 0;

void sendAudioBlock() {
  uint8_t checksum = 0;

  Serial.write(0xA5);
  Serial.write(0x5A);
  Serial.write((uint8_t)AUDIO_BLOCK);

  for (int i = 0; i < AUDIO_BLOCK; i++) {
    Serial.write(audioBuffer[i]);
    checksum += audioBuffer[i];
  }

  Serial.write(checksum);
}

// =========================
// SETUP
// =========================
void setup() {
  Serial.begin(115200);

  // Existing Classic Bluetooth terminal remains available.
  SerialBT.begin("ESP32-HEART");

  // BLE terminal for the web application.
  BLEDevice::init("ESP32-HEART-BLE");

  BLEServer *bleServer = BLEDevice::createServer();
  BLEService *bleService = bleServer->createService(BLE_SERVICE_UUID);

  bleTx = bleService->createCharacteristic(
    BLE_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  bleTx->addDescriptor(new BLE2902());
  bleService->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  pinMode(ANALOG_PIN, INPUT);
  pinMode(DIGITAL_PIN, INPUT);

  windowStartTime = millis();
  nextSampleTime = micros();

  BT("================================");
  BT("ESP32 HEART SOUND MONITOR");
  BT("================================");
  BT("USB audio: 4000 Hz / 128 samples");
  BT("Browser BPM: successive S1-to-S1");
  BT("S1 + S2 = one cardiac cycle");
  BT("Classic BT: ESP32-HEART");
  BT("Web BLE: ESP32-HEART-BLE");
  BT("================================");
}

// =========================
// MAIN LOOP
// =========================
void loop() {
  unsigned long now = millis();

  // Digital heart-sound event monitoring.
  int digitalState = digitalRead(DIGITAL_PIN);

  if (previousDigitalState == HIGH && digitalState == BEAT_STATE) {
    unsigned long soundTime = millis();

    if (lastSoundTime == 0 || soundTime - lastSoundTime >= MIN_SOUND_GAP) {
      totalSoundNumber++;
      soundCount5sec++;

      if (!waitingForS2) {
        s1Time = soundTime;
        waitingForS2 = true;

        BT("S1 detected - Sound #" + String(totalSoundNumber));
      }
      else {
        unsigned long gap = soundTime - s1Time;

        if (gap >= MIN_S1_S2_GAP && gap <= MAX_S1_S2_GAP) {
          heartBeatCount5sec++;

          BT("S2 detected - S1-S2 gap = " + String(gap) + " ms");

          waitingForS2 = false;
        }
        else if (gap > MAX_S1_S2_GAP) {
          // S2 was missed. Treat this as a new S1.
          s1Time = soundTime;
          waitingForS2 = true;

          BT("S2 missed - new S1 detected");
        }
        else {
          BT("Ignored short duplicate sound");
        }
      }

      lastSoundTime = soundTime;
    }
  }

  previousDigitalState = digitalState;

  // Five-second diagnostic summary.
  if (now - windowStartTime >= BPM_WINDOW) {
    float bpm = heartBeatCount5sec * 12.0;

    BT("");
    BT("========== 5 SECOND RESULT ==========");
    BT("Total sounds = " + String(soundCount5sec));
    BT("S1-S2 pairs = " + String(heartBeatCount5sec));
    BT("BPM (pair count) = " + String(bpm, 1));
    BT("Browser BPM uses S1-to-S1 timing");
    BT("=====================================");
    BT("");

    soundCount5sec = 0;
    heartBeatCount5sec = 0;
    windowStartTime = now;
  }

  // USB audio stream. DO NOT put text on Serial:
  // the browser expects a continuous 132-byte binary packet format.
  unsigned long currentMicros = micros();

  if ((long)(currentMicros - nextSampleTime) >= 0) {
    nextSampleTime += SAMPLE_INTERVAL;

    uint8_t sample = (uint8_t)(analogRead(ANALOG_PIN) >> 4);
    audioBuffer[audioIndex++] = sample;

    if (audioIndex >= AUDIO_BLOCK) {
      sendAudioBlock();
      audioIndex = 0;
    }
  }
}
