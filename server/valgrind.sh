#!/bin/bash
valgrind --leak-check=full --show-reachable=yes ./server
