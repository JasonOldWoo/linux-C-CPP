#!/bin/bash
for ((i=0;i<1000;i++));
do
	./mh_chat_client 192.168.1.18 $i > /dev/null 2>&1 &
done
