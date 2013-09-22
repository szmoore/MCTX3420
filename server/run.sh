#!/bin/bash
# Use this to quickly test run the server in valgrind
#spawn-fcgi -p9005 -n ./valgrind.sh
# Use this to run the server normally
#./stream &

# Check running as root
if [ "$(whoami)" != "root" ]; then
	(echo "Run $0 as root.") 1>&2
	exit 1
fi

# Check existence of program
if [ ! -e "server" ]; then
	(echo "Rebuild server.") 1>&2;
	exit 1
fi

# Identify cape-manager slots
slot=$(echo /sys/devices/bone_capemgr.*/slots | awk '{print $1}')

# Load PWM module
modprobe pwm_test
(echo am33xx_pwm > $slot) 1>&2 >> /dev/null

# Load ADCs
(echo cape-bone-iio > $slot) 1>&2 >> /dev/null
# Find adc_device_path
# NOTE: This has to be passed as a parameter, because it is not always the same. For some unfathomable reason. Hooray.
adc_device_path=$(dirname $(find /sys -name *AIN0))


# Run the program with parameters
# TODO: Can tell spawn-fcgi to run the program as an unprivelaged user?
# But first will have to work out how to set PWM/GPIO as unprivelaged user
spawn-fcgi -p9005 -n -- ./server -a "$adc_device_path"
