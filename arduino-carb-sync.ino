#include "CylinderManifoldAbsolutePressureData.h"
#include "CarbSyncDisplayLCD.h"

// #include <ArduinoLog.h>


/**
 * ERA 550492 MAP sensor or Bosch 0 261 230 289 or FACET 10.3195
 * The pressure sensor has the following specification at 5V supply;
 *   400 mV =  10 kPa
 * 4,650 mv = 115 kPa
 * 
 * we use a potential/ voltage divider (1:2 ratio), so the numbers we can read are divided by 3 and multiplied by 2
 * 
*/
#define MAP_SENSOR_MIN_MV        240  // 400
#define MAP_SENSOR_MAX_MV        2650 // 4650
#define MAP_SENSOR_MIN_HPA       100  // 10 kPa or 100 hPa
#define MAP_SENSOR_MAX_HPA       1150 // 115 kPa or 1150 hPa
#define MAP_SENSOR_REFERENCE_MV  3300 // 3300



#define ALPHA_SMOOTHING_RPM_VALUE 1
#define ALPHA_SMOOTHING_ADC_VALUE 1

#define NEW_ADC_VALUE_THRESHOLD 5
#define MINIMUM_ADC_VALUE_THRESHOLD 10

// resolution of ADC. Arduino typically 1024, esp8266 or esp32 4096
#define ADC_RESOLUTION 4096


#define DISPLAY_UPDATE_INTERVALL_MICROS 50000

#define SERIAL_UPDATE_INTERVALL_MICROS 1000000
#define MEASURE_UPDATE_INTERVALL_MICROS 1000000
#define SENSOR_READING_INTERVALL_MICROS 400


#define NUMBER_OF_CYLINDERS 2

CylinderManifoldAbsolutePressureData cylinders[NUMBER_OF_CYLINDERS];
CarbSyncDisplayLCD display;

uint8_t sensorPins[] = {34, 27};

unsigned long lastTimeStampDisplayUpdated = 0;
unsigned long lastTimeStampSensorReading = 0;
unsigned long lastTimeStampSerialUpdated = 0;
unsigned long lastTimeStampMeasure = 0;
unsigned long actTimeStamp = 0;
unsigned long measures = 0;
int measuresPerSec = 0;
int analogReadValue = -1;


void setup() {
  int maxSensorADCValueCalibration = 0;
  int cylinderSmoothedADCValue[NUMBER_OF_CYLINDERS];

  display.setup();

  //set the resolution to 12 bits (0-4096)
  //set the resolution to 11 bits (0-2048)
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);


  for (int i=0; i<NUMBER_OF_CYLINDERS; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  Serial.begin(230400);
  while(!Serial && !Serial.available()){}

  for (int i=0; i<NUMBER_OF_CYLINDERS; i++) {
    cylinders[i].setBoardCharacteristics(ADC_RESOLUTION, MAP_SENSOR_REFERENCE_MV);
    cylinders[i].setMAPSensorCharacteristics(MAP_SENSOR_MIN_MV, MAP_SENSOR_MAX_MV, MAP_SENSOR_MIN_HPA, MAP_SENSOR_MAX_HPA);
    cylinders[i].setMAPSensorOffset(0);
    cylinders[i].setSmoothingAlphaADC(ALPHA_SMOOTHING_ADC_VALUE);
    cylinders[i].setSmoothingAlphaRPM(ALPHA_SMOOTHING_RPM_VALUE);
    cylinders[i].setMinimumADCValueThreshold(MINIMUM_ADC_VALUE_THRESHOLD);
    cylinders[i].setNewADCValueThreshold(NEW_ADC_VALUE_THRESHOLD);
    cylinders[i].disableAutomaticMeasurementStart();
    cylinders[i].enableMeasurement();
  }


  // writeLogHeaderToSerial();

  display.setupCalibrationScreen();

  // "calibrate" sensors ...
  delay(1000);

  // for calibration we want a responsive smoothed ADC value ...
  // setting smoothing alpha to 50 ...
  for (int i=0; i<NUMBER_OF_CYLINDERS; i++) {
    cylinders[i].setSmoothingAlphaADC(20);
  }


  for (int i=0; i<=8000; i++) {
    for (int j=0; j < NUMBER_OF_CYLINDERS; j++) {
      analogReadValue = analogRead(sensorPins[j]);
      cylinders[j].setADCValue(analogReadValue);
    }

    delayMicroseconds(50);

    if ((i % 80) == 0) {
      display.updateCalibrationScreen(cylinders, NUMBER_OF_CYLINDERS, i);
    }
  }

  delay(3000);
  display.destroyCalibrationScreen();

  for (int i=0; i < NUMBER_OF_CYLINDERS; i++) {
    cylinderSmoothedADCValue[i] = round(cylinders[i].getSmoothedADCValue());

    if (cylinderSmoothedADCValue[i] > maxSensorADCValueCalibration) {
      maxSensorADCValueCalibration = cylinderSmoothedADCValue[i];
    }

    Serial.printf("Input: %d smoothed ADC val = %d  maxADCCalibration val = %d\n",i, cylinderSmoothedADCValue[i], maxSensorADCValueCalibration);
  }

  for (int i=0; i < NUMBER_OF_CYLINDERS; i++) {
    cylinders[i].resetMeasures();
  }

  for (int i=0; i < NUMBER_OF_CYLINDERS; i++) {
    int cylinderADCValueOffset = maxSensorADCValueCalibration - cylinderSmoothedADCValue[i];
    cylinders[i].setMAPSensorOffset(cylinderADCValueOffset);
    cylinders[i].setSmoothingAlphaADC(ALPHA_SMOOTHING_ADC_VALUE);
    cylinders[i].setAtmosphericPressureADCValue(maxSensorADCValueCalibration);
    cylinders[i].enableAutomaticMeasurementStart();
    cylinders[i].disableMeasurement();

    Serial.printf("Input: %d ADC offset val = %d \n",i, cylinderADCValueOffset);
  }

  // display.setupSyncBarScreen();
  display.toggleScreen();
}


void loop() {


  // delayMicroseconds(50);
  actTimeStamp = micros();

  if ((actTimeStamp - lastTimeStampSensorReading) > SENSOR_READING_INTERVALL_MICROS) {
    for (int i=0; i< NUMBER_OF_CYLINDERS; i++) {
      analogReadValue = analogRead(sensorPins[i]);
      cylinders[i].setADCValue(analogReadValue);

      // delayMicroseconds(3);
      // Serial.printf("Input: %d ADC mV = %d  val = %d\n",i, analogReadMilliVolts(sensorPins[i]), analogReadValue);
      // Serial.printf("cylinder: %d ADC val = %d\n",i, cylinders[i].getActualADCValue());
      // Serial.printf("cylinder: %d ADC val = %d\n",i, analogReadValue);
    }
    lastTimeStampSensorReading = actTimeStamp;
    measures++;
  }


  if ((actTimeStamp - lastTimeStampDisplayUpdated) > DISPLAY_UPDATE_INTERVALL_MICROS) {
    // display.updateSyncBarScreen(cylinders, 2, measuresPerSec);
    display.updateScreen(cylinders, 2, measuresPerSec);
    lastTimeStampDisplayUpdated = actTimeStamp;
  }

  if ((actTimeStamp - lastTimeStampSerialUpdated) > SERIAL_UPDATE_INTERVALL_MICROS) {
    // writeLogDataToSerial();
    lastTimeStampSerialUpdated = actTimeStamp;
  }

  if ((actTimeStamp - lastTimeStampMeasure) > MEASURE_UPDATE_INTERVALL_MICROS) {
    // Serial.print("measure per sec.: ");
    // Serial.println(measures);
    measuresPerSec = measures;
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
      Serial.print(";minMAPhPa_");
      Serial.print(i);
      Serial.print(";smoothedMinMAPhPa_");
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

    Serial.print(cylinders[i].getMinimumMAPValueAshPa());
    Serial.print(";");

    Serial.print(cylinders[i].getSmoothedMinimumMAPValueAshPa());
    Serial.print(";");

  }
  Serial.println();
}
