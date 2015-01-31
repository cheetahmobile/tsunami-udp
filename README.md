tsunami-udp
===================
 * Cloned version from http://tsunami-udp.sourceforge.net/
 * Fixed some bug and make it perfect

Simple Usages
----------------------
In a scenario of transfering a file from the server to the client:

# Compiling

    ./recompile.sh 

# Server Side

    ./server/tsunamid *

# Client Side

    ./client/tsunami 
    # Accepts following commands:     close    connect    get    dir    help    quit    set

# Full Command

    ./client/tsunami connect your.server.add set udpport 51031 get path/to/yourfile quit

# Enjoy

    ./server/tsunamid --help


    ./client/tsunami help


TODO
-------------
  * Nteractive difference


中文说明
==========
海啸UDP传输协议
---------------------
* http://tsunami-udp.sourceforge.net/ 的clone版本
* 解决和优化了原版的一些问题
使用方法
-----------
情景：从服务器-----传输文件---------->客户端

#编译安装

    ./recompile.sh 
#服务器端配置

    ./server/tsunamid *
#客户端配置

    ./client/tsunami 
    # Accepts following commands:     close    connect    get    dir    help    quit    set
#全部命令

    ./client/tsunami connect your.server.add set udpport 51031 get path/to/yourfile quit
TODO列表
--------------
  * Nteractive difference
