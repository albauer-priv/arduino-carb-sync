

#include "CylinderManifoldAbsolutePressureData.h"

#include <SPI.h>
#include <Ucglib.h>

//#include <TFT.h>

//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
// #include <SPI.h>


#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8
#define TFT_MOSI 11  // Data out
#define TFT_SCLK 13  // Clock out



#ifndef CARBSYNCDISPLAYLCD_H
#define CARBSYNCDISPLAYLCD_H

class CarbSyncDisplayLCD {
    public:
        CarbSyncDisplayLCD();

        // void setup(int cols, int rows);
        void setup();
        void displaySplashScreen();
        void displaySyncScreen();
        void updateSyncScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData);

    private:
        // TFT _tft = TFT(TFT_CS, TFT_DC, TFT_RST);
        Ucglib_ST7735_18x128x160_HWSPI _tft = Ucglib_ST7735_18x128x160_HWSPI(/*cd=*/ TFT_DC, /*cs=*/ TFT_CS, /*reset=*/ TFT_RST);

        struct displayData {
            float minkPaValue[2];
            float minSmoothedkPaValue[2];
            float minSmoothedADCValue[2];
            float differenceADCValue;
            float differencekPaValue;
            int lowerSideIndicator;
            int gaugeScaleFactor;
            int lastIndicatorXPos;

        } actDisplayData;

        void _showData (bool firstrun);

};

#endif