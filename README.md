Working Light Control for O'Reilly Theatre
==========================================

Copyright (C) 2014 Jay Anslow

About
-----
This system is designed to control the working lights in the O'Reilly Theatre. It does this by acting as a switch across two pairs of wires, one which turns the lights on and the other which turns it off. This is controlled by pressing the `ON` or `OFF` button, respectively.

In addition, the lights can be locked on or off for a finite amount of time, by holding the respective button for a longer period.

How to use
----------
Note, the same procedures apply for turning/locking the lights off, but with the `OFF` button and LED.

### Turn the lights on.
At any point, you can turn the lights on by pressing the `ON` button.

### Lock the lights on.
Hold the `ON` button for 3 seconds. The brightness of the `ON` LED will drop when the lights are locked.

The lights will then be locked off for four hours (and the wall switches in the theatre won't work).

After 3:30 hours, the `ON` LED will start flashing. After 3:55 hours, the flashing will get faster.

After 4:00 hours, the lights will be unlocked, but will remain on.

### Lock the lights on for a longer period.
Whilst the lights are locked on, press the `ON` button to reset the timer.

For example, if the lights are locked off and the `OFF` LED starts flashing (after 3:30 hours), press the `OFF` button to reset the timer. The LED will then stop flashing.

### Turn lights off when they are locked on
Press the `OFF` button to unlock the lights and turn them on.

Safety Notes
------------
Due to the fact that the device overrides the inputs from the wall switches around the venue, there should be someone in the tech box whenever the lights are locked off, in case of emergencies.

Troubleshooting
---------------
If the device is not responding, turn it off and on again (by unplugging the cable and then reconnecting it).

Implementation Notes
--------------------
The working lights are controlled by a Polaron system, which listens for a rising edge on it's digital inputs.

This control system works by taking the connections to the Polaron and connecting it to an optoisolator, changing the inputs to the polaron.

For a non-locking lighting xchange, this just closes the optoisolator whilst the button is pressed.

For a locking lighting change, the Arduino Pro Mini opens and closes the optoisolator at 25Hz. This means, if someone presses a button on the wall switch, the Polaron has <40ms to turns the lights on, which isn't enough time for them to start emitting light.

Resources
---------
This repository contains the Arduino sketches (in `arduino_sketches/`) and the Fritzing designs (in `hardware_designs/`). These require the [Arduino IDE](http://arduino.cc/en/main/software) and [Fritzing](http://fritzing.org/download/) to open and edit.
In addition there is a graph representing the deterministic automata that models the software. It requires [yED Graph Editor](http://www.yworks.com/en/products_yed_about.html) to edit.