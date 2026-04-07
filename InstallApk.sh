#!/bin/bash

install_to_device(){
local prettyName=$(adb -s $1 shell getprop ro.product.model)
echo "Starting Installatroning on $prettyName"
for APKLIST in $(find . -name "*.apk" -not -name "*unaligned*");
  do
  echo "Installing $APKLIST on $prettyName"
  adb -s $1 install -r $APKLIST
  done
  echo "Finished Installing on $prettyName"
  exit 
}

for SERIAL in $(adb devices | tail -n +2 | cut -sf 1);
do 
  install_to_device $SERIAL&
done