This is a userspace application which accesses the adc via /dev/iio in continuous sampling mode.

The application scans the scan_elements folder in /dev/iio/devices/iio:deviceX/scan_elements for enabled channels.

Creates a data structure.

Sets the buffer size. Enables the buffer. And reads from the dev file for the driver.

The source code is located under kernel sources "drivers/staging/iio/Documentation/generic_buffer.c".

How to compile:

arm-arago-linux-gnueabi-gcc --static generic_buffer.c -o generic_buffer

or

-gcc --static generic_buffer.c -o generic_buffer


SOURCE: https://github.com/ZubairLK/adc-iio-continuous-sampling-userspace