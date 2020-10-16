# RFID Reader for raspberry Pi connected by SPI

The RFID reader reads rfids chips or cards (without any security blocks) and send snc json messages, that can be used e.g. by the the audioserver (https://github.com/yorns/audioserver)

Reader is the RC522. One helping page could be this: https://tutorials-raspberrypi.de/raspberry-pi-rfid-rc522-tueroeffner-nfc/
The implementation given on that page is based on python and raspberian. The implementation on this page has no dependency, but rfid must be enabled. 

**The idea on this page is to provide code to use with yocto, raspberry pi and spi-connected RC522**

The RFID reader should be used within C++ Systems.

So have some fun!
