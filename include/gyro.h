#ifndef GYRO_H
#define GYRO_H

#include "common.h"

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
        gyro_wire.beginTransmission(MPU6050_ADDR);
        gyro_wire.write(PWR_MGMT_1);
        gyro_wire.write(0); // Wake up the MPU6050
        gyro_wire.endTransmission(true);
    }
    //
    void scanI2C()
    {
        byte error, address;
        int deviceCount = 0;

        debugln("Scanning I2C bus...");

        for (address = 1; address < 127; address++)
        {
            gyro_wire.beginTransmission(address);
            error = gyro_wire.endTransmission();

            if (error == 0)
            {
                debug("I2C device found at address 0x");
                if (address < 16)
                    debug("0");
                debug(address, HEX);
                debugln("");
                deviceCount++;
            }
            else if (error == 4)
            {
                debug("Unknown error at address 0x");
                if (address < 16)
                    debug("0");
                debugln(address, HEX);
            }
        }

        if (deviceCount == 0)
            debugln("No I2C devices found");
        else
            debugln("Scan complete");
        debugln("");
    }

    void readMPU6050()
    {
        gyro_wire.beginTransmission(MPU6050_ADDR);
        gyro_wire.write(ACCEL_XOUT_H);
        gyro_wire.endTransmission(false);
        gyro_wire.requestFrom((uint8_t)MPU6050_ADDR, (size_t)14, (bool)true);

        // Read accelerometer data (6 bytes)
        accel_x = gyro_wire.read() << 8 | gyro_wire.read();
        accel_y = gyro_wire.read() << 8 | gyro_wire.read();
        accel_z = gyro_wire.read() << 8 | gyro_wire.read();

        // Skip temperature and gyroscope data
        for (int i = 0; i < 8; i++)
        {
            gyro_wire.read();
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