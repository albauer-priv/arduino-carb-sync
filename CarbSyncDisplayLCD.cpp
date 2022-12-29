#include "CarbSyncDisplayLCD.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CarbSyncDisplayLCD::CarbSyncDisplayLCD() {
    _tft.init();
    _tft.setRotation(1);
    _tft.setFreeFont(FM12);
    _tft.fillScreen(TFT_BLACK);
}


void CarbSyncDisplayLCD::setup() {

    actualScreen = -1;

    this->actDisplayData.differenceADCValue = 0.0;
    this->actDisplayData.differencekPaValue = 0.0;
    this->actDisplayData.minSmoothedkPaValue[0] = 0.0;
    this->actDisplayData.minSmoothedADCValue[0] = 0.0;
    this->actDisplayData.minSmoothedkPaValue[1] = 0.0;
    this->actDisplayData.minSmoothedADCValue[1] = 0.0;
    this->actDisplayData.smoothedkPaValue[0] = 0.0;
    this->actDisplayData.smoothedkPaValue[1] = 0.0;
    this->actDisplayData.maxSmoothedkPaValue[0] = 0.0;
    this->actDisplayData.maxSmoothedkPaValue[1] = 0.0;
    this->actDisplayData.minkPaValue[0] = 0.0;
    this->actDisplayData.minkPaValue[1] = 0.0;
    this->actDisplayData.lowerSideIndicator = 0;
    this->actDisplayData.gaugeScaleFactor = 1;
    this->actDisplayData.lastIndicatorXPos = 50;



    // Populate the palette table, table must have 16 entries
    _colorpalette[0]  = TFT_BLACK;
    _colorpalette[1]  = TFT_ORANGE;
    _colorpalette[2]  = TFT_DARKGREEN;
    _colorpalette[3]  = TFT_DARKCYAN;
    _colorpalette[4]  = TFT_MAROON;
    _colorpalette[5]  = TFT_PURPLE;
    _colorpalette[6]  = TFT_OLIVE;
    _colorpalette[7]  = TFT_DARKGREY;
    _colorpalette[8]  = TFT_ORANGE;
    _colorpalette[9]  = TFT_BLUE;
    _colorpalette[10] = TFT_GREEN;
    _colorpalette[11] = TFT_CYAN;
    _colorpalette[12] = TFT_RED;
    _colorpalette[13] = TFT_NAVY;
    _colorpalette[14] = TFT_YELLOW;
    _colorpalette[15] = TFT_WHITE; 

    
};


void CarbSyncDisplayLCD::_updateInternalData(CylinderManifoldAbsolutePressureData data[], int sizeOfData) {
    int scaleMultiply[] = {1, 10, 100, 1000};
    int scaleBase[] = {1, 2, 5};
    int i, j;
    bool exit = false;

    this->actDisplayData.minkPaValue[0] = data[0].getMinimumMAPValueAskPa();
    this->actDisplayData.minkPaValue[1] = data[1].getMinimumMAPValueAskPa();
    this->actDisplayData.minSmoothedkPaValue[0] = data[0].getSmoothedMinimumMAPValueAskPa();
    this->actDisplayData.minSmoothedkPaValue[1] = data[1].getSmoothedMinimumMAPValueAskPa();
    this->actDisplayData.minSmoothedADCValue[0] = data[0].getSmoothedMinimumADCValue();
    this->actDisplayData.minSmoothedADCValue[1] = data[1].getSmoothedMinimumADCValue();
    this->actDisplayData.differenceADCValue = abs(this->actDisplayData.minSmoothedADCValue[0] - this->actDisplayData.minSmoothedADCValue[1]);
    this->actDisplayData.differencekPaValue = abs(this->actDisplayData.minSmoothedkPaValue[0] - this->actDisplayData.minSmoothedkPaValue[1]);

    this->actDisplayData.smoothedkPaValue[0] = data[0].getSmoothedMAPValueAskPa();
    this->actDisplayData.smoothedkPaValue[1] = data[1].getSmoothedMAPValueAskPa();

    this->actDisplayData.maxSmoothedkPaValue[0] = data[0].getSmoothedMaximumMAPValueAskPa();
    this->actDisplayData.maxSmoothedkPaValue[1] = data[1].getSmoothedMaximumMAPValueAskPa();


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
};


void CarbSyncDisplayLCD::toggleScreen() {

    switch (actualScreen) {
        case 0 : 
                destroyAbsolutePressureBarScreen();
                break;
        case 1 : 
                destroySyncBarScreen();
                break;
        case 2 : 
                destroyMinMaxPressureBarScreen();
                break;
        case 3 : 
                destroySyncScreen();
                break;
        default : 
                break;

    }

    actualScreen++;
    actualScreen = actualScreen % 4;

    // Serial.print("screen: ");
    // Serial.print(actualScreen);

    switch (actualScreen) {
        case 0 : 
                setupAbsolutePressureBarScreen();
                break;
        case 1 : 
                setupSyncBarScreen();
                break;
        case 2 : 
                setupMinMaxPressureBarScreen();
                break;
        case 3 : 
                setupSyncScreen();
                break;
        default : 
                break;

    }

}


void CarbSyncDisplayLCD::updateScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int additionalData) {
    uint16_t touchxPos = 0;
    uint16_t touchyPos = 0;
    bool touched = false;


    switch (actualScreen) {
        case 0 : 
                // updateCalibrationScreen(data, sizeOfData, additionalData);
                updateAbsolutePressureBarScreen(data, sizeOfData, additionalData);
                break;
        case 1 : 
                updateSyncBarScreen(data, sizeOfData, additionalData);
                break;
        case 2 : 
                updateMinMaxPressureBarScreen(data, sizeOfData, additionalData);
                break;
        case 3 : 
                updateSyncScreen(data, sizeOfData, additionalData);
                break;
    }

    calledTimes++;
    if (calledTimes % 10 == 0) {
        touched = _tft.getTouch(&touchxPos, &touchyPos);

        if (touched) {
            /*
            Serial.print("x: ");
            Serial.print(touchxPos);
            Serial.print(" y: ");
            Serial.print(touchyPos);
            Serial.println();
            */
            touched = false;
            toggleScreen();
        }
    }
    
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CarbSyncDisplayLCD::setupCalibrationScreen() {

    // CarbSyncDisplayLCD::setup();

    _tft.setFreeFont(FM12);
    _tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
    _tft.fillScreen(TFT_WHITE);

    _fb1.createSprite(_tft.width(), 4 * _tft.fontHeight(GFXFF));
    _fb1.setColorDepth(4);
    _fb1.createPalette(_colorpalette);
    _fb1.fillSprite(15);
    // _fb.fillSprite(TFT_WHITE);
    _fb1.setFreeFont(FM12);
    _fb1.setTextColor(TFT_BLACK, TFT_WHITE, true);

};


void CarbSyncDisplayLCD::destroyCalibrationScreen() {
    _fb1.deleteSprite();
};


void CarbSyncDisplayLCD::updateCalibrationScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int additionalData) {
    String headerStr = "Kalibrierung ...";
    int textyPos = 12;
    int textyPosSprite = 0;
    char text[256];
    char floatString[10];
    char floatString2[10];

    _updateInternalData(data, sizeOfData);

    _fb1.fillSprite(15);


    _tft.drawString(headerStr, (_tft.width()/2) - (_tft.textWidth(headerStr)/2), textyPos, GFXFF);
    textyPos += _tft.fontHeight(GFXFF);
    textyPos += _tft.fontHeight(GFXFF);

    sprintf(text, "Messungen: %d ", additionalData);
    _tft.drawString(text, 1, textyPos, GFXFF);
    textyPos += _tft.fontHeight(GFXFF);
    textyPos += _tft.fontHeight(GFXFF);


    for (int i=0; i < sizeOfData; i++) {
        dtostrf(actDisplayData.minSmoothedADCValue[i], 4, 1, floatString2);
        sprintf(text, "Sensor %d - %s ADC", i, floatString2);
        _fb1.drawString(text, 1, textyPosSprite, GFXFF);
        textyPosSprite += _fb1.fontHeight(GFXFF);
    }

    textyPosSprite += _fb1.fontHeight(GFXFF);
    dtostrf(actDisplayData.differenceADCValue, 4, 1, floatString2);
    sprintf(text, "Offset - %s ADC", floatString2);
    _fb1.drawString(text, 1, textyPosSprite, GFXFF);

    _fb1.pushSprite(0, textyPos);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CarbSyncDisplayLCD::setupSyncScreen() {
    // setup();

    _tft.fillScreen(TFT_BLACK);
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


    _fb1.createSprite(_tft.width(), 6 * _tft.fontHeight(GFXFF));
    _fb1.setColorDepth(4);
    _fb1.createPalette(_colorpalette);
    _fb1.fillSprite(TFT_BLACK);
    _fb1.setFreeFont(FM12);
    _fb1.setTextColor(TFT_WHITE, TFT_BLACK, true);

    _fb2.createSprite(_tft.width(), 2 * (scale_box_height/3));
    _fb2.setColorDepth(4);
    _fb2.createPalette(_colorpalette);
    _fb2.fillSprite(TFT_WHITE);
    _fb2.setFreeFont(FM12);
    _fb2.setTextColor(TFT_BLACK, TFT_WHITE, true);

    _displaySyncData(0, 0);
};


void CarbSyncDisplayLCD::destroySyncScreen() {
    _fb1.deleteSprite();
    _fb2.deleteSprite();
};


void CarbSyncDisplayLCD::updateSyncScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int additionalData) {
    _updateInternalData(data, sizeOfData);
    _displaySyncData(additionalData, data[0].getSmoothedRPMValue());
};


void CarbSyncDisplayLCD::_displaySyncData(int measuresPerSec, int actualRPM) {
    char text[256];
    char floatString[10];
    char floatString2[10];
    char lowerSideIndicatorChar = ' ';
    int trianglexPos;
    int textyPos = 0;
    int floatPrecision = 0;
    int lowerSideIndicatorBoxHeight = 0;


    _fb1.fillSprite(0);
    _fb2.fillSprite(15);


    // now it's time for white on black ...

    if (actDisplayData.lowerSideIndicator < 0) {
        lowerSideIndicatorChar = '<';
    } else if (actDisplayData.lowerSideIndicator > 0) {
        lowerSideIndicatorChar = '>';
    } else {
        lowerSideIndicatorChar = '=';
    }

    sprintf(text, "1 %c 2", lowerSideIndicatorChar);
    // _fb.drawString(text, (_tft.width()/2) - (_tft.textWidth(text)/2), scale_box_yPos + scale_box_height + 0*_tft.fontHeight(GFXFF), GFXFF);
    _fb1.drawString(text, (_fb1.width()/2) - (_fb1.textWidth(text)/2), textyPos, GFXFF);
    textyPos += _fb1.fontHeight(GFXFF);


    if (actDisplayData.differencekPaValue > 10) {
        floatPrecision = 0;
    } else {
        floatPrecision = 1;
    }
    dtostrf(actDisplayData.differencekPaValue, 3, floatPrecision, floatString);
    sprintf(text, "d %s kPa", floatString);
    _fb1.drawString(text, (_fb1.width()/2) - (_fb1.textWidth(text)/2), textyPos, GFXFF);
    textyPos += _fb1.fontHeight(GFXFF);

    if (actDisplayData.differenceADCValue > 100) {
        floatPrecision = 0;
    } else {
        floatPrecision = 1;
    }
    dtostrf(actDisplayData.differenceADCValue, 4, floatPrecision, floatString);
    sprintf(text, "d %s ADC", floatString);
    _fb1.drawString(text, (_fb1.width()/2)- (_fb1.textWidth(text)/2), textyPos, GFXFF);
    textyPos += _fb1.fontHeight(GFXFF);

    textyPos += _fb1.fontHeight(GFXFF);

    dtostrf(actDisplayData.minkPaValue[0], 4, 1, floatString);
    dtostrf(actDisplayData.minkPaValue[1], 4, 1, floatString2);
    sprintf(text, "%s kPa %s", floatString, floatString2);
    _fb1.drawString(text, (_fb1.width()/2) - (_fb1.textWidth(text)/2), textyPos, GFXFF);
    textyPos += _fb1.fontHeight(GFXFF);

    dtostrf(actualRPM, 4, 0, floatString);
    sprintf(text, "%s RPM", floatString);
    _fb1.drawString(text, (_fb1.width()/2) - (_fb1.textWidth(text)/2), textyPos, GFXFF);


    _fb1.pushSprite(0, scale_box_yPos + scale_box_height + _tft.fontHeight(GFXFF) );
    // _fb.deleteSprite();

    trianglexPos = map(actDisplayData.differencekPaValue, 0, actDisplayData.gaugeScaleFactor * 8, 0, _tft.width()/2);

    if (actDisplayData.lowerSideIndicator < 0) {
        trianglexPos = (_tft.width()/2) + trianglexPos;
    } else {
        trianglexPos = (_tft.width()/2) - trianglexPos;
    }


    lowerSideIndicatorBoxHeight = 2 * (scale_box_height/3);


    // now it's time for black on white ...
    // _tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    _fb2.fillRect(0, 0, _fb2.width()-1, _fb2.height()-1, TFT_WHITE);


    sprintf(text, "x%1d", actDisplayData.gaugeScaleFactor);
    _fb2.drawString(text, (_fb2.width()-1)- (_fb2.textWidth(text)) -3 , _fb2.height() - _fb2.fontHeight(GFXFF) -1, GFXFF);

    sprintf(text, "%1d/sec", measuresPerSec);
    _fb2.drawString(text, 2, _fb2.height() - _fb2.fontHeight(GFXFF) -1, GFXFF);


    actDisplayData.lastIndicatorXPos = trianglexPos;
    _fb2.fillTriangle(  trianglexPos, 2, 
                       trianglexPos + 10, lowerSideIndicatorBoxHeight -2, 
                       trianglexPos - 10, lowerSideIndicatorBoxHeight -2, TFT_BLACK);

    _fb2.pushSprite(0, scale_box_yPos + (scale_box_height/3) + 1);
    // _fb.deleteSprite();

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CarbSyncDisplayLCD::_drawDashedLine(int xs, int ys, int width, int height, int fgcolor, int bgcolor) {

    int dashes = height/4;

    _tft.startWrite();

    // Quick way to draw a dashed line
    _tft.setAddrWindow(xs, ys, width, height);
    
    for(int i = 0; i < height/4 ; i+=2) {
        _tft.pushColor(fgcolor, width*4); // push dash pixels
        _tft.pushColor(bgcolor, width*4); // push gap pixels
    }

    _tft.endWrite();
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CarbSyncDisplayLCD::setupSyncBarScreen() {
    // setup();
    String headerStr = "Differenzdruck";
    int textyPos = 0;

    _tft.fillScreen(TFT_WHITE);
    _tft.setTextColor(TFT_BLACK, TFT_WHITE, true);

    _tft.drawString(headerStr, (_tft.width()/2) - (_tft.textWidth(headerStr)/2), textyPos, GFXFF);
    textyPos += _tft.fontHeight(GFXFF) + 1;

    _tft.drawLine(0, textyPos, _tft.width() - 1, textyPos, TFT_BLACK);
    textyPos += _tft.fontHeight(GFXFF) / 2;

    // _tft.setTextFont(FM9);
    textyPos += _tft.fontHeight(GFXFF);

    /*
    Serial.print("Schrifthöhe px: ");
    Serial.println(_tft.fontHeight(GFXFF));

    Serial.print("akt y Position: ");
    Serial.println(textyPos);
    */


    int pBoxVerticalSpacing = 15;
    int pBoxHeight = 48;
    int pBox1yPos = textyPos + pBoxVerticalSpacing;
    int pBox2yPos = pBox1yPos + pBoxHeight + pBoxVerticalSpacing;

    int canvasHeight = 2 * pBoxHeight + (3 * pBoxVerticalSpacing);// _tft.height()- _tft.fontHeight(GFXFF) - textyPos - 6;

    // tick marks 
    for (int i=1; i<_tft.width(); i++) {
        if ((i % 20) == 0) {
            _drawDashedLine(i, textyPos, 1, canvasHeight, TFT_BLACK, TFT_WHITE);
        }
    }


    /*
    Serial.print("Zeichenfläche Höhe px: ");
    Serial.println(canvasHeight);
    */


    _tft.drawRect(0, pBox1yPos, _tft.width(), pBoxHeight, TFT_BLACK);
    _tft.drawRect(0, pBox2yPos, _tft.width(), pBoxHeight, TFT_BLACK);

    _tft.fillRect(1, pBox1yPos+1, _tft.width() - 2, pBoxHeight-2, TFT_LIGHTGREY);
    _tft.fillRect(1, pBox2yPos+1, _tft.width() - 2, pBoxHeight-2, TFT_LIGHTGREY);

    _tft.fillRect(3, pBox1yPos+3, _tft.width() - 6, pBoxHeight-6, TFT_WHITE);
    _tft.fillRect(3, pBox2yPos+3, _tft.width() - 6, pBoxHeight-6, TFT_WHITE);

    // middle red tick
    _tft.fillRect((_tft.width()/2)-1, textyPos, 3, canvasHeight, TFT_RED);

    _fb1PushCoords.x = 3;
    _fb1PushCoords.y = pBox1yPos+3;

    _fb2PushCoords.x = 3;
    _fb2PushCoords.y = pBox2yPos+3;

    _fb3PushCoords.x = 0;
    _fb3PushCoords.y = textyPos + canvasHeight;

    _fb4PushCoords.x = 0;
    _fb4PushCoords.y = textyPos - _tft.fontHeight(GFXFF);

    _fb1.createSprite(_tft.width() - 6, pBoxHeight-6);
    _fb1.setColorDepth(4);
    _fb1.createPalette(_colorpalette);
    _fb1.fillSprite(15);
    _fb1.setFreeFont(FM12);
    _fb1.setTextColor(TFT_BLACK, TFT_WHITE, false);

    _fb2.createSprite(_tft.width() - 6, pBoxHeight-6);
    _fb2.setColorDepth(4);
    _fb2.createPalette(_colorpalette);
    _fb2.fillSprite(15);
    _fb2.setFreeFont(FM12);
    _fb2.setTextColor(TFT_BLACK, TFT_WHITE, false);

    _fb3.createSprite(_tft.width(), 2*  _tft.fontHeight(GFXFF));
    _fb3.setColorDepth(4);
    _fb3.createPalette(_colorpalette);
    _fb3.fillSprite(15);
    _fb3.setFreeFont(FM12);
    _fb3.setTextColor(TFT_BLACK, TFT_WHITE, false);

    _fb4.createSprite(_tft.width(), 1*  _tft.fontHeight(GFXFF));
    _fb4.setColorDepth(4);
    _fb4.createPalette(_colorpalette);
    _fb4.fillSprite(15);
    _fb4.setFreeFont(FM12);
    _fb4.setTextColor(TFT_BLACK, TFT_WHITE, false);

};


void CarbSyncDisplayLCD::destroySyncBarScreen() {
    _fb1.deleteSprite();
    _fb2.deleteSprite();
    _fb3.deleteSprite();
    _fb4.deleteSprite();
};


void CarbSyncDisplayLCD::updateSyncBarScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int additionalData) {
    char text[256];
    char floatString[10];
    int textyPos = 1;
    int textxPos = 0;
    int floatPrecision = 0;
    float pLowValue = 120.0;
    float pHighValue = 0.0;
    float pAVGValue = 0.0;
    float pRange = 0.0;
    long barValue[2];


    _updateInternalData(data, sizeOfData);

    _fb1.fillSprite(15);
    _fb2.fillSprite(15);
    _fb3.fillSprite(15);
    _fb4.fillSprite(15);


    for (int i=0; i<2; i++) {
        pLowValue = min(pLowValue, actDisplayData.minSmoothedkPaValue[i]);
        pHighValue = max(pHighValue, actDisplayData.minSmoothedkPaValue[i]);
        pAVGValue += actDisplayData.minSmoothedkPaValue[i];
    }
    pAVGValue = pAVGValue / 2;
    pRange = pHighValue - pLowValue;

    for (int i=0; i<2; i++) {
        barValue[i] = map(round((actDisplayData.minSmoothedkPaValue[i] * 10) - (pAVGValue * 10)), 0, actDisplayData.gaugeScaleFactor * 80, 0, _tft.width()/2);
    }


    /*
    Serial.println("-------------------------------");
    Serial.print("p AVGValue: ");
    Serial.println(pAVGValue);
    Serial.print("p Range: ");
    Serial.println(pRange);
    Serial.print("p MinValue 0: ");
    Serial.println(actDisplayData.minSmoothedkPaValue[0]);
    Serial.print("p MinValue 1: ");
    Serial.println(actDisplayData.minSmoothedkPaValue[1]);
    Serial.print("BarValue 0: ");
    Serial.println(barValue[0]);
    Serial.print("BarValue 1: ");
    Serial.println(barValue[1]);
    */


    if ( (abs(barValue[0]) <= 3) || (abs(barValue[1]) <= 3)) {
        _fb1.fillRect((_fb1.width()/2)-1, 0, 3, _fb1.height(), TFT_RED);
        _fb2.fillRect((_fb2.width()/2)-1, 0, 3, _fb2.height(), TFT_RED);

    } else {
        if (barValue[0] >= 0) {
            _fb1.fillRect(_fb1.width()/2-1, 0, barValue[0], _fb1.height(), 0);
        } else {
            _fb1.fillRect(_fb1.width()/2+barValue[0], 0, abs(barValue[0]), _fb1.height(), 0);
        }

        if (barValue[1] >= 0) {
            _fb2.fillRect(_fb2.width()/2-1, 0, barValue[1], _fb2.height(), 0);
        } else {
            _fb2.fillRect(_fb2.width()/2+barValue[1], 0, abs(barValue[1]), _fb2.height(), 0);
        }
    }



    if (data[0].getSmoothedRPMValue() > 0) {
        _fb1.setTextDatum(ML_DATUM);
        _fb2.setTextDatum(ML_DATUM);

        dtostrf(actDisplayData.minSmoothedkPaValue[0] - pAVGValue, 3, 1, floatString);
        sprintf(text, "1 %s", floatString);
        _fb1.drawString(text, 0, _fb1.height()/2, GFXFF);

        dtostrf(actDisplayData.minSmoothedkPaValue[1] - pAVGValue, 3, 1, floatString);
        sprintf(text, "2 %s", floatString);
        _fb2.drawString(text, 0, _fb2.height()/2, GFXFF);

        // _fb1.drawFloat(actDisplayData.minSmoothedkPaValue[0] - pAVGValue, 1, 0, _fb1.height()/2); 
        // _fb2.drawFloat(actDisplayData.minSmoothedkPaValue[1] - pAVGValue, 1, 0, _fb2.height()/2); 
    }


    if (actDisplayData.differencekPaValue > 10) {
        floatPrecision = 0;
    } else {
        floatPrecision = 1;
    }
    _fb3.setTextDatum(TL_DATUM);
    dtostrf(actDisplayData.differencekPaValue, 3, floatPrecision, floatString);
    sprintf(text, "d %s kPa", floatString);
    _fb3.drawString(text, 2, 0, GFXFF);
    // textyPos += _fb3.fontHeight(GFXFF);

    _fb3.setTextDatum(TL_DATUM);
    dtostrf(data[0].getSmoothedRPMValue(), 4, 0, floatString);
    sprintf(text, "%s RPM", floatString);
    _fb3.drawString(text, _fb3.width() - _fb3.textWidth(text) - 4, 0, GFXFF);


    _fb4.setTextDatum(BL_DATUM);
    sprintf(text, "x%1d", actDisplayData.gaugeScaleFactor);
    _fb4.drawString(text, _fb4.width() - _fb4.textWidth(text) - 4, _fb4.height(), GFXFF);

    _fb4.setTextDatum(BL_DATUM);
    sprintf(text, "%1d/sec", additionalData);
    _fb4.drawString(text, 2, _fb4.height(), GFXFF);



    _fb1.pushSprite(_fb1PushCoords.x, _fb1PushCoords.y);
    _fb2.pushSprite(_fb2PushCoords.x, _fb2PushCoords.y);
    _fb3.pushSprite(_fb3PushCoords.x, _fb3PushCoords.y);
    _fb4.pushSprite(_fb4PushCoords.x, _fb4PushCoords.y);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CarbSyncDisplayLCD::setupAbsolutePressureBarScreen() {
    // setup();
    String headerStr = "Absolutdruck";
    int textyPos = 0;

    _tft.fillScreen(TFT_WHITE);
    _tft.setTextColor(TFT_BLACK, TFT_WHITE, true);

    _tft.drawString(headerStr, (_tft.width()/2) - (_tft.textWidth(headerStr)/2), textyPos, GFXFF);
    textyPos += _tft.fontHeight(GFXFF) + 1;

    _tft.drawLine(0, textyPos, _tft.width() - 1, textyPos, TFT_BLACK);
    textyPos += _tft.fontHeight(GFXFF) / 2;

    textyPos += _tft.fontHeight(GFXFF);

    int pBoxVerticalSpacing = 15;
    int pBoxHeight = 48;
    int pBox1yPos = textyPos + pBoxVerticalSpacing;
    int pBox2yPos = pBox1yPos + pBoxHeight + pBoxVerticalSpacing;

    int canvasHeight = 2 * pBoxHeight + (3 * pBoxVerticalSpacing);// _tft.height()- _tft.fontHeight(GFXFF) - textyPos - 6;

    // tick marks 
    for (int i=1; i<_tft.width(); i++) {
        if ((i % 20) == 0) {
            _drawDashedLine(i, textyPos, 1, canvasHeight, TFT_BLACK, TFT_WHITE);
        }
    }


    _tft.drawRect(0, pBox1yPos, _tft.width(), pBoxHeight, TFT_BLACK);
    _tft.drawRect(0, pBox2yPos, _tft.width(), pBoxHeight, TFT_BLACK);

    _tft.fillRect(1, pBox1yPos+1, _tft.width() - 2, pBoxHeight-2, TFT_LIGHTGREY);
    _tft.fillRect(1, pBox2yPos+1, _tft.width() - 2, pBoxHeight-2, TFT_LIGHTGREY);

    _tft.fillRect(3, pBox1yPos+3, _tft.width() - 6, pBoxHeight-6, TFT_WHITE);
    _tft.fillRect(3, pBox2yPos+3, _tft.width() - 6, pBoxHeight-6, TFT_WHITE);


    _fb1PushCoords.x = 3;
    _fb1PushCoords.y = pBox1yPos+3;

    _fb2PushCoords.x = 3;
    _fb2PushCoords.y = pBox2yPos+3;

    _fb3PushCoords.x = 0;
    _fb3PushCoords.y = textyPos + canvasHeight;

    _fb4PushCoords.x = 0;
    _fb4PushCoords.y = textyPos - _tft.fontHeight(GFXFF);

    _fb1.createSprite(_tft.width() - 6, pBoxHeight-6);
    _fb1.setColorDepth(4);
    _fb1.createPalette(_colorpalette);
    _fb1.fillSprite(15);
    _fb1.setFreeFont(FM12);
    _fb1.setTextColor(TFT_WHITE, TFT_BLACK, false);

    _fb2.createSprite(_tft.width() - 6, pBoxHeight-6);
    _fb2.setColorDepth(4);
    _fb2.createPalette(_colorpalette);
    _fb2.fillSprite(15);
    _fb2.setFreeFont(FM12);
    _fb2.setTextColor(TFT_WHITE, TFT_BLACK, false);

    _fb3.createSprite(_tft.width(), 2*  _tft.fontHeight(GFXFF));
    _fb3.setColorDepth(4);
    _fb3.createPalette(_colorpalette);
    _fb3.fillSprite(15);
    _fb3.setFreeFont(FM12);
    _fb3.setTextColor(TFT_BLACK, TFT_WHITE, false);

    _fb4.createSprite(_tft.width(), 1*  _tft.fontHeight(GFXFF));
    _fb4.setColorDepth(4);
    _fb4.createPalette(_colorpalette);
    _fb4.fillSprite(15);
    _fb4.setFreeFont(FM12);
    _fb4.setTextColor(TFT_BLACK, TFT_WHITE, false);

};


void CarbSyncDisplayLCD::destroyAbsolutePressureBarScreen() {
    _fb1.deleteSprite();
    _fb2.deleteSprite();
    _fb3.deleteSprite();
    _fb4.deleteSprite();
};


void CarbSyncDisplayLCD::updateAbsolutePressureBarScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int additionalData) {
    char text[256];
    char floatString[10];
    int textyPos = 1;
    int textxPos = 0;
    int floatPrecision = 0;
    long barValue[2];


    _updateInternalData(data, sizeOfData);

    _fb1.fillSprite(15);
    _fb2.fillSprite(15);
    _fb3.fillSprite(15);
    _fb4.fillSprite(15);


    for (int i=0; i<2; i++) {
        barValue[i] = map(round(actDisplayData.minSmoothedkPaValue[i] * 10), 0, 1150, 0, _tft.width());
    }


    _fb1.fillRect(0, 0, barValue[0], _fb1.height(), 0);
    _fb2.fillRect(0, 0, barValue[1], _fb2.height(), 0);

    if (data[0].getSmoothedRPMValue() > 0) {
        _fb1.setTextDatum(ML_DATUM);
        _fb2.setTextDatum(ML_DATUM);

        dtostrf(actDisplayData.minSmoothedkPaValue[0], 3, 1, floatString);
        sprintf(text, "1 %s", floatString);
        _fb1.drawString(text, 0, _fb1.height()/2, GFXFF);

        dtostrf(actDisplayData.minSmoothedkPaValue[1], 3, 1, floatString);
        sprintf(text, "2 %s", floatString);
        _fb2.drawString(text, 0, _fb2.height()/2, GFXFF);

        // _fb1.drawFloat(actDisplayData.minSmoothedkPaValue[0], 1, 0, _fb1.height()/2); 
        // _fb2.drawFloat(actDisplayData.minSmoothedkPaValue[1], 1, 0, _fb2.height()/2); 
    }


    if (actDisplayData.differencekPaValue > 10) {
        floatPrecision = 0;
    } else {
        floatPrecision = 1;
    }
    _fb3.setTextDatum(TL_DATUM);
    dtostrf(actDisplayData.differencekPaValue, 3, floatPrecision, floatString);
    sprintf(text, "d %s kPa", floatString);
    _fb3.drawString(text, 2, 0, GFXFF);

    _fb3.setTextDatum(TL_DATUM);
    dtostrf(data[0].getSmoothedRPMValue(), 4, 0, floatString);
    sprintf(text, "%s RPM", floatString);
    _fb3.drawString(text, _fb3.width() - _fb3.textWidth(text) - 4, 0, GFXFF);

    _fb4.setTextDatum(BL_DATUM);
    sprintf(text, "%1d/sec", additionalData);
    _fb4.drawString(text, 2, _fb4.height(), GFXFF);

    _fb1.pushSprite(_fb1PushCoords.x, _fb1PushCoords.y);
    _fb2.pushSprite(_fb2PushCoords.x, _fb2PushCoords.y);
    _fb3.pushSprite(_fb3PushCoords.x, _fb3PushCoords.y);
    _fb4.pushSprite(_fb4PushCoords.x, _fb4PushCoords.y);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CarbSyncDisplayLCD::setupMinMaxPressureBarScreen() {
    // setup();
    String headerStr = "Min-Max-Anzeige";
    int textyPos = 0;

    _tft.fillScreen(TFT_WHITE);
    _tft.setTextColor(TFT_BLACK, TFT_WHITE, true);

    _tft.drawString(headerStr, (_tft.width()/2) - (_tft.textWidth(headerStr)/2), textyPos, GFXFF);
    textyPos += _tft.fontHeight(GFXFF) + 1;

    _tft.drawLine(0, textyPos, _tft.width() - 1, textyPos, TFT_BLACK);
    textyPos += _tft.fontHeight(GFXFF) / 2;

    textyPos += _tft.fontHeight(GFXFF);

    int pBoxVerticalSpacing = 15;
    int pBoxHeight = 48;
    int pBox1yPos = textyPos + pBoxVerticalSpacing;
    int pBox2yPos = pBox1yPos + pBoxHeight + pBoxVerticalSpacing;

    int canvasHeight = 2 * pBoxHeight + (3 * pBoxVerticalSpacing);// _tft.height()- _tft.fontHeight(GFXFF) - textyPos - 6;

    // tick marks 
    for (int i=1; i<_tft.width(); i++) {
        if ((i % 20) == 0) {
            _drawDashedLine(i, textyPos, 1, canvasHeight, TFT_BLACK, TFT_WHITE);
        }
    }


    _tft.drawRect(0, pBox1yPos, _tft.width(), pBoxHeight, TFT_BLACK);
    _tft.drawRect(0, pBox2yPos, _tft.width(), pBoxHeight, TFT_BLACK);

    _tft.fillRect(1, pBox1yPos+1, _tft.width() - 2, pBoxHeight-2, TFT_LIGHTGREY);
    _tft.fillRect(1, pBox2yPos+1, _tft.width() - 2, pBoxHeight-2, TFT_LIGHTGREY);

    _tft.fillRect(3, pBox1yPos+3, _tft.width() - 6, pBoxHeight-6, TFT_WHITE);
    _tft.fillRect(3, pBox2yPos+3, _tft.width() - 6, pBoxHeight-6, TFT_WHITE);


    _fb1PushCoords.x = 3;
    _fb1PushCoords.y = pBox1yPos+3;

    _fb2PushCoords.x = 3;
    _fb2PushCoords.y = pBox2yPos+3;

    _fb3PushCoords.x = 0;
    _fb3PushCoords.y = textyPos + canvasHeight;

    _fb4PushCoords.x = 0;
    _fb4PushCoords.y = textyPos - _tft.fontHeight(GFXFF);

    _fb1.createSprite(_tft.width() - 6, pBoxHeight-6);
    _fb1.setColorDepth(4);
    _fb1.createPalette(_colorpalette);
    _fb1.fillSprite(15);
    _fb1.setFreeFont(FM12);
    _fb1.setTextColor(TFT_WHITE, TFT_BLACK, false);

    _fb2.createSprite(_tft.width() - 6, pBoxHeight-6);
    _fb2.setColorDepth(4);
    _fb2.createPalette(_colorpalette);
    _fb2.fillSprite(15);
    _fb2.setFreeFont(FM12);
    _fb2.setTextColor(TFT_WHITE, TFT_BLACK, false);

    _fb3.createSprite(_tft.width(), 2*  _tft.fontHeight(GFXFF));
    _fb3.setColorDepth(4);
    _fb3.createPalette(_colorpalette);
    _fb3.fillSprite(15);
    _fb3.setFreeFont(FM12);
    _fb3.setTextColor(TFT_BLACK, TFT_WHITE, false);

    _fb4.createSprite(_tft.width(), 1*  _tft.fontHeight(GFXFF));
    _fb4.setColorDepth(4);
    _fb4.createPalette(_colorpalette);
    _fb4.fillSprite(15);
    _fb4.setFreeFont(FM12);
    _fb4.setTextColor(TFT_BLACK, TFT_WHITE, false);

};


void CarbSyncDisplayLCD::destroyMinMaxPressureBarScreen() {
    _fb1.deleteSprite();
    _fb2.deleteSprite();
    _fb3.deleteSprite();
    _fb4.deleteSprite();
};


void CarbSyncDisplayLCD::updateMinMaxPressureBarScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData, int additionalData) {
    char text[256];
    char floatString[10];
    int textyPos = 1;
    int textxPos = 0;
    int floatPrecision = 0;
    long lowBarValue[2];
    long highBarValue[2];


    _updateInternalData(data, sizeOfData);

    _fb1.fillSprite(15);
    _fb2.fillSprite(15);
    _fb3.fillSprite(15);
    _fb4.fillSprite(15);


    for (int i=0; i<2; i++) {
        lowBarValue[i] = map(round(actDisplayData.minSmoothedkPaValue[i] * 10), 0, 1150, 0, _tft.width());
        highBarValue[i] = map(round(actDisplayData.maxSmoothedkPaValue[i] * 10), 0, 1150, 0, _tft.width());
    }


    _fb1.fillRect(lowBarValue[0], 0, highBarValue[0] - lowBarValue[0], _fb1.height(), 0);
    _fb2.fillRect(lowBarValue[1], 0, highBarValue[1] - lowBarValue[1], _fb2.height(), 0);

    if (data[0].getSmoothedRPMValue() > 0) {
        _fb1.setTextDatum(ML_DATUM);
        _fb2.setTextDatum(ML_DATUM);

        dtostrf(highBarValue[0] - lowBarValue[0], 2, 0, floatString);
        sprintf(text, "1 %s", floatString);
        _fb1.drawString(text, 0, _fb1.height()/2, GFXFF);

        dtostrf(highBarValue[1] - lowBarValue[1], 2, 0, floatString);
        sprintf(text, "2 %s", floatString);
        _fb2.drawString(text, 0, _fb2.height()/2, GFXFF);

        // _fb1.drawFloat(actDisplayData.minSmoothedkPaValue[0], 1, 0, _fb1.height()/2); 
        // _fb2.drawFloat(actDisplayData.minSmoothedkPaValue[1], 1, 0, _fb2.height()/2); 
    }


    if (actDisplayData.differencekPaValue > 10) {
        floatPrecision = 0;
    } else {
        floatPrecision = 1;
    }
    _fb3.setTextDatum(TL_DATUM);
    dtostrf(actDisplayData.differencekPaValue, 3, floatPrecision, floatString);
    sprintf(text, "d %s kPa", floatString);
    _fb3.drawString(text, 2, 0, GFXFF);

    _fb3.setTextDatum(TL_DATUM);
    dtostrf(data[0].getSmoothedRPMValue(), 4, 0, floatString);
    sprintf(text, "%s RPM", floatString);
    _fb3.drawString(text, _fb3.width() - _fb3.textWidth(text) - 4, 0, GFXFF);

    _fb4.setTextDatum(BL_DATUM);
    sprintf(text, "%1d/sec", additionalData);
    _fb4.drawString(text, 2, _fb4.height(), GFXFF);

    _fb1.pushSprite(_fb1PushCoords.x, _fb1PushCoords.y);
    _fb2.pushSprite(_fb2PushCoords.x, _fb2PushCoords.y);
    _fb3.pushSprite(_fb3PushCoords.x, _fb3PushCoords.y);
    _fb4.pushSprite(_fb4PushCoords.x, _fb4PushCoords.y);
};
