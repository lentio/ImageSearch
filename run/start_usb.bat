@echo off
REM Usage: ImageSearch -usb <path for adb usb shell to phone> <delay millseconds to fetching image from phone> <output file name for result>
REM  /mnt/sdcard/DCIM/Camera /mnt/sdcard/DCIM/100MEDIA


ImageSearch -usb "/mnt/sdcard/DCIM/Camera" 1000 "g:\image.txt"

pause