#!/usr/bin/env bash



if [ $2 ]
then
  sed -i -e 's/'$1'/'$2'/g' ../slinktool.ino
  sed -i -e 's/'$1'.local/'$2'.local/g' http/index_base.html
  sed -i -e 's/'$1'.local/'$2'.local/g' http/loginCont.html
  sed -i -e 's/'$1'.local/'$2'.local/g' src/login.js
fi


npm run build
./gen_compact_index_file.sh

