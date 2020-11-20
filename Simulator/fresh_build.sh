# Make a clean build
rm -r build
mkdir build
cd build/
cmake ../ -Darduino=$1
make