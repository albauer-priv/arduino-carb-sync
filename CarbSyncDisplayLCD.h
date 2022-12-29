

#include "CylinderManifoldAbsolutePressureData.h"

#include <SPI.h>
#include <TFT_eSPI.h>


#ifndef CARBSYNCDISPLAYLCD_H
#define CARBSYNCDISPLAYLCD_H

#define CARB_SYNC_DISPLAY_NUM_SCREENS 2


// root class for displaying data ...
class CarbSyncDisplayLCD {
    public:
        CarbSyncDisplayLCD();

        // void setup(int cols, int rows);
        void setup();

        /*
        virtual void setupScreen();
        virtual void destroyScreen();
        void updateScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int additionalData);
        */

        /*
        void setupSplashScreen();
        void displaySplashScreen();
        void destroySplashScreen();
        */

        void updateScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int additionalData);
        void toggleScreen();


        void setupCalibrationScreen();
        void updateCalibrationScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int measures);
        void destroyCalibrationScreen();

        void setupSyncScreen();
        void updateSyncScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int measuresPerSec);
        void destroySyncScreen();

        void setupSyncBarScreen();
        void updateSyncBarScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int measuresPerSec);
        void destroySyncBarScreen();

        void setupAbsolutePressureBarScreen();
        void updateAbsolutePressureBarScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int measuresPerSec);
        void destroyAbsolutePressureBarScreen();

        void setupMinMaxPressureBarScreen();
        void updateMinMaxPressureBarScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int measuresPerSec);
        void destroyMinMaxPressureBarScreen();

    protected: 

    private:
        TFT_eSPI _tft = TFT_eSPI();
        TFT_eSprite _fb1 = TFT_eSprite(&_tft);  // Sprites are used as framebuffer for "non"-flicker display of data
        TFT_eSprite _fb2 = TFT_eSprite(&_tft);  // Create Sprite object "_fb" with pointer to "_tft" object
        TFT_eSprite _fb3 = TFT_eSprite(&_tft);  // the pointer is used by pushSprite() to push it onto the TFT
        TFT_eSprite _fb4 = TFT_eSprite(&_tft);  

        struct spritePushCoordinates {
            int x;
            int y;
        } _fb1PushCoords, _fb2PushCoords, _fb3PushCoords, _fb4PushCoords;

        struct displayData {
            float minkPaValue[2];
            float minSmoothedkPaValue[2];
            float maxSmoothedkPaValue[2];
            float minSmoothedADCValue[2];
            float smoothedkPaValue[2];
            float differenceADCValue;
            float differencekPaValue;
            int lowerSideIndicator;
            int gaugeScaleFactor;
            int lastIndicatorXPos;
        } actDisplayData;


        const int scale_box_height = 72;
        const int scale_box_yPos = 0;
        const int line_height = 15;

        int actualScreen = -1;

        // Palette color table
        uint16_t _colorpalette[16];

        unsigned long calledTimes = 0;

        void _updateInternalData(CylinderManifoldAbsolutePressureData data[], int sizeOfData);
        void _displaySyncData (int measuresPerSec, int actualRPM);
        void _drawDashedLine(int xs, int ys, int width, int height, int fgcolor, int bgcolor);

};


#endif
