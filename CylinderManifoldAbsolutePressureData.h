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




class CylinderManifoldAbsolutePressureData {
    public:
        CylinderManifoldAbsolutePressureData();

        void resetMeasures();
        void setMAPSensorCharacteristics (int minimummV, int maximummV, int minimumhPa, int maximumhPa);
        void setMAPSensorOffset (int offset);
        int  getMAPSensorOffset ();

        void setBoardCharacteristics (int stepsADC, int referenceVoltagemV);

        void setSmoothingAlphaADC (int alpha);
        void setSmoothingAlphaRPM (int alpha);

        void setMinimumADCValueThreshold (int threshold);
        void setNewADCValueThreshold (int threshold);

        void setAtmosphericPressureADCValue (int adcValue);

        void enableAutomaticMeasurementStart();
        void disableAutomaticMeasurementStart();

        void enableMeasurement();
        void disableMeasurement();

        // void setSmoothingAlphaMAP (float alpha);

        void setADCValue (int newADCValue);

        int   getActualADCValue ();
        int   getMinimumADCValue ();
        int   getMaximumADCValue ();
        float getSmoothedADCValue ();
        float getSmoothedMinimumADCValue ();
        float getSmoothedMaximumADCValue ();

        float getMinimumMAPValueAshPa ();
        float getMAPValueAshPa ();
        float getSmoothedMinimumMAPValueAshPa ();
        float getSmoothedMAPValueAshPa ();
        float getMaximumMAPValueAshPa ();
        float getSmoothedMaximumMAPValueAshPa ();

        int getActualRPMValue ();
        int getSmoothedRPMValue ();

    private:

        float _mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
        void _dumpDataToSerial();
        void _calculateBoardSensorFactor();

        struct MAPSensor {
            int minhPa;
            int maxhPa;
            int minmV;
            int maxmV;
            int sensorADCOffset;
        } sensor;

        struct Board {
            int refVoltagemV;
            int stepsADCValues;
            float increasemVPerADCStep;
        } board;


        struct Measures {
            int _actualADCValue;
            int _lastADCValue;
            int _minimumADCValueCandidate;
            int _minimumADCValue;
            int _maximumADCValueCandidate;
            int _maximumADCValue;

            int _actualRPMValue;
            int _smoothingAlphaADC;
            int _smoothingAlphaRPM;
            // float _smoothingAlphaMAP;
            float _smoothedADCValue;
            float _smoothedMinimumADCValue;
            float _smoothedMaximumADCValue;
            float _smoothedRPMValue;
            unsigned long   _minimumADCValueTimeStamp;
            bool _doCalculations;
            float _boardSensorFactor;
            bool _calcBoardSensorFactor;
        } measures;

        struct Parameters {
            int newADCValueThreshold;
            int minimumADCValueThreshold;
            int atmosphericADCValue;
            int automaticMeasurementStartThreshold;
            bool automaticMeasurementStart;
            bool doMeasurement;
        } params;


};

#endif
