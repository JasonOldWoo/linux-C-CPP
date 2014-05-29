#!/bin/sh
#. ./showproc.sh
read -p "process name(default mh_chat_client) : " REPLY
if [ "$REPLY" = "" ];
then
	REPLY=mh_chat_client
fi
echo "Starting mass murdering!...."
#ps -aux|grep $REPLY | grep -v grep | awk '{print $2}'|xargs kill -9 > /dev/null 2>&1 &
killall -9 $REPLY
sleep 1
#. ./showproc.sh
echo "OK!.... bye!"
