#include <Wire.h>
#include <SPI.h>
#include <mcp2515.h>

#define APPS1 A0
#define APPS2 A1
#define brake A3
#define RTDS 10
#define P1 13
#define P2 12
#define SDC_IN 9
#define START_BTN 8
#define Brake_Out 7
#define SC_Relay 4 // relay out
#define AIR1 22
#define AIR2 23
#define MG1 24
#define MG2 25
#define MotorCurrent1 A10
#define MotorCurrent2 A8

#define ADDR1 0x60 // motorout1 DAC address
#define ADDR2 0x61 // motorout2 DAC address

// #define AIR1 1
// #define AIR2 2

int val1 = 0;
int val2 = 0;
int var1 = 0;
int var2 = 0;

int brakein = 0;
int motorout = 0;

MCP2515 mcp2515(53);

void setup()
{
    Serial.begin(9600); // Serial communication Intialisation
    Wire.begin();       // I2C Intialisation

    // CAN module  Intialisation
    mcp2515.reset();
    mcp2515.setBitrate(CAN_1000KBPS);
    mcp2515.setNormalMode();

    // Timer Intialistion
    TCCR1B = 0;
    TCCR1A = 0;
    TCCR1B = bit(CS12);

    // throttle of both motors to 0
    DAC1(0);
    DAC2(0);
}

void loop()
{
    control();
}

void control()
{
    val1 = analogRead(APPS1);
    val2 = analogRead(APPS2);
    brakein = analogRead(brake);
    // Serial.print(val2);

    //
    if (brakein > 205)
    {
        DAC1(0);
        DAC2(0);

        // Serial.print("brake:");
        // Serial.println(brake);
    }

    // sensor failure (outside acceptable range)
    else if (val1 < 103 || val1 > 920 || val2 < 103 || val2 > 920)
    {
        // Serial.println("sensor failure (outside acceptable range)");
        DAC1(0);
        DAC2(0);
    }

    else if (abs(val1 - val2) > 103)
    {
        DAC1(0);
        DAC2(0);

        TCNT1 = 0;
        while (TCNT1 <= 6250)
        {
            var1 = analogRead(APPS1);
            var2 = analogRead(APPS2);
            if (abs(var1 - var2) < 103)
            {
                break;
            }
        }

        if (abs(var1 - var2) > 103)
        {
            // Serial.print("APPS implausibility failure :");
            // Serial.println(var1-var2);
        }
    }

    else
    {
        int motorpre;
        motorpre = (val1 + val2) * 2;
        motorout = tmap(motorpre);
        DAC1(motorout);
        // DAC2(motorout);
        // Serial.print("ssss");
        Serial.println(motorout);
        Serial.println(val1);
        Serial.println(val2);
    }
}

void RTDS_check()
{
    if (digitalRead(START_BTN) == HIGH)
    {
        if (digitalRead(AIR1) == HIGH && digitalRead(AIR2) == HIGH && digitalRead(SDC_IN) == HIGH && analogRead(brake) > 512 &&
            digitalRead(MG1) == HIGH && digitalRead(MG2) == HIGH)
        {
            digitalWrite(RTDS, HIGH);
        }
    }
}

void CAN()
{
}

void DAC1(int output)
{
    Wire.beginTransmission(ADDR1);
    Wire.write(64);                 // cmd to update the DAC
    Wire.write(output >> 4);        // the 8 most significant bits…
    Wire.write((output & 15) << 4); // the 4 least significant bits…
    Wire.endTransmission();
}

void DAC2(int output)
{
    Wire.beginTransmission(ADDR2);
    Wire.write(64);                 // cmd to update the DAC
    Wire.write(output >> 4);        // the 8 most significant bits…
    Wire.write((output & 15) << 4); // the 4 least significant bits…
    Wire.endTransmission();
}

int tmap(int inp)
{
    int tmapout;
    if ((digitalRead(P1) == HIGH) && (digitalRead(P1) == HIGH))
    {
        tmapout = inp;
    }
    else if ((digitalRead(P1) == LOW) && (digitalRead(P1) == LOW))
    {
        tmapout = map(inp, 0, 4095, 0, 2047);
    }
    else
    {
        tmapout = map(inp, 0, 4095, 0, 3072);
    }

    return tmapout;
}
