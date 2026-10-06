# ESP32 Heart Sound Monitor & Visualizer

This project streams live heart sounds from an ESP32 to a web browser using USB Serial. It visualizes the audio waveform, detects S1 (Lub) and S2 (Dub) sounds, calculates real-time BPM, animates a beating heart, and plays the live audio through your computer's speakers.

## Features
* **Real-time Audio Streaming:** 4000Hz audio streamed over USB at 115200 baud.
* **Web Serial API:** No software installation required; runs directly in Chrome or Edge.
* **S1 / S2 Detection:** Algorithm analyzes amplitude peaks to detect Lub/Dub phases.
* **Live Visualization:** Oscilloscope-style waveform drawn on an HTML5 canvas.
* **Smart Heart Animation:** A CSS heart that pulses differently for S1 vs S2 beats.
* **Bluetooth Output:** BPM and S1/S2 heartbeat logs are also sent over Bluetooth Classic.

## How to Use
1. Flash the `ESP32_Audio_Sender.ino` code to your ESP32.
2. Connect your ESP32 to your PC via USB.
3. Open `index.html` in **Google Chrome** or **Microsoft Edge**.
4. Click **Connect ESP32** and select your COM port.

*Note: Ensure the Arduino IDE Serial Monitor is closed before connecting the web app, as only one program can access the COM port at a time.*
