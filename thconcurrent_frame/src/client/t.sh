#!/bin/bash
while true
do
	ls /proc/11709/fd -l | grep socket: | wc -l
	sleep 1
done
