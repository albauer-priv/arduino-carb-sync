#include "CylinderManifoldAbsolutePressureData.h"
#include <Arduino.h>


/**
 * Constructor. Initialize with some values ...
*/
CylinderManifoldAbsolutePressureData::CylinderManifoldAbsolutePressureData() {
    this->sensor.minkPa = 0;
    this->sensor.maxkPa = 0;
    this->sensor.minmV = 0;
    this->sensor.maxmV = 0;
    this->sensor.sensorADCOffset = 0;

    this->board.refVoltagemV = 0;
    this->board.stepsADCValues = 0;
    this->board.increasemVPerADCStep = 0.0;

    this->measures._smoothingAlphaADC = 2;
    this->measures._smoothingAlphaRPM = 2;
    // this->measures._smoothingAlphaMAP = 0.1;
    this->measures._boardSensorFactor = 0.0;
    this->measures._calcBoardSensorFactor = true;

    this->resetMeasures();
}


void CylinderManifoldAbsolutePressureData::resetMeasures() {
    this->measures._minimumADCValueCandidate = 16384;
    this->measures._actualRPMValue = 0;
    this->measures._doCalculations = false;
    this->measures._smoothedADCValue = 0;
    this->measures._smoothedRPMValue = 0;
    this->measures._smoothedMinimumADCValue = 0;
    this->measures._minimumADCValueTimeStamp = 0;
    this->measures._minimumADCValue = 16384;
    this->measures._actualADCValue = 0;
    this->measures._lastADCValue = 0;
}


void CylinderManifoldAbsolutePressureData::setMAPSensorCharacteristics (int minimummV, int maximummV, int minimumkPa, int maximumkPa) {
    this->sensor.minmV = minimummV;
    this->sensor.maxmV = maximummV;
    this->sensor.minkPa = minimumkPa;
    this->sensor.maxkPa = maximumkPa;
    this->measures._calcBoardSensorFactor = true;
};


void CylinderManifoldAbsolutePressureData::setMAPSensorOffset (int offset) {
    this->sensor.sensorADCOffset = offset;
};


int CylinderManifoldAbsolutePressureData::getMAPSensorOffset () {
    return this->sensor.sensorADCOffset;
};


void CylinderManifoldAbsolutePressureData::setBoardCharacteristics (int stepsADC, int referenceVoltagemV) {
    this->board.stepsADCValues = stepsADC;
    this->board.refVoltagemV = referenceVoltagemV;
    this->board.increasemVPerADCStep = float(referenceVoltagemV) / float(stepsADC);
    this->measures._calcBoardSensorFactor = true;
};


void CylinderManifoldAbsolutePressureData::setSmoothingAlphaADC (int alpha) {
    this->measures._smoothingAlphaADC = alpha;
};


void CylinderManifoldAbsolutePressureData::setSmoothingAlphaRPM (int alpha) {
    this->measures._smoothingAlphaRPM = alpha;
};


/*
void CylinderManifoldAbsolutePressureData::setSmoothingAlphaMAP (float alpha) {
    this->measures._smoothingAlphaMAP = alpha;
};
*/



/**
 * set new ADC value and calculate other dependant values
*/
void CylinderManifoldAbsolutePressureData::setADCValue(int newADCValue) {
    unsigned long timeStampDifference = 0, actTimeStamp;

    this->measures._actualADCValue = newADCValue + this->sensor.sensorADCOffset;


    if (abs(this->measures._actualADCValue - this->measures._lastADCValue) <= ADC_CONSIDER_NEW_VALUE) {
        return;
    }

    // Serial.printf("ADC val act = %d  last = %d\n",this->measures._actualADCValue, this->measures._lastADCValue);

    this->measures._lastADCValue = this->measures._actualADCValue;

    // calculate smoothed ADC value using exponential smoothing 
    this->measures._smoothedADCValue =  (((100 - this->measures._smoothingAlphaADC) * this->measures._smoothedADCValue) + 
                                        (this->measures._smoothingAlphaADC * this->measures._actualADCValue)) / 100.0;

    
    // check if the new ADC value is smaller than the actual minimum value 
    // if it's smaller we found a new minimum
    if (this->measures._actualADCValue < this->measures._minimumADCValueCandidate) {
        this->measures._minimumADCValueCandidate = this->measures._actualADCValue;
        this->measures._doCalculations = true;
    } else if (this->measures._actualADCValue > (this->measures._minimumADCValueCandidate + ADC_VALUE_EPSILON)) {
    // here we know, that the new ADC value is larger than our actual ADC value
    // we assume, that a new cycle of the engine has started
    // and the last ADC value was a minimal ADC value
    // we can start to calculate the RPM value ... 
    // we calculate RPM value from minimum to minimum MAP value
        if (this->measures._doCalculations) {

            this->measures._doCalculations = false;

            this->measures._minimumADCValue = this->measures._minimumADCValueCandidate;

            this->measures._smoothedMinimumADCValue =   (((100 - this->measures._smoothingAlphaADC) * this->measures._smoothedMinimumADCValue) + 
                                                        (this->measures._smoothingAlphaADC * this->measures._minimumADCValue)) / 100.0;

            this->measures._minimumADCValueCandidate = this->board.stepsADCValues;

            actTimeStamp = millis();

            timeStampDifference = actTimeStamp - this->measures._minimumADCValueTimeStamp;

            // In a four-stroke engine that uses a camshaft, each valve is opened every second
            // rotation of the crankshaft. According to this the camshaft runs with half speed
            // of the crankshaft. Because the time difference is derived from the camshaft (Intake to
            // Intake), we have to multiply by 2 to get the speed of the engine.
            // RPM = (1 / timeStampDifference) * MSecPerSec * SecPerMin * 2
            //     = (1 / timeStampDifference) * 1000 * 60 * 2
            //     = (1/ timeStampDifference) * 120000
            if (timeStampDifference > 0) {
              this->measures._actualRPMValue = 120000 / timeStampDifference;        
            } else {
              this->measures._actualRPMValue = 0;        
            }
            // this->measures._actualRPMValue = 120000 / timeStampDifference;
            this->measures._smoothedRPMValue =  (((100 - this->measures._smoothingAlphaRPM) * this->measures._smoothedRPMValue) + 
                                                (this->measures._smoothingAlphaRPM * float(this->measures._actualRPMValue))) / 100.0;

            this->measures._minimumADCValueTimeStamp = actTimeStamp;
        }
    }
    _dumpDataToSerial();
};


float CylinderManifoldAbsolutePressureData::_mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


int CylinderManifoldAbsolutePressureData::getActualADCValue() {
    return this->measures._actualADCValue;
};


int CylinderManifoldAbsolutePressureData::getMinimumADCValue() {
    return this->measures._minimumADCValue;
};


float CylinderManifoldAbsolutePressureData::getSmoothedADCValue() {
    return this->measures._smoothedADCValue;
};


float CylinderManifoldAbsolutePressureData::getSmoothedMinimumADCValue() {
    return this->measures._smoothedMinimumADCValue;
};


void CylinderManifoldAbsolutePressureData::_calculateBoardSensorFactor() {
    float factor = 0.0;

    if ( (this->board.stepsADCValues > 0) && (this->sensor.maxmV > 0)) {
        factor = float(this->board.refVoltagemV) / float(this->board.stepsADCValues) * float(this->sensor.maxkPa) / float(this->sensor.maxmV);
        // Serial.print("boardfactor: ");
        // Serial.println(factor);
    }

    this->measures._boardSensorFactor = factor;
    this->measures._calcBoardSensorFactor = false;
};


float CylinderManifoldAbsolutePressureData::getMinimumMAPValueAskPa() {
    _dumpDataToSerial();
    if (this->measures._calcBoardSensorFactor) {
        _calculateBoardSensorFactor();
    }
    return float(this->measures._minimumADCValue) * this->measures._boardSensorFactor;
};

float CylinderManifoldAbsolutePressureData::getMAPValueAskPa() {
    _dumpDataToSerial();
    if (this->measures._calcBoardSensorFactor) {
        _calculateBoardSensorFactor();
    }
    return float(this->measures._minimumADCValueCandidate) * this->measures._boardSensorFactor;
};

float CylinderManifoldAbsolutePressureData::getSmoothedMAPValueAskPa() {
    _dumpDataToSerial();
    if (this->measures._calcBoardSensorFactor) {
        _calculateBoardSensorFactor();
    }
    return this->measures._smoothedADCValue * this->measures._boardSensorFactor;
};


float CylinderManifoldAbsolutePressureData::getSmoothedMinimumMAPValueAskPa() {
    _dumpDataToSerial();
    if (this->measures._calcBoardSensorFactor) {
        _calculateBoardSensorFactor();
    }
    return this->measures._smoothedMinimumADCValue * this->measures._boardSensorFactor;
};


int CylinderManifoldAbsolutePressureData::getActualRPMValue() {
    return round(this->measures._actualRPMValue);
};


int CylinderManifoldAbsolutePressureData::getSmoothedRPMValue() {
    return round(this->measures._smoothedRPMValue);
};


void CylinderManifoldAbsolutePressureData::_dumpDataToSerial() {
/*
    Serial.println("Board:");
    Serial.print("reference voltage mv: ");
    Serial.println(this->board.refVoltagemV);
    Serial.print("steps ADC value: ");
    Serial.println(this->board.stepsADCValues);
    Serial.print("increase mv per ADC step: ");
    Serial.println(this->board.increasemVPerADCStep);

    Serial.println("--");
    Serial.println("Sensor:");
    Serial.print("min kPa: ");
    Serial.println(this->sensor.minkPa);
    Serial.print("max kPa: ");
    Serial.println(this->sensor.maxkPa);
    Serial.print("min mv: ");
    Serial.println(this->sensor.minmV);
    Serial.print("max mv: ");
    Serial.println(this->sensor.maxmV);
    Serial.print("ADC offset: ");
    Serial.println(this->sensor.sensorADCOffset);
    
    Serial.println("--");
    Serial.println("Measure:");
    Serial.print("actual ADC value: ");
    Serial.println(this->measures._actualADCValue);
    Serial.print("minimum ADC value candidate: ");
    Serial.println(this->measures._minimumADCValueCandidate);
    Serial.print("minimum ADC value: ");
    Serial.println(this->measures._minimumADCValue);
    Serial.print("actual RPM value: ");
    Serial.println(this->measures._actualRPMValue);
    Serial.print("smoothing alpha ADC: ");
    Serial.println(this->measures._smoothingAlphaADC);
    Serial.print("smoothing alpha RPM: ");
    Serial.println(this->measures._smoothingAlphaRPM);
    Serial.print("smoothed ADC value: ");
    Serial.println(this->measures._smoothedADCValue);
    Serial.print("smoothed minimum ADC value: ");
    Serial.println(this->measures._smoothedMinimumADCValue);
    Serial.print("smoothed RPM value: ");
    Serial.println(this->measures._smoothedRPMValue);

    Serial.println("-----------------------");
*/

};
