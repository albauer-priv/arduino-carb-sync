#include "CarbSyncDisplay.h"
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header


/*
CarbSyncDisplay::CarbSyncDisplay() {
    this->_LCDCols = 0;
    this->_LCDRows = 0;

}
*/


void CarbSyncDisplay::setup(int cols, int rows) {
  this->_LCDCols = cols;
  this->_LCDRows = rows;
 
  this->_lcd.begin(this->_LCDCols, this->_LCDRows);
  this->_lcd.backlight();
  this->_lcd.setCursor(0, 0);
  this->_lcd.clear();
  this->_lcd.leftToRight();;

  /*
  lcd.print("Hello, world!");
  lcd.setCursor(0, 1);
  lcd.print("It Works!"); 
  */
};

void CarbSyncDisplay::displaySplashScreen() {

};

void CarbSyncDisplay::displaySyncScreen() {
    this->_lcd.setCursor(1,0);
    this->_lcd.print("|--------|");

    this->_lcd.setCursor(0,1);
    this->_lcd.print("1");

    this->_lcd.setCursor(0,2);
    this->_lcd.print("2");

    this->_lcd.setCursor(0,3);
    this->_lcd.print("RPM");

    this->_lcd.setCursor(17,0);
    this->_lcd.print("kPa");

    this->_lcd.setCursor(17,1);
    this->_lcd.print("kPa");

    this->_lcd.setCursor(17,2);
    this->_lcd.print("kPa");

    this->_lcd.setCursor(17,3);
    this->_lcd.print("kPa");

    this->_lcd.setCursor(12,3);
    this->_lcd.print("d");

    this->_lcd.setCursor(12,0);
    this->_lcd.print("f");

};

void CarbSyncDisplay::updateSyncScreen(CylinderManifoldAbsolutePressureData data[], int sizeOfData) {
    int scale, mapValue[2], rpmValue, mapValueDifference;

    // this->_lcd.noDisplay();
    // this->_lcd.rightToLeft();
    for (int i=0; i<sizeOfData; i++) {
        this->_lcd.setCursor(13, i+1);

        mapValue[i] = data[i].getMinimumMAPValueAskPa();

        if (mapValue[i] < 100) {
          this->_lcd.print("  ");
        } else if (mapValue[i] < 1000) {
          this->_lcd.print(" ");
        }

        this->_lcd.print(mapValue[i]);
    }

    rpmValue = data[0].getSmoothedRPMValue();

    this->_lcd.setCursor(4, 3);
    if (rpmValue < 100) {
        this->_lcd.print("  ");
    } else if (rpmValue < 1000) {
        this->_lcd.print(" ");
    }
    this->_lcd.print(rpmValue);

    mapValueDifference = abs(mapValue[0] - mapValue[1]);
    this->_lcd.setCursor(13, 3);
    if (mapValueDifference < 10) {
        this->_lcd.print("   ");
    } else if (mapValueDifference < 100) {
        this->_lcd.print("  ");
    } else if (mapValueDifference < 1000) {
        this->_lcd.print(" ");
    }
    this->_lcd.print(mapValueDifference);

    scale = 100;
        this->_lcd.setCursor(13, 0);
    if (mapValueDifference <= 10) {
        scale = 1;
        this->_lcd.print("   ");
    } else if (mapValueDifference <= 20) {
        scale = 2;
        this->_lcd.print("  ");
    } else if (mapValueDifference <= 50) {
        scale = 5;
        this->_lcd.print("  ");
    } else if (mapValueDifference <= 100) {
        scale = 10;
        this->_lcd.print(" ");
    } else if (mapValueDifference <= 200) {
        scale = 20;
        this->_lcd.print(" ");
    } else if (mapValueDifference <= 500) {
        scale = 50;
        this->_lcd.print(" ");
    } else if (mapValueDifference <= 1000) {
        scale = 100;
        // this->_lcd.print(" ");
    }
    this->_lcd.print(scale);

    // this->_lcd.display();
};

