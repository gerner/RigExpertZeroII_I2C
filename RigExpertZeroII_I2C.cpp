#include "RigExpertZeroII_I2C.h"
#include "Arduino.h"
#include <Wire.h>

RigExpertZeroII_I2C::RigExpertZeroII_I2C()
{
    m_Z0 = 50.0;
    m_R = 0;
    m_X = 0;
    m_SWR = 0;
    m_RL = 0;
    m_Phase = 0;
    m_Rho = 0;
    m_isInited = false;
    m_majorVersion = 0;
    m_minorVersion = 0;
    m_hwRevision = 0;
    m_serialNumber = 0;
    m_Errors = 0;
}


bool RigExpertZeroII_I2C::startZeroII(void)
{
    Wire.begin(); // init ZEROII I2C
    delay(100);
    if(getFwVersion())
    {
        m_isInited = true;
        return true;
    }
    if(getFwVersion())
    {
        m_isInited = true;
        return true;
    }
    m_Errors++;
    return false;
}

bool RigExpertZeroII_I2C::isInited(void)
{
    return m_isInited;
}


bool RigExpertZeroII_I2C::getFwVersion(void)
{
    // if connect fails don't leak old values
    m_majorVersion = 0;
    m_minorVersion = 0;
    m_hwRevision = 0;
    m_serialNumber = 0;
    //
    Wire.beginTransmission(ZEROIIAddress);
    Wire.write(0x0E5);
    Wire.endTransmission();
    bool wait = true;
    short failCounter = 0;
    unsigned char buff[7]; 
    Wire.requestFrom(ZEROIIAddress, 7);
    while(wait)
    {
        delayMicroseconds(100);
        if(failCounter++ >= 5000)
        {
            wait = false;
        }
        if(Wire.available() >= 7)
        {
            wait = false;
            Wire.readBytes(buff, 7);
            m_majorVersion = buff[0];
            m_minorVersion = buff[1];
            m_hwRevision = buff[2];
            ((unsigned char*)&m_serialNumber)[0] = buff[3];
            ((unsigned char*)&m_serialNumber)[1] = buff[4];
            ((unsigned char*)&m_serialNumber)[2] = buff[5];
            ((unsigned char*)&m_serialNumber)[3] = buff[6];
            return true;
        }
    }
    m_Errors++;
    return false; 
}

unsigned char RigExpertZeroII_I2C::getMajorVersion(void)
{
    return m_majorVersion;
}

unsigned char RigExpertZeroII_I2C::getMinorVersion(void)
{
    return m_minorVersion;
}

unsigned char RigExpertZeroII_I2C::getHwRevision(void)
{
    return m_hwRevision;
}

unsigned long RigExpertZeroII_I2C::getSerialNumber(void)
{
    return m_serialNumber;
}

bool RigExpertZeroII_I2C::startMeasure(int32_t fq)
{
    if(!beginMeasure(fq)) {
        return false;
    }

    delay(20); // Added delay like UART

    bool wait = true;
    bool result = false;
    int failCounter = 0;
    while(wait)
    {
        delayMicroseconds(700);
        if(failCounter++ >= 1600)
        {
            wait = false;
            m_Errors++;
            return result;
        }
        if(tryReadRX())
        {
            result = true;
            wait = false;
        }
    }
    if(result)
    {
        computeAll(m_Z0,m_R,m_X);
    }
    return result;
}

bool RigExpertZeroII_I2C::beginMeasure(int32_t fq)
{
    if(!m_isInited)
    {
        return false;
    }
    if(fq > MAX_FQ)
    {
        fq = MAX_FQ;
    }

    bool result = false;
    byte command[5] = {0x6D,0x00,0x00,0x00,0x00};

    command[1] = ((unsigned char*)&fq)[0];
    command[2] = ((unsigned char*)&fq)[1];
    command[3] = ((unsigned char*)&fq)[2];
    command[4] = ((unsigned char*)&fq)[3];

    Wire.beginTransmission(ZEROIIAddress);
    Wire.write(command,5);
    Wire.endTransmission();

    return true;
}

bool RigExpertZeroII_I2C::tryReadRX(void)
{
    if(getStatus() == ZEROII_I2C_STATUS_READY)
    {
        return readRX();
    }
    else
    {
        return false;
    }
}

bool RigExpertZeroII_I2C::readRX(void)
{
    bool wait = true;
    short failCounter = 0;
    Wire.requestFrom( ZEROIIAddress, 8);
    while(wait)
    {
        delayMicroseconds(100);
        if(failCounter++ >= 10000)
        {
            wait = false;
            m_Errors++;
            return false;
        }
        if(Wire.available() >= 8)
        {
            wait = false;
            unsigned char buff[8];
            Wire.readBytes(buff, 8);
            ((unsigned char*)&m_R)[0] = buff[0];
            ((unsigned char*)&m_R)[1] = buff[1];
            ((unsigned char*)&m_R)[2] = buff[2];
            ((unsigned char*)&m_R)[3] = buff[3];

            ((unsigned char*)&m_X)[0] = buff[4];
            ((unsigned char*)&m_X)[1] = buff[5];
            ((unsigned char*)&m_X)[2] = buff[6];
            ((unsigned char*)&m_X)[3] = buff[7];
        }
    }
    return true;
}

unsigned char RigExpertZeroII_I2C::getStatus(void)
{
    Wire.beginTransmission(ZEROIIAddress);
    Wire.write(0x05A);
    Wire.endTransmission();
    bool wait = true;
    short failCounter = 0;
    unsigned char buff[1];
    Wire.requestFrom( ZEROIIAddress, 1);
    while(wait)
    {
        delay(7);
        if(failCounter++ >= 10000)
        {
            wait = false;
            m_Errors++;
            return ZEROII_I2C_STATUS_ERROR;
        }
        if(Wire.available() > 0)
        {
            wait = false;
            Wire.readBytes(buff, 1);
        }
    }
    return buff[0];
}

//
// The user can save only (3) values Z0,R,X and computer all others
//
bool RigExpertZeroII_I2C::computeAll(double Z0, double R, double X)
{
    m_Z0 = Z0;
    m_R = R;
    m_X = X;
    //
    // compute SWR 
    //
    if (R <= 0)
    {
        R = 0.001;
    }
    double SWR, Gamma;
    double XX = X * X;
    double denominator = (R + Z0) * (R + Z0) + XX;

    if (denominator == 0)
    {
        m_Errors++;
        return false;
    }
    Gamma = sqrt(((R - Z0) * (R - Z0) + XX) / denominator);
    if (Gamma == 1.0)
    {
        m_Errors++;
        return false;
    }
    SWR = (1 + Gamma) / (1 - Gamma);

    if ((SWR > 200) || (Gamma > 0.99))
    {
        SWR = 200;
    } 
    else if (SWR < 1)
    {
        SWR = 1;
    }
    m_SWR = SWR;
    m_RL = -20 * log10(Gamma);
    //
    m_Rp = m_R * (1+m_X * m_X / m_R / m_R);
    m_Z = sqrt( (R*R) + (X*X) );
    m_Xp = m_X*(1 + m_R * m_R / m_X / m_X);
    //
    // compute phase
    //
    double Rnorm = R/Z0;
    double Xnorm = X/Z0;
    double Denom = (Rnorm+1)*(Rnorm+1)+Xnorm*Xnorm;
    double RhoReal = ((Rnorm-1)*(Rnorm+1)+Xnorm*Xnorm)/Denom;
    double RhoImag = 2*Xnorm/Denom;
    m_Phase = atan2(RhoImag, RhoReal) / M_PI * 180.0;
    m_Rho = sqrt(RhoReal*RhoReal+RhoImag*RhoImag);
    //
    return true;
}


