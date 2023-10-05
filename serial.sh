#! /bin/bash

sudo picocom -b 115200 -r -l /dev/ttyACM0
while [ $? -ne 0 ]; do
    sleep 1
    sudo picocom -b 115200 -r -l /dev/ttyACM0
done
