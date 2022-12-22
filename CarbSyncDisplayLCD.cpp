#include "CarbSyncDisplayLCD.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch



const int scale_box_height = 72;
const int scale_box_yPos = 0;
const int line_height = 15;


CarbSyncDisplayLCD::CarbSyncDisplayLCD() {

}


void CarbSyncDisplayLCD::setup() {

    this->actDisplayData.differenceADCValue = 0.0;
    this->actDisplayData.differencekPaValue = 0.0;
    this->actDisplayData.minSmoothedkPaValue[0] = 0.0;
    this->actDisplayData.minSmoothedADCValue[0] = 0.0;
    this->actDisplayData.minSmoothedkPaValue[1] = 0.0;
    this->actDisplayData.minSmoothedADCValue[1] = 0.0;
    this->actDisplayData.minkPaValue[0] = 0.0;
    this->actDisplayData.minkPaValue[1] = 0.0;
    this->actDisplayData.lowerSideIndicator = 0;
    this->actDisplayData.gaugeScaleFactor = 1;
    this->actDisplayData.lastIndicatorXPos = 50;


    _tft.init();
    _tft.setRotation(1);
    _tft.setFreeFont(FM12);
    // _tft.setTextFont(FONT4);
    // _tft.setTextSize(2);
    _tft.fillScreen(TFT_BLACK);
};

void CarbSyncDisplayLCD::displaySplashScreen() {

};

void CarbSyncDisplayLCD::displaySyncScreen() {
    // draw lower pressure side indicator box ...
    // white rectangle 
    _tft.fillRect(0, scale_box_yPos, _tft.width()-1, scale_box_height, TFT_WHITE);

    // separator line
    _tft.drawLine(0,scale_box_yPos + (scale_box_height/3) ,_tft.width()-1, scale_box_yPos + (scale_box_height/3), TFT_BLACK);

    // tick marks 
    for (int i=1; i<_tft.width(); i++) {
        if ((i % 20) == 0) {
            _tft.drawLine(i, scale_box_yPos+1, i, (scale_box_height/3)-1, TFT_BLACK);
        }
    }

    // middle red tick
    _tft.fillRect((_tft.width()/2)-1, scale_box_yPos+1, 3, (scale_box_height/3)-1, TFT_RED);

    _showData(0, 0);
};

void CarbSyncDisplayLCD::updateSyncScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int measuresPerSec) {
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

    if (this->actDisplayData.minSmoothedkPaValue[0] < this->actDisplayData.minSmoothedkPaValue[1]) {
        this->actDisplayData.lowerSideIndicator = -1;
    } else if (this->actDisplayData.minSmoothedkPaValue[0] > this->actDisplayData.minSmoothedkPaValue[1]) {
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

            exit = round(this->actDisplayData.differencekPaValue) < (8*this->actDisplayData.gaugeScaleFactor);
        }
        i++;
    }

    _showData(measuresPerSec, data[0].getSmoothedRPMValue());
};




void CarbSyncDisplayLCD::_showData(int measuresPerSec, int actualRPM) {
    char text[256];
    char floatString[10];
    char floatString2[10];
    char lowerSideIndicatorChar = ' ';
    int trianglexPos;
    int textyPos = 0;
    int floatPrecision = 0;
    int lowerSideIndicatorBoxHeight = 0;


    // Serial.println("_showData()");
    _fb.createSprite(_tft.width(), 6 * _tft.fontHeight(GFXFF));
    _fb.fillSprite(TFT_BLACK);
    _fb.setFreeFont(FM12);


    // now it's time for white on black ...
    _fb.setTextColor(TFT_WHITE, TFT_BLACK, true);

    if (actDisplayData.lowerSideIndicator < 0) {
        lowerSideIndicatorChar = '<';
    } else if (actDisplayData.lowerSideIndicator > 0) {
        lowerSideIndicatorChar = '>';
    } else {
        lowerSideIndicatorChar = '=';
    }

    sprintf(text, "1 %c 2", lowerSideIndicatorChar);
    // _fb.drawString(text, (_tft.width()/2) - (_tft.textWidth(text)/2), scale_box_yPos + scale_box_height + 0*_tft.fontHeight(GFXFF), GFXFF);
    _fb.drawString(text, (_fb.width()/2) - (_fb.textWidth(text)/2), textyPos, GFXFF);
    textyPos += _tft.fontHeight(GFXFF);


    if (actDisplayData.differencekPaValue > 100) {
        floatPrecision = 0;
    } else {
        floatPrecision = 1;
    }
    dtostrf(actDisplayData.differencekPaValue, 4, floatPrecision, floatString);
    sprintf(text, "d %s kPa", floatString);
    _fb.drawString(text, (_fb.width()/2) - (_fb.textWidth(text)/2), textyPos, GFXFF);
    textyPos += _tft.fontHeight(GFXFF);

    if (actDisplayData.differenceADCValue > 100) {
        floatPrecision = 0;
    } else {
        floatPrecision = 1;
    }
    dtostrf(actDisplayData.differenceADCValue, 4, floatPrecision, floatString);
    sprintf(text, "d %s ADC", floatString);
    _fb.drawString(text, (_fb.width()/2)- (_fb.textWidth(text)/2), textyPos, GFXFF);
    textyPos += _tft.fontHeight(GFXFF);

    textyPos += _tft.fontHeight(GFXFF);

    dtostrf(actDisplayData.minkPaValue[0], 4, 1, floatString);
    dtostrf(actDisplayData.minkPaValue[1], 4, 1, floatString2);
    sprintf(text, "%s kPa %s", floatString, floatString2);
    _fb.drawString(text, (_fb.width()/2) - (_fb.textWidth(text)/2), textyPos, GFXFF);
    textyPos += _tft.fontHeight(GFXFF);

    dtostrf(actualRPM, 4, 0, floatString);
    sprintf(text, "%s RPM", floatString);
    _fb.drawString(text, (_fb.width()/2) - (_fb.textWidth(text)/2), textyPos, GFXFF);


    _fb.pushSprite(0, scale_box_yPos + scale_box_height + _tft.fontHeight(GFXFF) / 2);
    _fb.deleteSprite();
   

    trianglexPos = map(actDisplayData.differencekPaValue, 0, actDisplayData.gaugeScaleFactor * 8, 0, _tft.width()/2);

    if (actDisplayData.lowerSideIndicator < 0) {
        trianglexPos = (_tft.width()/2) + trianglexPos;
    } else {
        trianglexPos = (_tft.width()/2) - trianglexPos;
    }

    if (abs(trianglexPos - actDisplayData.lastIndicatorXPos) > 2) {

        lowerSideIndicatorBoxHeight = 2 * (scale_box_height/3);
        _fb.createSprite(_tft.width(), lowerSideIndicatorBoxHeight);
        _fb.fillSprite(TFT_WHITE);
        _fb.setFreeFont(FM12);

        // now it's time for black on white ...
        _fb.setTextColor(TFT_BLACK, TFT_WHITE, true);
        // _tft.setTextColor(TFT_WHITE, TFT_BLACK, true);

        sprintf(text, "x%1d", actDisplayData.gaugeScaleFactor);
        _fb.drawString(text, (_fb.width()-1)- (_fb.textWidth(text)) , _fb.height() - _fb.fontHeight(GFXFF) -1, GFXFF);

        sprintf(text, "%1d/sec", measuresPerSec);
        _fb.drawString(text, 1, _fb.height() - _fb.fontHeight(GFXFF) -1, GFXFF);


        actDisplayData.lastIndicatorXPos = trianglexPos;
        _fb.fillTriangle(  trianglexPos, 2, 
                           trianglexPos + 10, lowerSideIndicatorBoxHeight -2, 
                           trianglexPos - 10, lowerSideIndicatorBoxHeight -2, TFT_BLACK);

        _fb.pushSprite(0, scale_box_yPos + (scale_box_height/3) + 1);
        _fb.deleteSprite();

        
    }


};
