# Copy this script to firmware/main
# Note: link the fridget src folder to ../user/applications/fridget
#ln -s /Users/thorsten/develop/fridget/fridget-client-firmware/spark/app/src ../user/applications/fridget
make PLATFORM=core APP=fridget TARGET_FILE=core-firmware all program-dfu
