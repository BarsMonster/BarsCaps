# BarsCaps
This is a very simple CapsLock language switcher for Windows. 
It switches languages sequentially using CapsLock button. You can switch conventional CapsLock mode using Alt+CapsLock (press Alt first). If you want another modifier key to toggle CapsLock - you can specify -shift, -ctrl or -alt command line parameter. Alt is default.
When running, it has a notification icon, where you can close the program if needed. If you were wondering, notification icon is a character "Ӂ" which is part of old Abkhaz alphabet. 

v1.01 and later should work on Windows Vista and later versions. v1.00 might work in earlier versions.

# Installation
* Download latest [pre-built release here](https://github.com/BarsMonster/BarsCaps/releases/latest).
* Unpack, select suitable binary for your system (32 or 64-bit x86, ARM64).
* Recommended way is to run via Task Scheduler with admin privileges to make it work both for admin and user programs. This can be made manually, or 

you can create task from admin console (make sure to replace "full-path" to actual path to executable):
```
schtasks /create /sc onlogon /delay 0000:10 /tn "BarsCaps - language switcher" /tr "c:\full-path\barscaps_x64.exe -alt" /rl highest /f
```

And then run it immediately for the first time:
```
schtasks /run /tn "BarsCaps - language switcher"
```

* Simpler/quicker way of installation is to put executable or shortcut to it into autostart folder (C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp), and start it to activate immediately. But **this will not work in privileged programs**. 

# Uninstall
* Terminate BarsCaps if it's still running
* Remove binary
* Remove task sheduler task in admin console:
```
schtasks /delete /tn "BarsCaps - language switcher" /f
```

# Intentionally missing features
* LED is used to indicate CapsLock mode. Using it to indicate language is very fragile / complex, so I decided not to implement it. 
* BarsCaps does not have hidden mode. If you want to hide it - feel free to hide the notification icon in taskbar notification area settings. 
* Does not support more than 16 languages / layouts (it will be hard to use for more than 3 anyways due to sequential nature of switching).

# Build instructions
* You need to have Visual Studio installed, for example 2022 Community Edition. 
* You need to have command line build tools in your PATH ("c:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build" or something like that)
* Run build.bat

# Feedback
Please let me know if ARM64 version works for you. I have no suitable hardware to test. 
Please report any issues here on github. 
