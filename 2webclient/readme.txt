就是能演示注册、打洞、发数据，这些功能

DANSON 2016/2/22 14:43:18
编译成MNDBServer.exe后，用命令行可以运行

DANSON 2016/2/22 14:43:24
MNDBServer.exe -name Virtual-IPC -type 0 -ip 121.40.124.13 -port 8500 -mode 1 -frame_mode_enable 0 

DANSON 2016/2/22 14:43:29
这个是命令行例子

DANSON 2016/2/22 14:43:40
MNDBServer.exe -name ipcclient22 -type 0 -target Virtual-IPC1 -contype 0 -ip 121.40.124.13 -port 8500 -mode 0 -frame_mode_enable 0

这个是命令行客户端。前面是服务端。

对方已成功接收了您发送的离线文件“SDKlibs-20160222-100439.rar”(2.98MB)。

DANSON 2016/2/22 14:43:50
只需要mndbserver.cpp和mndbserver.h也能编译这个的linux版本，his的也行

DANSON 2016/2/22 14:43:55
各平台都是用这一套例子代码测试。先运行服务端，在多运行几个客户端可以一起连上服务端

test
