

#include "CylinderManifoldAbsolutePressureData.h"
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

#ifndef CARBSYNCDISPLAY_H
#define CARBSYNCDISPLAY_H

class CarbSyncDisplay {
    public:
        // CarbSyncDisplay(int cols, int rows);

        void setup(int cols, int rows);
        void displaySplashScreen();
        void displaySyncScreen();
        void updateSyncScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData);

    private:
        int _LCDCols;
        int _LCDRows;
        hd44780_I2Cexp _lcd; // declare lcd object: auto locate & config exapander chip
};

#endif