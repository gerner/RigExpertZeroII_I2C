/*
   RigExpertZeroII_I2C.h - Library for easy work with RigExpert ZEROII.
   Created by RigExpert Ltd., May 24, 2022.
   Released into the public domain.

   Modified by Ed March WB9RAA fixed several bugs
   Software timing loops => delay for differet speed CPU's
   ATMEL 328 UNO doubles and floats are both 4 bytes.
   ESP32 doubles are 8 bytes sizeof and floats are 4 bytes
   Both CPU have IEEE-754 floating point formats.
   Tuned data transfers to ~100-200ms per measurement
//
returns R/X all other values are calculated on host CPU side
does not need Z0 to send to the ST32F because we are not
using the ST Micro on the AA-ZERO II for SWR math.
All calulations are done in startMeasure()
Getters simply return member variables.
*/
#ifndef RigExpertZeroII_I2C_h
#define RigExpertZeroII_I2C_h

#ifndef byte
typedef unsigned char byte;
#endif

#include <Wire.h>

#define ZEROIIAddress 0x5B
#define MAX_FQ 1000000000 // 1000 MHz

enum ZeroII_I2C_Status
{
    ZEROII_I2C_STATUS_BUSY_USB = 1,
    ZEROII_I2C_STATUS_BUSY_SPI,
    ZEROII_I2C_STATUS_BUSY_I2C,
    ZEROII_I2C_STATUS_BUSY_UART,
    ZEROII_I2C_STATUS_IDLE,
    ZEROII_I2C_STATUS_READY,
    ZEROII_I2C_STATUS_ERROR,
};

class RigExpertZeroII_I2C
{
    public:
        RigExpertZeroII_I2C();
        bool startZeroII(void);
        bool isInited(void); //WB9RAA added
        bool getFwVersion(void);
        bool startMeasure(int32_t fq);
        unsigned char getMajorVersion(void);
        unsigned char getMinorVersion(void);
        unsigned char getHwRevision(void);
        unsigned long getSerialNumber(void);
        unsigned char getStatus(void);
        inline double getZ0(void) { return m_Z0;}
        inline void   setZ0(double Z0) { m_Z0 = Z0; }
        //
        bool computeAll(double Z0, double R, double X);
        // WB9RAA faster smaller inline code 
        inline double getR(void)  { return m_R;}
        inline double getRp(void) { return m_Rp;}
        inline double getX(void)  { return m_X;}
        inline double getXp(void) { return m_Xp;}
        inline double getSWR(void){ return m_SWR;}
        inline double getRL(void) { return m_RL;}
        inline double getZ(void)  { return m_Z;}
        inline double getPhase(void){ return m_Phase;}
        inline double getRho(void){ return m_Rho;}
        inline void   clearErrors(void){ m_Errors=0;}
        inline unsigned long   getErrors(void){ return m_Errors;}
    private:
        bool readRX(void);
        //
        unsigned long m_Errors;
        double m_Z0;
        // WB9RAA
        // UNO sizeof(double) == sizeof(float) but most CPU
        // do not - bug fixed below
        // ST32F Micro must store floats in same 4 byte
        // format for this to work properly
        //
        float m_R; //WB9RAA IEEE-754 from Zero2 HW CPU
        float m_X; //WB9RAA IEEE-754 from Zero2 HW CPU
        //
        double m_SWR;
        double m_RL;
        double m_Z;
        double m_Rp;
        double m_Xp;
        double m_Phase;
        double m_Rho;
        bool   m_isInited;
        //	
        unsigned char m_majorVersion = 0;
        unsigned char m_minorVersion = 0;
        unsigned char m_hwRevision = 0;
        unsigned long m_serialNumber = 0;
};

#endif

