# Make a clean build

boards=("Teensy_32\tTeensy_35\tTeensy_40\tUno")

if [[ ! "\t${boards[@],,}\t" =~ "\t${1,,}\t" ]]; then
	echo -e "\n\tUsage: ./fresh_build <board version>"
	echo -e "\n\tValid Board Version (case-insensitive)"
	echo -e "\t--------------------------------------"
	echo -e "\t${boards}\n"
	exit 0
fi

rm -r build
mkdir build
cd build/
cmake ../ -Darduino=$1