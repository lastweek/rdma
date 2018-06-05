usage() {
	echo "Usage:"
	echo "  ./insmod 1  Install"
	echo "  ./insmod 2  Remove"
}
if [ "$1" == "1" ]; then
	insmod test.ko
elif [ "$1" == "2" ]; then
	rmmod test.ko
else
	usage
fi
