#!/bin/bash
. parameters
valgrind --leak-check=full --track-origins=yes ./server $parameters
#valgrind ./server $parameters
