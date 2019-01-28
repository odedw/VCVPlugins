#!/bin/sh
RACK_DIR=../Rack-SDK make dist && cp -r dist/Inja ~/Documents/Rack/plugins
osascript -e 'tell application "Rack" to quit'
sleep 0.3
open -a "Rack" 