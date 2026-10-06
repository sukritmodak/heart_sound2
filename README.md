# ESP32 Heart Sound Monitor

Real-time ESP32 heart-sound visualization in the browser using USB Serial.

## Current BPM method

The browser receives the unchanged 132-byte audio packets at 4000 Hz.

- S1 = first heart sound / systolic sound.
- S2 = second heart sound / diastolic sound.
- S1-to-S2 timing confirms the two sounds belong to one cardiac cycle.
- BPM is NOT calculated from S1-to-S2.
- BPM is calculated only from successive S1-to-S1 intervals.

BPM = 60000 / S1-to-S1 interval in milliseconds.

The displayed BPM is refreshed every 1 second using the latest valid S1-to-S1 interval.

## Heart visualization

The browser uses the same detected audio events for the animation:

- S1 triggers the first heart beat animation.
- S2 triggers the second heart beat animation.
- Therefore the visual heart follows the detected heart-sound timing and frequency.

## Project

heart_sound2/
├── README.md
├── index.html
└── ESP32_Audio_Sender/
    └── ESP32_Audio_Sender.ino

## Audio packet format

The ESP32 packet format is unchanged:

- Header: A5 5A
- Length: 128
- Payload: 128 unsigned 8-bit audio samples
- Checksum: 8-bit sum of payload
- Total: 132 bytes
- Sampling rate: 4000 Hz
- USB Serial: 115200 baud

## Setup

1. Open ESP32_Audio_Sender/ESP32_Audio_Sender.ino in Arduino IDE.
2. Select the correct ESP32 board and COM port.
3. Upload the firmware.
4. Close Arduino Serial Monitor.
5. Open index.html in Chrome or Edge.
6. Press Connect ESP32 (USB).
7. Allow the browser to access the ESP32 serial port.
8. Place the sensor/probe correctly and wait for repeated S1/S2 sounds.

## Detection safeguards

The browser detector uses the actual sample clock rather than USB packet arrival time. It applies adaptive baseline/noise estimation, a minimum sound-event separation, S1/S2 timing limits, and physiologically bounded S1-to-S1 intervals.

If the signal is too weak, clipped, heavily noisy, or the probe is not positioned correctly, BPM may remain -- rather than displaying an unreliable value.

## Important

This is an engineering/research prototype and not a medical diagnostic device.
