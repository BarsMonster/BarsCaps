# BarsCaps
This is a very simple CapsLock language switcher for Windows. 
It switches languages sequentially using CapsLock button. You can switch conventional CapsLock mode using Alt+CapsLock (press Alt first).
When running, it has a notification icon, where you can close the program if needed.

# Installation
Select suitable binary for your system (32 or 64-bit x86, ARM64), and put into autostart (ether via shortcut or put binary there directly). 
Has no external dependencies. 

# Intentionally missing features
* LED is used to indicate CapsLock mode. Using it to indicate language is very fragile / complex, so I decided not to implement it. 
* BarsCaps does not have hidden mode. If you want to hide it - feel free to hide the notification icon in taskbar notification area settings. 
* Does not support more than 16 languages / layouts (it will be hard to use for more than 3 anyways due to sequential nature of switching).

# Feedback
Please let me know if ARM64 version works for you. I have no suitable hardware to test. 
Please report any issues here on github. 
I intend to keep BarsCaps very simple and lean. If you want to request / add some major feature - feel free to fork.
