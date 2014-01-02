#!/bin/bash

# Print the last state change for each thread
# (to help find e.g. unterminated threads)

grep ThreadAxis natr.log|grep '\->'|sort -k3|gawk -F' ' '{if (a!=$2) {print b; a=$2}; b=$N}'
