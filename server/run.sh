#!/bin/bash
# Use this to quickly test run the server in valgrind
spawn-fcgi -p9005 -n ./valgrind.sh
# Use this to run the server normally
#./stream &
#spawn-fcgi -p9005 -n ./server
