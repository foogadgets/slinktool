#!/bin/bash

TMPFILE='mktemp' || exit 1
printf "static const char index_html[] PROGMEM = R\"rawliteral(" > ../webpages.h
cat http/main_base_ip_arduino.js > $TMPFILE
cat dist/main.js >> $TMPFILE
awk '/GURKMAJO/{system("cat 'http/layout.css'");next}1' http/index_base.html >> webpage.tmp
awk '/HEPPELEPPE/{system("cat '$TMPFILE'");next}1' webpage.tmp | tr -d '\011\012\015' | tr -s " " >> ../webpages.h
printf ")rawliteral\";\n" >> ../webpages.h
rm -rf $TMPFILE webpage.tmp

TMPFILE='mktemp' || exit 1
printf "static const char login_template[] PROGMEM = R\"rawliteral(" >> ../webpages.h
cat http/main_base_ip_arduino.js > $TMPFILE
cat src/login.js >> $TMPFILE
awk '/GURKMAJO/{system("cat 'http/layout.css'");next}1' http/loginTemplate.html >> webpage.tmp
awk '/HEPPELEPPE/{system("cat '$TMPFILE'");next}1' webpage.tmp | tr -d '\011\012\015' | tr -s " " >> ../webpages.h
printf ")rawliteral\";\n" >> ../webpages.h
rm -rf $TMPFILE webpage.tmp
printf "static const char login_next[] PROGMEM = R\"rawliteral(" >> ../webpages.h
cat http/loginCont.html | tr -d '\011\012\015' | tr -s " " >> ../webpages.h
printf ")rawliteral\";\n" >> ../webpages.h



TMPFILE='mktemp' || exit 1
cat http/main_base_ip.js > $TMPFILE
cat dist/main.js >> $TMPFILE
awk '/GURKMAJO/{system("cat 'http/layout.css'");next}1' http/index_base.html > webpage.tmp
awk '/HEPPELEPPE/{system("cat '$TMPFILE'");next}1' webpage.tmp | tr -d '\011\012\015' | tr -s " " > webpage.html
rm -rf $TMPFILE webpage.tmp
