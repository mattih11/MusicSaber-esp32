# LightSaber controlled MIDI device
This repository provides python scripts to receive 9-DoF sensor data via BTLE and generate MIDI signals.
The idea is to generate a device that can translate light saber movements into music.
![MusicSaber dataflow](_resources/lightsaber_dataflow.png?raw=true "Dataflow")
# Setup
## Hardware
The module is developed on an ESP32 board with a 9-DoF sensor module connected via I2C.
### Wiring
Connect the input voltage to 3v3 and gnd respectively. SCL goes to pin 22. SDA to 21.
![MusicSaber dataflow](_resources/wiring.png?raw=true "Dataflow")
## Software
I used ArduinoIDE for compiling and deployment. You will find a library subfolder in the sources.
This needs to be made available for compilation. I just created a symlink in the library folder of arduino.
```
cd ARDUINO_LIBRARY_PATH (e.g. /home/USER/Arduino/libraries)
ln -s PATH_TO_MUSICSABER_REPO/LSBT . (e.g. ../LightSaber/LSBT)
```
