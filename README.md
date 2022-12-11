# arduino-carb-sync
electronic motorcycle carburator synchronisation device using a microprocessor like arduino or ESP and some manifold absolute pressure (MAP) sensors

## credits, inspiration
If you goolge for digital or electronic carb sync you will find several devices. Some on Git.
A few to mention here, which inspired me:
- Discussion in German Forum: https://forum.2-ventiler.de/vbboard/showthread.php?67592-Eigenbau-Synchrontester&highlight
- Repo yz88: https://github.com/yz88/arduino-digital-carb-sync
- Repo cuyahoga: https://github.com/cuyahoga/CarbSync/blob/master/CarbSync.ino

## sampling rate
some thoughts regarding sampling rate. Assumption: We want to balance carbs around 1,200 rpm (a little bit high but easy to compute). For a 4 stroke engine this will result in 1,200 / 60 = 20 rounds per second per cylinder. Each round will take approx. 1/20 sec. For a full 4 stroke cycle we need to consider 2 rounds (720° crank) resulting in 2/20 sec or 1/10 sec observing/ sample window.

For each cylce (duration 720° or 2 rounds) we can observe that the MAP pressure will have a minimum and maximum. A kind of sine wave (simplified) (the picture was taken with Tunerstudio's MAP Logger).
![Visualization of MAP over time](/doc/2022-04-10%2010-04-33.jpg)

Most interesting is the minumum value. Identifying the minimum should be possible with 10-100 samples per 4 stroke cycle. The more values the better you get cathing the minimum. 

Any sampling rate of 100-1,000 readings per second should be enough. I did not consider Shannon et al.


## initial version using an Arduino nano

### bill of material
- arduino nano
- 1.8" LCD display
- ERA 550492 pressure sensors (MAP sensors) eq. FACET 10.3195, BOSCH 0 261 230 289

![prototype](/doc/20221211_110721.jpg)

The display has an indicator for the high pressure side (triangle) and show a smoothed average reading for MAP (kPa) and ADC values.

![display](/doc/20221211_110739.jpg)

### learnings

The limiting factor using the nano is not sampling rate. Without freerunning mode for ADC reading it samples around 1,000 samples per second for each cylinder (I have two) while updating the display twice a second.

The limiting factor is the needed time and compute power for display updating.

## next steps, bucket list
- [] calibration of sensors at start
- [] documentation
- [] testing
- [] faster display handling
- [] measure arduino nano performane and maybe switch to ESP32
