#!/bin/bash
# Use this to quickly test run the server in valgrind
spawn-fcgi -p9005 -n /usr/bin/valgrind ./server
