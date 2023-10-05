#! /bin/bash

picotool load -f build/picobutton.elf --verify --execute
while [ $? -ne 0 ]; do
    sleep 1
    picotool load -f build/picobutton.elf --verify --execute
done
