#!/bin/bash
ulimit -s unlimited  # set stack size to unlimited
export OMP_STACKSIZE="20 m"
./bug4
