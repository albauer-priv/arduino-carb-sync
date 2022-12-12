#include "CylinderManifoldAbsolutePressureData.h"
#include "CarbSyncDisplayLCD.h"
// #include <ArduinoLog.h>


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



#define ALPHA_SMOOTHING_RPM_VALUE 0.1
#define ALPHA_SMOOTHING_ADC_VALUE 0.1

// resolution of ADC. Arduino typically 1024, esp8266 or esp32 4096
#define ADC_RESOLUTION 1024


#define DISPLAY_UPDATE_INTERVALL_MS 500

#define SERIAL_UPDATE_INTERVALL_MS 1000
#define MEASURE_UPDATE_INTERVALL_MS 1000


#define NUMBER_OF_CYLINDERS 2

CylinderManifoldAbsolutePressureData cylinders[NUMBER_OF_CYLINDERS];
CarbSyncDisplayLCD display;

uint8_t sensorPins[] = {A1, A3};

unsigned long lastTimeStampDisplayUpdated = 0;
unsigned long lastTimeStampSerialUpdated = 0;
unsigned long lastTimeStampMeasure = 0;
unsigned long actTimeStamp = 0;
unsigned long measures = 0;


void setup() {
  int maxSensorADCValueCalibration = 0;
  int cylinderSmoothedADCValue[NUMBER_OF_CYLINDERS];

  for (int i=0; i<NUMBER_OF_CYLINDERS; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  Serial.begin(115200);
  while(!Serial && !Serial.available()){}
  randomSeed(analogRead(0));

  for (int i=0; i<NUMBER_OF_CYLINDERS; i++) {
    cylinders[i].setBoardCharacteristics(ADC_RESOLUTION, MAP_SENSOR_REFERENCE_MV);
    cylinders[i].setMAPSensorCharacteristics(MAP_SENSOR_MIN_MV, MAP_SENSOR_MAX_MV, MAP_SENSOR_MIN_KPA, MAP_SENSOR_MAX_KPA);
    cylinders[i].setMAPSensorOffset(0);
    cylinders[i].setSmoothingAlphaADC(ALPHA_SMOOTHING_ADC_VALUE);
    cylinders[i].setSmoothingAlphaRPM(ALPHA_SMOOTHING_RPM_VALUE);
  }


  // writeLogHeaderToSerial();

  // display.setup(LCD_COLS, LCD_ROWS);
  display.setup();
  display.displaySyncScreen();

  // "calibrate" sensors ...
  for (int i=0; i<500; i++) {
    for (int j=0; j < NUMBER_OF_CYLINDERS; j++) {
      cylinders[j].setADCValue(analogRead(sensorPins[j]));
    }
  }


  for (int i=0; i < NUMBER_OF_CYLINDERS; i++) {
    cylinderSmoothedADCValue[i] = round(cylinders[i].getSmoothedADCValue());

    if (cylinderSmoothedADCValue[i] > maxSensorADCValueCalibration) {
      maxSensorADCValueCalibration = cylinderSmoothedADCValue[i];
    }
  }

  for (int i=0; i < NUMBER_OF_CYLINDERS; i++) {
    cylinders[i].resetMeasures();
  }

  for (int i=0; i < NUMBER_OF_CYLINDERS; i++) {
    int cylinderADCValueOffset = maxSensorADCValueCalibration - cylinderSmoothedADCValue[i];
    cylinders[i].setMAPSensorOffset(cylinderADCValueOffset);
  }

}


void loop() {

  for (int i=0; i< NUMBER_OF_CYLINDERS; i++) {
    cylinders[i].setADCValue(analogRead(sensorPins[i]));
  }

  measures++;

  actTimeStamp = millis();

  if ((actTimeStamp - lastTimeStampDisplayUpdated) > DISPLAY_UPDATE_INTERVALL_MS) {
    display.updateSyncScreen(cylinders, 2);
    lastTimeStampDisplayUpdated = actTimeStamp;
  }

  if ((actTimeStamp - lastTimeStampSerialUpdated) > SERIAL_UPDATE_INTERVALL_MS) {
    // writeLogDataToSerial();
    lastTimeStampSerialUpdated = actTimeStamp;
  }

  if ((actTimeStamp - lastTimeStampMeasure) > MEASURE_UPDATE_INTERVALL_MS) {
    // Serial.print("measure per sec.: ");
    // Serial.println(measures);
    measures = 0;
    lastTimeStampMeasure = actTimeStamp;
  }

 }



void writeLogHeaderToSerial() {
  Serial.print("timestamp;rpm");
    for (int i=0; i < NUMBER_OF_CYLINDERS; i++) {
      Serial.print(";ADC_");
      Serial.print(i);
      Serial.print(";minADC_");
      Serial.print(i);
      Serial.print(";smoothedMinADC_");
      Serial.print(i);
      Serial.print(";minMAPkPa_");
      Serial.print(i);
      Serial.print(";smoothedMinMAPkPa_");
      Serial.print(i);
    }
  Serial.println(";");
}

void writeLogDataToSerial() {
  Serial.print(millis());
  Serial.print(";");

  Serial.print(cylinders[0].getActualRPMValue());
  Serial.print(";");

  for (int i=0; i < NUMBER_OF_CYLINDERS; i++) {
    Serial.print(cylinders[i].getActualADCValue());
    Serial.print(";");

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