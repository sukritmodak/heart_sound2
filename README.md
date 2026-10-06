# ESP32 Heart Sound Monitor

Real-time heart sound monitoring using ESP32, Web Serial, 4000 Hz audio sampling, S1/S2 detection, waveform visualization and browser BPM estimation.

## Repository structure

```
heart_sound2/
├── README.md
├── index.html
└── ESP32_Audio_Sender/
    └── ESP32_Audio_Sender.ino
```

## Features

- ESP32 analog heart-sound acquisition
- 4000 Hz audio sampling
- 128-sample audio blocks
- Checksum-protected serial packets
- Web Serial connection at 115200 baud
- Real-time audio playback
- Live waveform
- Smoothed envelope detection
- Adaptive noise threshold
- S1 (LUB) and S2 (DUB) event detection
- S1-to-S1 BPM calculation
- Median filtering of recent BPM values
- Heart animation

## Packet format

The ESP32 sends:

```
A5 5A | 128 | 128 audio bytes | checksum
```

Total packet size: 132 bytes.

The checksum is the 8-bit sum of the 128 audio bytes.

## BPM calculation

The browser does not use the maximum amplitude of each packet as a heartbeat.

It processes the continuous audio sample stream:

```
Raw samples
   ↓
Rectification
   ↓
Envelope smoothing
   ↓
Adaptive noise threshold
   ↓
Sound event
   ↓
S1/S2 identification
   ↓
S1-to-S1 interval
   ↓
BPM
   ↓
Median filtering
```

BPM is calculated as:

```
BPM = 60000 / S1-to-S1 interval (ms)
```

Example:

```
S1-to-S1 = 833 ms
BPM ≈ 72
```

## ESP32 settings

- Analog input: GPIO 34
- Digital input: GPIO 27
- Sample rate: 4000 Hz
- Audio block: 128 samples
- Serial baud: 115200
- Bluetooth device: ESP32-HEART

## Running the web application

Use a browser supporting Web Serial, such as Google Chrome or Microsoft Edge.

1. Upload the ESP32 sketch.
2. Connect ESP32 through USB.
3. Close Arduino Serial Monitor.
4. Open `index.html` from a suitable local/web server.
5. Press **Connect ESP32 (USB)**.
6. Select the ESP32 serial port.

## Important

The ESP32 packet format must remain unchanged unless the browser packet parser is also updated.

This project is an engineering/research prototype. The displayed BPM is an algorithmic estimate and is not a medical diagnostic measurement.
