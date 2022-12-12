/**
 * 
 * CylinderManifoldAbsolutePressureData implementation for the Arduino Carb Sync
 * 
 * CylinderManifoldAbsolutePressureData is used to calculate basic data based on the value of the ADC.
 * The ADV value represents the absolute pressure. For carb synchronisation we are interested in the minimum value.
 * The class provides functions to calculate the following values based on the ADC value and elapsed time:
 * - RPM (rounds per minutes)
 * - RPM smoothed
 * - ADC value smoothed
 * - ADC value represented in millivolts (mv)
 * - ADC value converted to kilo Pascal (kPa)
 * 
 * 
 * 
 * Author:      Alexander Bauer (albauer@gmx.de)
 * Version:     2022.12.06.1
 * Date:        12/06/2022
 * 
 *  
 */

#ifndef CYLINDERMANIFOLDABSOLUTEPRESSUREDATA_H
#define CYLINDERMANIFOLDABSOLUTEPRESSUREDATA_H


// ADC epsilon, minimum difference to consider ADC as potentially new minimum value
#ifndef ADC_VALUE_EPSILON
    #define ADC_VALUE_EPSILON 10
#endif





class CylinderManifoldAbsolutePressureData {
    public:
        CylinderManifoldAbsolutePressureData();

        void resetMeasures();
        void setMAPSensorCharacteristics (float minimummV, float maximummV, float minimumkPa, float maximumkPa);
        void setMAPSensorOffset (int offset);
        int  getMAPSensorOffset ();

        void setBoardCharacteristics (int stepsADC, float referenceVoltagemV);

        void setSmoothingAlphaADC (float alpha);
        void setSmoothingAlphaRPM (float alpha);
        // void setSmoothingAlphaMAP (float alpha);

        void setADCValue (int newADCValue);

        int   getActualADCValue ();
        int   getMinimumADCValue ();
        float getSmoothedADCValue ();
        float getSmoothedMinimumADCValue ();

        float getMinimumMAPValueAskPa ();
        float getMAPValueAskPa ();
        float getSmoothedMinimumMAPValueAskPa ();
        float getSmoothedMAPValueAskPa ();

        int getActualRPMValue ();
        int getSmoothedRPMValue ();

    private:

        float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);

        struct MAPSensor {
            float minkPa;
            float maxkPa;
            float minmV;
            float maxmV;
            int sensorADCOffset;
        } sensor;

        struct Board {
            float refVoltagemV;
            int stepsADCValues;
            float increasemVPerADCStep;
        } board;


        struct Measures {
            int _actualADCValue;
            int _minimumADCValueCandidate;
            int _minimumADCValue;
            int _actualRPMValue;
            float _smoothingAlphaADC;
            float _smoothingAlphaRPM;
            // float _smoothingAlphaMAP;
            float _smoothedADCValue;
            float _smoothedMinimumADCValue;
            float _smoothedRPMValue;
            unsigned long   _minimumADCValueTimeStamp;
            bool _doCalculations;
        } measures;

};

#endif