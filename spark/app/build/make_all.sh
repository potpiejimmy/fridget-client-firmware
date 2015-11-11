rm core-firmware-*
echo "Building for Core, 4.41 inch screen"
echo "#define EPD_SCREEN_TYPE 0"
echo "//#define PLATFORM_PHOTON"
read -p "Press [Enter]"
. fridget_core.sh
mv ../build/target/main/platform-0-lto/core-firmware.bin core-firmware-0-$1.bin
echo "Building for Core, 7.4 inch screen"
echo "#define EPD_SCREEN_TYPE 1"
echo "//#define PLATFORM_PHOTON"
read -p "Press [Enter]"
. fridget_core.sh
mv ../build/target/main/platform-0-lto/core-firmware.bin core-firmware-1-$1.bin
echo "Building for Photon, 7.4 inch screen"
echo "#define EPD_SCREEN_TYPE 1"
echo "#define PLATFORM_PHOTON"
read -p "Press [Enter]"
. fridget_photon.sh
mv ../build/target/user-part/platform-6-m/core-firmware.bin core-firmware-17-$1.bin
echo "Building for Photon, 4.41 inch screen"
echo "#define EPD_SCREEN_TYPE 0"
echo "#define PLATFORM_PHOTON"
read -p "Press [Enter]"
. fridget_photon.sh
mv ../build/target/user-part/platform-6-m/core-firmware.bin core-firmware-16-$1.bin
scp core-firmware-* root@potpiejimmy.de:download
