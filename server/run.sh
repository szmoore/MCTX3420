#!/bin/bash

# Check existence of program
if [ ! -e "server" ]; then
        (echo "Rebuild server.") 1>&2;
        exit 1
fi


if [[ "$(uname -m)" != *arm*  ]]; then
        echo Not running on the BBB
        # Use this to quickly test run the server in valgrind
        spawn-fcgi -p9005 -n ./valgrind.sh
        # Use this to run the server normally
        #spawn-fcgi -p9005 -n ./server
        exit 0
fi

# Check running as root
if [ "$(whoami)" != "root" ]; then
        (echo "Run $0 as root.") 1>&2
        exit 1
fi

# Rotate the logs
echo Rotating the system logs
logrotate -f /etc/logrotate.d/mctxserv.conf

# Identify cape-manager slots
slot=$(echo /sys/devices/bone_capemgr.*/slots | awk '{print $1}')
pwm=/sys/class/pwm/

# Load PWM module
if [ $(cat $slot | grep am33xx_pwm -c) -gt 0 ]; then 
        echo am33xx_pwm already loaded, not loading again; 
else
        echo Enabling PWM driver am33xx_pwm
        echo am33xx_pwm > $slot
fi;

# Safe pins that won't interfere with one another
#ports=(P9_22 P9_42 P9_16 P8_13 P9_28);
#portnumbers=(0 2 4 6 7);
#Full correspondence from pwm0-pwm7
ports=(P9_22 P9_21 P9_42 P9_14 P9_16 P8_19 P8_13 P9_28);
portnumbers=(0 1 2 3 4 5 6 7);
# Enable PWM pins
# They must be exported at this stage, before the device tree
# overlay for that pin is enabled.
for ((c=0; c < ${#ports[*]}; c++)); do
        if [ ! -d $pwm/pwm${portnumbers[$c]} ]; then
                echo Exporting PWM ${portnumbers[$c]} \(${ports[$c]}\)
                echo ${portnumbers[$c]} > $pwm/export
        else
                echo PWM ${portnumbers[$c]} already enabled
        fi;
                
        if [ $(cat $slot | grep ${ports[$c]} -c) -gt 0 ]; then
                echo PWM pin ${ports[$c]} already enabled, not enabling again
        else
                (echo bone_pwm_${ports[$c]} > $slot) 1>&2 >> /dev/null
        fi;
done;

# Load ADCs
if [ $(cat $slot | grep BB-ADC -c) -gt 0 ]; then 
        echo BB-ADC already loaded, not loading again; 
else 
        echo Enabling BB-ADC
        (echo BB-ADC > $slot) 1>&2 >> /dev/null
fi;

# Find adc_device_path
# NOTE: This has to be passed as a parameter, because it is not always the same. For some unfathomable reason. Hooray.
#adc_device_path=$(dirname $(find /sys -name *AIN0))


# Get the parameters
. parameters

echo "Parameters are: $parameters"

# Run the program with parameters
# TODO: Can tell spawn-fcgi to run the program as an unprivelaged user?
# But first will have to work out how to set PWM/GPIO as unprivelaged user
fails=0
while [ $fails -lt 1 ]; do
        spawn-fcgi -p9005 -n -- ./server $parameters
		error=$?
        if [ "$error" == "0" ]; then
                exit 0
        fi
        fails=$(( $fails + 1 ))
        (echo "Restarting server after Fatal Error #$fails") 1>&2
        
done
(echo "Server had too many Fatal Errors ($fails)") 1>&2
exit $fails
