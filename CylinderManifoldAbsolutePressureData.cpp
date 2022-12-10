#include "CylinderManifoldAbsolutePressureData.h"
#include <Arduino.h>
// #include <ArduinoLog.h> // https://github.com/thijse/Arduino-Log for logging functionality ...
//     Log.verbose("FiniteStateMachine::addStateTableEntry()\n");



/**
 * Constructor. Initialize with some values ...
*/
CylinderManifoldAbsolutePressureData::CylinderManifoldAbsolutePressureData() {
    this->sensor.minkPa = 0;
    this->sensor.maxkPa = 0;
    this->sensor.minmV = 0;
    this->sensor.maxmV = 0;
    this->sensor.sensorOffset = 0;

    this->board.refVoltagemV = 0;
    this->board.stepsADCValues = 0;
    this->board.increasemVPerADCStep = 0.0;

    this->measures._minimumADCValueCandidate = 16384;
    this->measures._actualRPMValue = 0;
    this->measures._doCalculations = false;
    this->measures._smoothedADCValue = 0.0;
    this->measures._smoothedRPMValue = 0.0;
    this->measures._smoothedMinimumADCValue = 0.0;
    this->measures._minimumADCValueTimeStamp = 0;
    this->measures._minimumADCValue = 16384;
    this->measures._smoothingAlphaADC = 0.0;
    this->measures._smoothingAlphaRPM = 0.0;
    // this->measures._smoothingAlphaMAP = 0.0;

}


void CylinderManifoldAbsolutePressureData::setMAPSensorCharacteristics (float minimummV, float maximummV, float minimumkPa, float maximumkPa) {
    this->sensor.minmV = minimummV;
    this->sensor.maxmV = maximummV;
    this->sensor.minkPa = minimumkPa;
    this-> sensor.maxkPa = maximumkPa;
};

void CylinderManifoldAbsolutePressureData::setMAPSensorOffset (float offset) {
    this->sensor.sensorOffset = offset;
};

void CylinderManifoldAbsolutePressureData::setBoardCharacteristics (int stepsADC, float referenceVoltagemV) {
    this->board.stepsADCValues = stepsADC;
    this->board.refVoltagemV = referenceVoltagemV;
    this->board.increasemVPerADCStep = referenceVoltagemV / stepsADC;
};

void CylinderManifoldAbsolutePressureData::setSmoothingAlphaADC (float alpha) {
    this->measures._smoothingAlphaADC = alpha;
};


void CylinderManifoldAbsolutePressureData::setSmoothingAlphaRPM (float alpha) {
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
    unsigned long timeStampDifference = 0;

    //Serial.print("ADC Value: ");
    //Serial.println(newADCValue);

    newADCValue = newADCValue + this->sensor.sensorOffset;

    // calculate smoothed ADC value using exponential smoothing 
    this->measures._smoothedADCValue =  ((1 - this->measures._smoothingAlphaADC) * this->measures._smoothedADCValue) + 
                                        (this->measures._smoothingAlphaADC * newADCValue);

    
    // check if the new ADC value is smaller than the actual minimum value 
    // if it's smaller we found a new minimum
    if (newADCValue < this->measures._minimumADCValueCandidate) {
        this->measures._minimumADCValueCandidate = newADCValue;
        this->measures._doCalculations = true;

        //Serial.print("ADC minimum candidate: ");
        //Serial.println(this->measures._minimumADCValueCandidate);

    } else if (newADCValue > (this->measures._minimumADCValueCandidate + ADC_VALUE_EPSILON)) {
    // here we know, that the new ADC value is larger than our actual ADC value
    // we assume, that a new cycle of the engine has started
    // and the last ADC value was a minimal ADC value
    // we can start to calculate the RPM value ... 
    // we calculate RPM value from minimum to minimum MAP value
        if (this->measures._doCalculations) {
            // Log.traceln("setADCValue()._doCalculations");

            this->measures._doCalculations = false;

            this->measures._minimumADCValue = this->measures._minimumADCValueCandidate;

            // Serial.print("ADC minimum: ");
            // Serial.println(this->measures._minimumADCValue);


            this->measures._smoothedMinimumADCValue =   ((1 - this->measures._smoothingAlphaADC) * this->measures._smoothedMinimumADCValue) + 
                                                        (this->measures._smoothingAlphaADC * this->measures._minimumADCValue);

            this->measures._minimumADCValueCandidate = this->board.stepsADCValues;

            timeStampDifference = millis() - this->measures._minimumADCValueTimeStamp;

            // In a four-stroke engine that uses a camshaft, each valve is opened every second
            // rotation of the crankshaft. According to this the camshaft runs with half speed
            // of the crankshaft. Because the time difference is derived from the camshaft (Intake to
            // Intake), we have to multiply by 2 to get the speed of the engine.
            // RPM = (1 / timeStampDifference) * MSecPerSec * SecPerMin * 2
            //     = (1 / timeStampDifference) * 1000 * 60 * 2
            //     = (1/ timeStampDifference) * 120000
            this->measures._actualRPMValue = 120000 / timeStampDifference;
            this->measures._smoothedRPMValue =  ((1 - this->measures._smoothingAlphaRPM) * this->measures._smoothedRPMValue) + 
                                                (this->measures._smoothingAlphaRPM * this->measures._actualRPMValue);

            this->measures._minimumADCValueTimeStamp = millis();
        }
    }
};


float CylinderManifoldAbsolutePressureData::mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


int CylinderManifoldAbsolutePressureData::getMinimumADCValue() {
    return this->measures._minimumADCValue;
};


float CylinderManifoldAbsolutePressureData::getSmoothedADCValue() {
    return this->measures._smoothedADCValue;
};


float CylinderManifoldAbsolutePressureData::getSmoothedMinimumADCValue() {
    return this->measures._smoothedMinimumADCValue;
};


float CylinderManifoldAbsolutePressureData::getMinimumMAPValueAskPa() {
    return this->measures._minimumADCValue * this->board.refVoltagemV / this->board.stepsADCValues * this->sensor.maxkPa / this->sensor.maxmV;
/*    
    return map(this->measures._minimumADCValue, 
                round((this->sensor.minmV / this->board.increasemVPerADCStep)-1),
                round((this->sensor.maxmV / this->board.increasemVPerADCStep)-1),
                this->sensor.minkPa, this->sensor.maxkPa);
*/                

};

float CylinderManifoldAbsolutePressureData::getMAPValueAskPa() {
    return this->measures._minimumADCValueCandidate * this->board.refVoltagemV / this->board.stepsADCValues * this->sensor.maxkPa / this->sensor.maxmV;

/*
    return mapfloat(this->measures._minimumADCValue, 
                this->sensor.minmV / this->board.increasemVPerADCStep,
                this->sensor.maxmV / this->board.increasemVPerADCStep,
                this->sensor.minkPa, this->sensor.maxkPa);
*/
/*    
    return mapfloat(this->measures._minimumADCValue, 
                round((this->sensor.minmV / this->board.increasemVPerADCStep)-1),
                round((this->sensor.maxmV / this->board.increasemVPerADCStep)-1),
                this->sensor.minkPa, this->sensor.maxkPa);
*/                

};

float CylinderManifoldAbsolutePressureData::getSmoothedMAPValueAskPa() {
    return this->measures._smoothedADCValue * this->board.refVoltagemV / this->board.stepsADCValues * this->sensor.maxkPa / this->sensor.maxmV;

/*
    return map(round(this->measures._smoothedADCValue), 
            round((this->sensor.minmV / this->board.increasemVPerADCStep)-1),
            round((this->sensor.maxmV / this->board.increasemVPerADCStep)-1),
            this->sensor.minkPa, this->sensor.maxkPa);
*/            
};


float CylinderManifoldAbsolutePressureData::getSmoothedMinimumMAPValueAskPa() {
    return this->measures._smoothedMinimumADCValue * this->board.refVoltagemV / this->board.stepsADCValues * this->sensor.maxkPa / this->sensor.maxmV;

/*    
    return map(round(this->measures._smoothedMinimumADCValue), 
            round((this->sensor.minmV / this->board.increasemVPerADCStep)-1),
            round((this->sensor.maxmV / this->board.increasemVPerADCStep)-1),
            this->sensor.minkPa, this->sensor.maxkPa);
*/            
};



int CylinderManifoldAbsolutePressureData::getActualRPMValue() {
    return round(this->measures._actualRPMValue);
};


int CylinderManifoldAbsolutePressureData::getSmoothedRPMValue() {
    return round(this->measures._smoothedRPMValue);
};

