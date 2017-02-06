# DRIVER

this folder contains 

Board Definition. the information package to install the board under the arduino IDE

open your Arduino preferences (File > Preferences). Then find the Additional Board Manager URLs text box, and paste the below link 


https://raw.githubusercontent.com/arzaman/smartPID/master/Driver/SPC1000/package_arzaman_index.json

Then hit “OK”, and travel back to the Board Manager menu. You should (but probably won’t) be able to find a new entry for SPC1000 board. If you don’t see it, close the board manager and open it again.

USB serial driver

After downloading, extract the ZIP folder and copy down the location of the arzaman.inf file

Open your Device Manager
In Windows 8 or 10, simply search for "Device Manager" and select the Windows app
In the Device Manager, expand the "Other devices" tree -- you should see an entry for "Unknown Device", Right-click and select Update Driver Software...

Select Browse my computer for driver software. On the next screen

Paste the directory location of your arzaman.inf and sparkfun.cat files into the search location. Then hit "Next".

Click "Install" when the next pop-up questions if you want to install the driver.

The driver installation may take a moment, when it's done you should be greeted with a "successfully updated your driver software" message!