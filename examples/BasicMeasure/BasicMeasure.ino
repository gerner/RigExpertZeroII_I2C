#include "RigExpertZeroII_I2C.h"

#define RX_PIN 4
#define TX_PIN 7
#define ZEROII_Reset_Pin  2

RigExpertZeroII_I2C ZERO;

void setup()
{
    pinMode(ZEROII_Reset_Pin, OUTPUT);
    digitalWrite(ZEROII_Reset_Pin, LOW);
    Serial.begin(38400);
    Serial.flush();
    digitalWrite(ZEROII_Reset_Pin, HIGH );
    while(!ZERO.startZeroII())
    {
        Serial.println("ZERO not found!");
        delay(500);
    }
    Serial.println("----------------------------");
    Serial.println("Found RigExpert ZEROII!");
    String str = "Version: ";
    Serial.println(str + ZERO.getMajorVersion() + "." + ZERO.getMinorVersion() +
    ", HW Revision: " + ZERO.getHwRevision() +
    ", SN: " + ZERO.getSerialNumber());
}

void loop()
{
    int32_t centerFq = 150000000;// 15 000 000 Hz
    int32_t rangeFq =  300000000;// 30 000 000 Hz
    int32_t dotsNumber = 30;

    int32_t startFq = centerFq - (rangeFq/2);
    int32_t endFq = centerFq + (rangeFq/2);
    int32_t stepFq = (endFq - startFq)/dotsNumber;

    for(int i = 0; i <= dotsNumber; ++i)
    {
        int32_t temp = startFq + (stepFq*i);
        ZERO.startMeasure(temp);
        Serial.print("Fq: ");
        Serial.print(temp);
        Serial.print(", R: ");
        Serial.print(ZERO.getR());
        Serial.print(", Rp: ");
        Serial.print(ZERO.getRp());
        Serial.print(", X: ");
        Serial.print(ZERO.getX());
        Serial.print(", Xp: ");
        Serial.print(ZERO.getXp());
        Serial.print(", SWR: ");
        Serial.print(ZERO.getSWR());
        Serial.print(", RL: ");
        Serial.print(ZERO.getRL());
        Serial.print(", Z: ");
        Serial.print(ZERO.getZ());
        Serial.print(", Phase: ");
        Serial.print(ZERO.getPhase());
        Serial.print(", Rho: ");
        Serial.print(ZERO.getRho());
        Serial.print("\r\n");
    }
    Serial.print("------------------------\r\n");
    delay(5000);
}
