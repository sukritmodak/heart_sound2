#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#define ANALOG_PIN 34
#define DIGITAL_PIN 27

#define BEAT_STATE LOW

#define MIN_SOUND_GAP 150
#define MIN_S1_S2_GAP 100
#define MAX_S1_S2_GAP 500

#define SAMPLE_RATE 4000
const unsigned long SAMPLE_INTERVAL =
    1000000UL / SAMPLE_RATE;

#define AUDIO_BLOCK 128

uint8_t audioBuffer[AUDIO_BLOCK];
int audioIndex = 0;

unsigned long nextSampleTime = 0;

int previousDigitalState = HIGH;

unsigned long lastSoundTime = 0;
unsigned long s1Time = 0;

bool waitingForS2 = false;

unsigned long totalSoundNumber = 0;
unsigned long totalHeartBeatNumber = 0;

unsigned long windowStartTime = 0;

int soundCount5sec = 0;
int heartBeatCount5sec = 0;


/* =========================================================
   SEND TERMINAL MESSAGE
========================================================= */

void BT(String message)
{
    SerialBT.println(message);

    /*
     * Also show the same information
     * in the USB Serial Monitor.
     */

    Serial.println(message);
}


/* =========================================================
   AUDIO PACKET
========================================================= */

void sendAudioBlock()
{
    uint8_t checksum = 0;

    Serial.write(0xA5);
    Serial.write(0x5A);

    Serial.write((uint8_t)AUDIO_BLOCK);

    for(int i=0;i<AUDIO_BLOCK;i++)
    {
        Serial.write(audioBuffer[i]);
        checksum += audioBuffer[i];
    }

    Serial.write(checksum);
}


/* =========================================================
   SETUP
========================================================= */

void setup()
{
    Serial.begin(115200);

    /*
     * Classic Bluetooth device:
     * ESP32-HEART
     */

    SerialBT.begin("ESP32-HEART");

    pinMode(
        ANALOG_PIN,
        INPUT
    );

    pinMode(
        DIGITAL_PIN,
        INPUT
    );

    windowStartTime =
        millis();

    nextSampleTime =
        micros();


    BT("");
    BT("================================");
    BT("ESP32 HEART SOUND MONITOR");
    BT("================================");

    BT("Bluetooth device: ESP32-HEART");

    BT("USB audio: 4000 Hz");

    BT("Audio packet: 128 samples");

    BT("Browser BPM: S1-to-S1");

    BT("S1 + S2 = one cardiac cycle");

    BT("================================");

    BT("SYSTEM READY");

}


/* =========================================================
   MAIN LOOP
========================================================= */

void loop()
{

    unsigned long now =
        millis();


    /* =====================================================
       DIGITAL HEART SOUND DETECTION
    ===================================================== */

    int digitalState =
        digitalRead(
            DIGITAL_PIN
        );


    if(
        previousDigitalState == HIGH &&
        digitalState == BEAT_STATE
    )
    {

        unsigned long soundTime =
            millis();


        if(
            lastSoundTime == 0 ||
            soundTime -
            lastSoundTime >=
            MIN_SOUND_GAP
        )
        {

            totalSoundNumber++;

            soundCount5sec++;


            /* =============================================
               FIRST SOUND = S1
            ============================================= */

            if(!waitingForS2)
            {

                s1Time =
                    soundTime;

                waitingForS2 =
                    true;


                BT(
                    "S1 detected - Sound #" +
                    String(
                        totalSoundNumber
                    )
                );

            }


            /* =============================================
               SECOND SOUND = S2
            ============================================= */

            else
            {

                unsigned long gap =
                    soundTime -
                    s1Time;


                if(
                    gap >= MIN_S1_S2_GAP &&
                    gap <= MAX_S1_S2_GAP
                )
                {

                    totalHeartBeatNumber++;

                    heartBeatCount5sec++;


                    BT(
                        "S2 detected - S1-S2 gap = " +
                        String(gap) +
                        " ms"
                    );


                    waitingForS2 =
                        false;

                }


                /* =========================================
                   MISSED S2
                ========================================= */

                else if(
                    gap >
                    MAX_S1_S2_GAP
                )
                {

                    s1Time =
                        soundTime;

                    waitingForS2 =
                        true;


                    BT(
                        "S2 missed - new S1 detected"
                    );

                }


                else
                {

                    BT(
                        "Ignored short duplicate sound"
                    );

                }

            }


            lastSoundTime =
                soundTime;

        }

    }


    previousDigitalState =
        digitalState;


    /* =====================================================
       FIVE SECOND STATUS
    ===================================================== */

    if(
        now -
        windowStartTime >=
        5000
    )
    {

        float bpm =
            heartBeatCount5sec *
            12.0;


        BT("");

        BT(
            "========== 5 SECOND RESULT =========="
        );


        BT(
            "Total sounds = " +
            String(
                soundCount5sec
            )
        );


        BT(
            "S1-S2 pairs = " +
            String(
                heartBeatCount5sec
            )
        );


        BT(
            "BPM (pair count) = " +
            String(
                bpm,
                1
            )
        );


        BT(
            "Browser BPM = S1-to-S1 timing"
        );


        BT(
            "====================================="
        );

        BT("");


        soundCount5sec = 0;

        heartBeatCount5sec = 0;

        windowStartTime =
            now;

    }


    /* =====================================================
       4000 Hz AUDIO SAMPLING
    ===================================================== */

    unsigned long currentMicros =
        micros();


    if(
        (long)(
            currentMicros -
            nextSampleTime
        ) >= 0
    )
    {

        nextSampleTime +=
            SAMPLE_INTERVAL;


        uint8_t sample =
            (uint8_t)(
                analogRead(
                    ANALOG_PIN
                ) >> 4
            );


        audioBuffer[
            audioIndex++
        ] = sample;


        if(
            audioIndex >=
            AUDIO_BLOCK
        )
        {

            sendAudioBlock();

            audioIndex = 0;

        }

    }

}
