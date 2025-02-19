# BarsCaps
This is a very simple CapsLock language switcher for Windows. 
It switches languages sequentially using CapsLock button. You can switch conventional CapsLock mode using Alt+CapsLock (press Alt first).
When running, it has a notification icon, where you can close the program if needed.

# Installation
* Download latest [pre-built release here](https://github.com/BarsMonster/BarsCaps/releases/latest).
* Unpack, select suitable binary for your system (32 or 64-bit x86, ARM64).
* Put executable or shortcut to it into autostart folder (C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp). 

# Intentionally missing features
* LED is used to indicate CapsLock mode. Using it to indicate language is very fragile / complex, so I decided not to implement it. 
* BarsCaps does not have hidden mode. If you want to hide it - feel free to hide the notification icon in taskbar notification area settings. 
* Does not support more than 16 languages / layouts (it will be hard to use for more than 3 anyways due to sequential nature of switching).

# Build instructions
* You need to have Visual Studio installed, for example 2022 Community Edition. 
* YOu need to have command line build tools in your PATH ("c:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build" or something like that)
* Run build.bat

# Feedback
Please let me know if ARM64 version works for you. I have no suitable hardware to test. 
Please report any issues here on github. 
I intend to keep BarsCaps very simple and lean. If you want to request / add some major feature - feel free to fork.
