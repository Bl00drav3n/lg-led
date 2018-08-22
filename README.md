# lg-led
Change coloring of zones while typing on an LG-G213 keyboard

## Description
This program runs as a background app to change the different zone's colors on keypresses. On startup, it will add a notification icon to the taskbar(s) 
from where the application can be closed. It works by installing a keyboard hook to record keypresses and change the color of the corresponding zone.

## Dependencies
* Logitech LED Illumination SDK (program was written against v8.98): https://www.logitechg.com/en-us/partnerdeveloperlab.html
* Logitech G-213 keyboard
* English United States-International keyboard layout

## Issues
The keymap gets indexed by virtual keycodes (only the lowest 8 bits are used), 
and is currently hardcoded for the 'English United States-International'-layout (as it is what I'm using). 
It will probably not map correctly to other layouts.
