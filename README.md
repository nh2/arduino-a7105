arduino-a7105
=============

Fixed code from http://www.instructables.com/id/Easy-Android-controllable-PC-Interfaceable-Relati/step6/Arduino-Software/

Witten for the Arduino Duemilanove.


How to use
----------

* Clone the repo
* Work around [this ino bug](https://github.com/amperka/ino/issues/134): remove `/usr/share/arduino/libraries/Robot_Control/` (renaming will not work)
* Install [`ino`](https://github.com/amperka/ino/)
* run `ino build`
* run `ino upload` to upload it to your Arduino
