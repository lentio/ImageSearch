:loop
adb pull /mnt/sdcard/DCIM/Camera
adb shell "rm /mnt/sdcard/DCIM/Camera/*.*"
@echo off 
ping localhost -n 2
@echo on
goto loop
