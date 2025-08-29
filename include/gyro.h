#ifndef GYRO_H
#define GYRO_H

#include "common.h"
#include <Wire.h>

// MPU6050 I2C address and registers
#define MPU6050_ADDR 0x68
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43

namespace Gyro
{
    // Variables to store sensor data
    int16_t accel_x, accel_y, accel_z;
    int16_t gyro_x, gyro_y, gyro_z;
    int16_t temperature;
    
    // Processed values
    float accel_x_g, accel_y_g, accel_z_g;
    float roll, pitch;

    void begin()
    {
        Wire.beginTransmission(MPU6050_ADDR);
        Wire.write(PWR_MGMT_1);
        Wire.write(0); // Wake up the MPU6050
        Wire.endTransmission(true);
    }

    void readMPU6050()
    {
        Wire.beginTransmission(MPU6050_ADDR);
        Wire.write(ACCEL_XOUT_H);
        Wire.endTransmission(false);
        Wire.requestFrom(MPU6050_ADDR, 14, true);

        // Read accelerometer data (6 bytes)
        accel_x = Wire.read() << 8 | Wire.read();
        accel_y = Wire.read() << 8 | Wire.read();
        accel_z = Wire.read() << 8 | Wire.read();

        // Skip temperature and gyroscope data
        for (int i = 0; i < 8; i++)
        {
            Wire.read();
        }
    }
    
    void update()
    {
        readMPU6050();
        
        // Convert to g-forces
        accel_x_g = accel_x / 16384.0;
        accel_y_g = accel_y / 16384.0;
        accel_z_g = accel_z / 16384.0;
        
        // Calculate angles
        roll = atan2(accel_y_g, accel_z_g) * 180.0 / PI;
        pitch = atan2(-accel_x_g, sqrt(accel_y_g * accel_y_g + accel_z_g * accel_z_g)) * 180.0 / PI;
    }

  
    String getTiltDirection(float roll, float pitch)
    {
        String result = "";

        if (abs(roll) > 10)
        {
            result += roll > 0 ? "Tilted Right " : "Tilted Left ";
            result += String(abs(roll), 0) + "° ";
        }

        if (abs(pitch) > 10)
        {
            result += pitch > 0 ? "Tilted Forward " : "Tilted Backward ";
            result += String(abs(pitch), 0) + "°";
        }

        return result.length() > 0 ? result : "Level";
    }
    
    // Getter functions
    float getRoll() { return roll; }
    float getPitch() { return pitch; }
    String getTiltString() { return getTiltDirection(roll, pitch); }
    bool isLevel() { return abs(roll) < 5 && abs(pitch) < 5; }
}

#endif