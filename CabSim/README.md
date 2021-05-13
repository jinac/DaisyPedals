# Cabinet Simulator
Experiments with cabinet simulator.

We experiment with implementing cabinet simulation on the Electrosmith Daisy Petal platform.
5/13/2021: we now suspect noise to be originating from power source as it still can be heard in the non-SD card version. Further tests with less noisy power may need to be made. 
-As of 5/4/2021, we have some functioning code, but we notice a particular frequency hum/noise in the versions that are running the SD card loading (CabSim.cpp and CabSimNoSD2.cpp), even when the weights loaded from the file are not used. We hypothesize that there is power noise being introduced from the SD card reader as the noise is not present when no SD card code is run (CabSimNoSD.cpp).-

Improvements to wait for could be SD card noise suppression (either software-wise or hardware improvements) and wav loading (current code is possibly inefficient loading and is expecting specifically 24-bit 48 kHZ IR)

## Link to IR Pack for testing
https://shift-line.com/irpackbass
