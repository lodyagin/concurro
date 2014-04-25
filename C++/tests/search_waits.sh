#!/bin/bash
egrep '(wait: signalled|waits w/o)' "$1" |awk -f search_waits.awk 
