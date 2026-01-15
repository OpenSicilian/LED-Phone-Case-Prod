High-level purpose

This firmware runs on an ATtiny1616 and controls:

An 8-LED WS2812B (NeoPixel) strip

A single push button

A power-enable pin that likely controls power to the LED rail or an external regulator

The system is designed for very low power consumption.
It normally stays in deep sleep (power-down mode) and only wakes when the user long-presses the button. Once awake, the button is used to:

Short press → cycle through LED modes

Long press → turn the system off (return to sleep)

The currently selected mode is saved in EEPROM so it can be restored after power loss.
