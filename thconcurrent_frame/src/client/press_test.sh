#!/bin/bash
for ((i=0;i<10000;i++));
do
	./mh_chat_client `/sbin/ifconfig | grep inet | grep -v 127.0.0.1 | grep -v inet6 |  awk '{print $2}' | tr -d "addr:"` >> app.out 2>&1 &
done
