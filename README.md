# sscp
适用于局域网内，远程拷贝文件，可远程执行命令（类似简单的ssh）
程序编译后，一台机器执行./sscp start_server，另外一台机器可执行./sscp ip file就可传输文件
执行./sscp ip "shell command" -r， 就可远程在对方机器执行命令
