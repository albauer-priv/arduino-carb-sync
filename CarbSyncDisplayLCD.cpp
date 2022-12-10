#include "CarbSyncDisplayLCD.h"
#include <SPI.h>
// #include <TFT.h>
#include <Ucglib.h>


// #include <Adafruit_GFX.h>    // Core graphics library
// #include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
// #include <SPI.h>


const int scale_box_height = 45;
const int scale_box_yPos = 0;
const int line_height = 15;


CarbSyncDisplayLCD::CarbSyncDisplayLCD() {

}


void CarbSyncDisplayLCD::setup(int cols, int rows) {
    this->_TFTCols = cols;
    this->_TFTRows = rows;


    /*
    _tft.begin();
    _tft.setRotation(3);
    _tft.background(0, 0, 0);
    */

    this->actDisplayData.differenceADCValue = 0.0;
    this->actDisplayData.differencekPaValue = 0.0;
    this->actDisplayData.minSmoothedkPaValue[0] = 0.0;
    this->actDisplayData.minSmoothedADCValue[0] = 0.0;
    this->actDisplayData.minSmoothedkPaValue[1] = 0.0;
    this->actDisplayData.minSmoothedADCValue[1] = 0.0;
    this->actDisplayData.minkPaValue[0] = 0.0;
    this->actDisplayData.minkPaValue[1] = 0.0;
    this->actDisplayData.lowerSideIndicator = 0;
    this->actDisplayData.gaugeScaleFactor = 100;
    this->actDisplayData.lastIndicatorXPos = 0;


    _tft.begin(UCG_FONT_MODE_SOLID);
    _tft.clearScreen();

    _tft.setRotate90();
    _tft.setFont(ucg_font_courB12_mf);
    _tft.setColor(0, 255, 255, 255); // indedx 0 == white
    _tft.setColor(1, 0, 0, 0); // index 1 == black
    _tft.setColor(2, 255, 0, 0); // index 2 == green
    _tft.setColor(3, 255, 255, 0); // index 3 == yellow

};

void CarbSyncDisplayLCD::displaySplashScreen() {

};

void CarbSyncDisplayLCD::displaySyncScreen() {
    char text[256];

    _tft.setColor(0, 255, 255, 255); // indedx 0 == white
    _tft.drawBox(0, scale_box_yPos,_tft.getWidth()-1, scale_box_height);

    _tft.setColor(0, 0, 0);
    _tft.drawHLine(0,scale_box_yPos + (scale_box_height/3) ,_tft.getWidth()-1);

    _tft.setColor(0, 0, 0);
    for (int i=1; i<_tft.getWidth(); i++) {
        if ((i % 20) == 0) {
            _tft.drawVLine(i, scale_box_yPos+1, (scale_box_height/3)-1);
        }
    }

    _tft.setColor(255, 0, 0);
    _tft.drawBox((_tft.getWidth()/2)-1, scale_box_yPos+1, 3, (scale_box_height/3)-1);


    _showData(true);
  
};

void CarbSyncDisplayLCD::updateSyncScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData) {
    // int scale, mapValue[2], rpmValue, mapValueDifference;
    int scaleMultiply[] = {1, 10, 100, 1000};
    int scaleBase[] = {1, 2, 5};
    int i, j, limit;
    bool exit = false;

    this->actDisplayData.minkPaValue[0] = data[0].getMinimumMAPValueAskPa();
    this->actDisplayData.minkPaValue[1] = data[1].getMinimumMAPValueAskPa();
    this->actDisplayData.minSmoothedkPaValue[0] = data[0].getSmoothedMinimumMAPValueAskPa();
    this->actDisplayData.minSmoothedkPaValue[1] = data[1].getSmoothedMinimumMAPValueAskPa();
    this->actDisplayData.minSmoothedADCValue[0] = data[0].getSmoothedMinimumADCValue();
    this->actDisplayData.minSmoothedADCValue[1] = data[1].getSmoothedMinimumADCValue();
    this->actDisplayData.differenceADCValue = abs(this->actDisplayData.minSmoothedADCValue[0] - this->actDisplayData.minSmoothedADCValue[1]);
    this->actDisplayData.differencekPaValue = abs(this->actDisplayData.minSmoothedkPaValue[0] - this->actDisplayData.minSmoothedkPaValue[1]);

    if (this->actDisplayData.minSmoothedADCValue[0] < this->actDisplayData.minSmoothedADCValue[1]) {
        this->actDisplayData.lowerSideIndicator = -1;
    } else if (this->actDisplayData.minSmoothedADCValue[0] > this->actDisplayData.minSmoothedADCValue[1]) {
        this->actDisplayData.lowerSideIndicator = 1;
    } else {
        this->actDisplayData.lowerSideIndicator = 0;
    }


    this->actDisplayData.gaugeScaleFactor = 1;
    i = 0;
    j = 0;
    exit = false;

    while ((i < sizeof(scaleMultiply)) && !exit)  {
        j = 0;
        while ((j < sizeof(scaleBase)) && !exit)  {
            this->actDisplayData.gaugeScaleFactor = scaleBase[j] * scaleMultiply[i];
            j++;

            exit = round(this->actDisplayData.differenceADCValue) < (4*this->actDisplayData.gaugeScaleFactor);
        }
        i++;
    }

    _showData(false);
};


void CarbSyncDisplayLCD::_showData(bool firstrun) {
    char text[256];
    char lowerSideIndicatorChar = ' ';
    int trianglexPos;


    // we draw first all white on black things ...
    _tft.setColor(0, 255, 255, 255); // indedx 0 == white
    _tft.setColor(1, 0, 0, 0); // index 1 == black


    if (actDisplayData.lowerSideIndicator < 0) {
        lowerSideIndicatorChar = '<';
    } else if (actDisplayData.lowerSideIndicator > 0) {
        lowerSideIndicatorChar = '>';
    } else {
        lowerSideIndicatorChar = '=';
    }

    if (firstrun) {
        sprintf(text, "1 %c 2", lowerSideIndicatorChar);
    } else {
        sprintf(text, "%c", lowerSideIndicatorChar);
    }
    _tft.drawString((_tft.getWidth()/2)- (_tft.getStrWidth(text)/2), scale_box_yPos + scale_box_height + line_height, 0, text);


    if (firstrun) {
        sprintf(text, "d       kPa", actDisplayData.differencekPaValue);
    } else {
        dtostrf(actDisplayData.differencekPaValue, 4, 1, text);
    }
    _tft.drawString((_tft.getWidth()/2)- (_tft.getStrWidth(text)/2) -2, scale_box_yPos + scale_box_height + 2*line_height, 0, text);


    if (firstrun) {
        sprintf(text, "d       ADC", actDisplayData.differenceADCValue);
    } else {
        dtostrf(actDisplayData.differenceADCValue, 4, 1, text);
    }
    _tft.drawString((_tft.getWidth()/2)- (_tft.getStrWidth(text)/2) -2, scale_box_yPos + scale_box_height + 3*line_height, 0, text);



/*
    if (firstrun) {
    _tft.drawString((_tft.getWidth()/2)- (_tft.getStrWidth("kPa")/2) -2, scale_box_yPos + scale_box_height + 5*line_height, 0, "kPa");
    }

    dtostrf(actDisplayData.minkPaValue[0], 5, 1, text);
    _tft.drawString(_tft.getWidth()- _tft.getStrWidth(text) -1, scale_box_yPos + scale_box_height + 5*line_height, 0, text);

    dtostrf(actDisplayData.minkPaValue[1], 5, 1, text);
    _tft.drawString(1, scale_box_yPos + scale_box_height + 5*line_height, 0, text);
*/

    // "erase" triangle ...
    _tft.drawTriangle(  actDisplayData.lastIndicatorXPos, scale_box_yPos + (scale_box_height/3) + 2, 
                        actDisplayData.lastIndicatorXPos + 5, scale_box_yPos + scale_box_height -2, 
                        actDisplayData.lastIndicatorXPos - 5, scale_box_yPos + scale_box_height -2);

    // now it's time for black on white ...
    _tft.setColor(1, 255, 255, 255);    // indedx 1 == white
    _tft.setColor(0, 0, 0, 0);          // index 0 == black

    
    sprintf(text, "x%3d", actDisplayData.gaugeScaleFactor);
    _tft.drawString((_tft.getWidth()-1)- (_tft.getStrWidth(text)) , scale_box_yPos + scale_box_height -3, 0, text);


    trianglexPos = map(actDisplayData.differenceADCValue, 0, actDisplayData.gaugeScaleFactor * 4, 0, _tft.getWidth()/2);

    if (actDisplayData.lowerSideIndicator < 0) {
        trianglexPos = (_tft.getWidth()/2) + trianglexPos;
    } else {
        trianglexPos = (_tft.getWidth()/2) - trianglexPos;
    }
    actDisplayData.lastIndicatorXPos = trianglexPos;

    _tft.drawTriangle(  trianglexPos, scale_box_yPos + (scale_box_height/3) + 2, 
                        trianglexPos + 5, scale_box_yPos + scale_box_height -2, 
                        trianglexPos - 5, scale_box_yPos + scale_box_height -2);

/*
    sprintf(text, "%3d", trianglexPos);
    _tft.setColor(r, g, b);
    _tft.drawString((_tft.getWidth()/2)- (_tft.getStrWidth(text)/2) -2, scale_box_yPos + scale_box_height + 4*line_height, 0, text);
*/



};
