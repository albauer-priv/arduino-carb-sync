#include "CylinderManifoldAbsolutePressureData.h"
#include "CarbSyncDisplayLCD.h"
// #include <ArduinoLog.h>


// LCD geometry
const int LCD_COLS = 20;
const int LCD_ROWS = 4;



/**
 * ERA 550492 MAP sensor or Bosch 0 261 230 289 or FACET 10.3195
 * The pressure sensor has the following specification at 5V supply;
 *   400 mV =  10 kPa
 * 4,650 mv = 115 kPa
 * 
*/
#define MAP_SENSOR_MIN_MV        400.0
#define MAP_SENSOR_MAX_MV        4650.0
#define MAP_SENSOR_MIN_KPA       10.0
#define MAP_SENSOR_MAX_KPA       115.0
#define MAP_SENSOR_REFERENCE_MV  5000.0



#define ALPHA_SMOOTHING_RPM_VALUE 0.03
#define ALPHA_SMOOTHING_ADC_VALUE 0.003

// resolution of ADC. Arduino typically 1024, esp8266 or esp32 4096
#define ADC_RESOLUTION 1024


#define DISPLAY_UPDATE_INTERVALL_MS 500

#define SERIAL_UPDATE_INTERVALL_MS 1000
#define MEASURE_UPDATE_INTERVALL_MS 1000


CylinderManifoldAbsolutePressureData cylinders[2];
CarbSyncDisplayLCD display;




// forward void writeLogHeaderToSerial();
// forward void writeLogDataTiSerial();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400);
  while(!Serial && !Serial.available()){}
  randomSeed(analogRead(0));

  for (int i=0; i<2; i++) {
    cylinders[i].setBoardCharacteristics(ADC_RESOLUTION, MAP_SENSOR_REFERENCE_MV);
    cylinders[i].setMAPSensorCharacteristics(MAP_SENSOR_MIN_MV, MAP_SENSOR_MAX_MV, MAP_SENSOR_MIN_KPA, MAP_SENSOR_MAX_KPA);
    cylinders[i].setMAPSensorOffset(0);
    cylinders[i].setSmoothingAlphaADC(ALPHA_SMOOTHING_ADC_VALUE);
    cylinders[i].setSmoothingAlphaRPM(ALPHA_SMOOTHING_RPM_VALUE);
  }



  writeLogHeaderToSerial();

  display.setup(LCD_COLS, LCD_ROWS);
  display.displaySyncScreen();

}

int ADCValue[2] = {0, 0};

unsigned long lastTimeStampDisplayUpdated = 0;
unsigned long lastTimeStampSerialUpdated = 0;
unsigned long lastTimeStampMeasure = 0;
unsigned long actTimeStamp = 0;
unsigned long measures = 0;

void loop() {

  ADCValue[0] = analogRead(A1);
  ADCValue[1] = analogRead(A3);
  measures++;

  cylinders[0].setADCValue(ADCValue[0]);
  cylinders[1].setADCValue(ADCValue[1]);

  actTimeStamp = millis();

  if ((actTimeStamp - lastTimeStampDisplayUpdated) > DISPLAY_UPDATE_INTERVALL_MS) {
    display.updateSyncScreen(cylinders, 2);
    lastTimeStampDisplayUpdated = actTimeStamp;
  }

  if ((actTimeStamp - lastTimeStampSerialUpdated) > SERIAL_UPDATE_INTERVALL_MS) {
    // writeLogDataToSerial();
    /*
    Serial.print("ADC: ");
    Serial.print(ADCValue[0]);
    Serial.print(" <-> ");
    Serial.print(ADCValue[1]);
    Serial.print(" MAP (kPa): ");
    Serial.print(cylinders[0].getMAPValueAskPa());
    Serial.print(" <-> ");
    Serial.print(cylinders[1].getMAPValueAskPa());
    Serial.println();
    */
    lastTimeStampSerialUpdated = actTimeStamp;
  }

  if ((actTimeStamp - lastTimeStampMeasure) > MEASURE_UPDATE_INTERVALL_MS) {
    Serial.print("measure per sec.: ");
    Serial.println(measures);
    measures = 0;
    lastTimeStampMeasure = actTimeStamp;
  }

 }



void writeLogHeaderToSerial() {
  Serial.println("timestamp;rpm;adc1;smoothedminadc1;minmap1kPa;smoothedminmap1kPa;adc2;smothedminadc2;minmap2kPa;smoothedminmap2kPa;");
}

void writeLogDataToSerial() {
  Serial.print(millis());
  Serial.print(";");

  Serial.print(cylinders[0].getActualRPMValue());
  Serial.print(";");

  for (int i=0; i<2; i++) {
    Serial.print(cylinders[i].getMinimumADCValue());
    Serial.print(";");

    Serial.print(cylinders[i].getSmoothedMinimumADCValue());
    Serial.print(";");

    Serial.print(cylinders[i].getMinimumMAPValueAskPa());
    Serial.print(";");

    Serial.print(cylinders[i].getSmoothedMinimumMAPValueAskPa());
    Serial.print(";");

  }
  Serial.println();
}