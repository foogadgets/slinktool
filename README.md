# slinktool
Recording robot to record from Spotify to Sony MiniDisc recorders featuring S-Link port.

---------------------------

Installation

Prerequisites
Install node.js, Webpack
Git clone Sony_Slink and arduino-spotify-api




Git clone stlink tool

Start Arduino IDE and open the .ino-file
Update the Credentials.h file.

Select LOLIN(WEMOS) D1 R2 & mini

Build and upload sketch to your D1 mini.

===============
Preparations
Install Arduino IDE

cd Arduino/libraries/
git clone https://github.com/foogadgets/spotify-api-arduino.git
git clone https://github.com/foogadgets/Sony_SLink.git
git clone https://github.com/foogadgets/MusicAlbumTOC.git

Tools -> Manage Libraries…
Search for ArduinoJson and install the package.
Search for arduinoWebSockets and install the package.



cd <your workspace>
git clone https://github.com/foogadgets/slinktool.git
cd slinktool/


install node.js
cd html-generator/
npm install
npm run build
./gen_compact_index_file.sh
cd ..

Start Arduino IDE and open slinktool.ino

Go to menu Arduino -> Preferences and paste following in
Additional Boards Manager URLs:
http://arduino.esp8266.com/stable/package_esp8266com_index.json
Press OK

Go to Tools -> Board -> Boards Manager…
Search for esp and install esp8266

Go to Tools -> Board -> ESP8266 Boards -> LOLIN(WEMOS) D1 R2 & mini

(på OS X behöver man installera egen python3. Det gjorde jag med hjälp av Homebrew. Homebrew kräver dessutom Java JDK samt att jag behövde justera patén till python3 i espressif i filen platform.txt)

Update the wifi credentials in Credentials.h

Build and flash to LOLIN(WEMOS) D1.


=====================
Using Slinktool
=====================

Browse to http://slinktool.local/

If only manual track naming will be used the slinktool is ready to be
used at http://slinktool.local/

To be able to extract album and track data and to sync recordings from
Spotify you need to be a Spotify Premium user.

Login on this page, https://developer.spotify.com/dashboard/login

1. Create an App
     App name and description is not important.

2. Go into you new App and add a redirect uri by pressing "Edit Settings"
    add "http://slinktool.local/callback" as a Redirect URI and Save

3. Go to http://slinktool.local/login and feed in you Applications
    Client ID and Client Secret and click Login and click the link.

You are now done and you can continue to,
http://slinktool.local/

Step 1 and 2 is only needed to be done once.
Step 3 need to be done every time the ESP8266 is rebooted.

Enjoy!

