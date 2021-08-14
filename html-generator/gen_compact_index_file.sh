#!/bin/bash

TMPFILE='mktemp' || exit 1
printf "static const char index_html[] PROGMEM = R\"rawliteral(" > ../webpages.h
cat http/main_base_ip_arduino.js > $TMPFILE
cat dist/main.js >> $TMPFILE
awk '/HEPPELEPPE/{system("cat '$TMPFILE'");next}1' http/index_base.html | tr -d '\011\012\015' | tr -s " " >> ../webpages.h
printf ")rawliteral\";\n" >> ../webpages.h
rm -rf $TMPFILE

TMPFILE='mktemp' || exit 1
printf "static const char login_template[] PROGMEM = R\"rawliteral(" >> ../webpages.h
cat http/main_base_ip_arduino.js > $TMPFILE
cat src/login.js >> $TMPFILE
awk '/HEPPELEPPE/{system("cat '$TMPFILE'");next}1' http/loginTemplate.html | tr -d '\011\012\015' | tr -s " " >> ../webpages.h
printf ")rawliteral\";\n" >> ../webpages.h
rm -rf $TMPFILE
printf "static const char login_next[] PROGMEM = R\"rawliteral(" >> ../webpages.h
cat http/loginCont.html | tr -d '\011\012\015' | tr -s " " >> ../webpages.h
printf ")rawliteral\";\n" >> ../webpages.h



#printf "static const char favicon[] PROGMEM = R\"rawliteral(" > ../favicon.h
#cat http/favicon.ico >> ../favicon.h
#printf ")rawliteral\";\n" >> ../favicon.h

TMPFILE='mktemp' || exit 1
cat http/main_base_ip.js > $TMPFILE
cat dist/main.js >> $TMPFILE
awk '/HEPPELEPPE/{system("cat '$TMPFILE'");next}1' http/index_base.html | tr -d '\011\012\015' | tr -s " " > webpage.html
rm -rf $TMPFILE
