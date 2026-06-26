### High-level purpose

This firmware runs on an ATtiny1616 and controls:

An 8-LED WS2812B (NeoPixel) strip

A single push button

A power-enable pin that likely controls power to the LED rail or an external regulator

The system is designed for very low power consumption.
It normally stays in deep sleep (power-down mode) and only wakes when the user long-presses the button. Once awake, the button is used to:

Short press → cycle through LED modes

Long press → turn the system off (return to sleep)

The colour cycle is as follows with a solid colour mode followed by flashing mode of the same colour
RED -> YELLOW -> PINK -> GREEN -> ORANGE -> PURPLE -> BLUE

The currently selected mode is saved in EEPROM so it can be restored after power loss.

## How to upload to board
1. Install this board package: https://raw.githubusercontent.com/SpenceKonde/ReleaseScripts/refs/heads/master/package_drazzy.com_index.json
2. Use USB Serial UPDI Programmer with 3.3V I think
3. Settings for upload: (Make sure Chip is ATtiny1616, Clock is 16Mhz Internal and Programmer is set to SerialUPDI - SLOW: 57600 baud)
<img width="490" height="550" alt="image" src="https://github.com/user-attachments/assets/27b8eb26-e0c2-4d51-a724-0cf734578c80" />
4. Then Sketch -> Upload Using Programmer, doing normal upload might work as well
