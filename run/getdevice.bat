:loop
adb pull /mnt/sdcard/DCIM/100MEDIA
adb shell "rm /mnt/sdcard/DCIM/100MEDIA/*.*"

ping localhost -n 2
goto loop
