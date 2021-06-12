#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
//#include <QFile>
//#include <QFileInfo>
//#include <QDebug>
#include "server.h"
#include "client.h"
#include "comm.h"

void sig_child(int)
{
    pid_t pid;
    int stat;
    while( (pid = waitpid(-1,&stat,WNOHANG)) > 0 )
    {
//        printf("pid %d now quit safely \r\n",pid);
        //对子进程进行善后处理
    }
}
void doServer(int argc,char *argv[]);
void doClient(int argc,char *argv[]);

/********************************************
* 函数名：main
* 作者：xdz
* 功能：主函数
* 参数：
* 返回值：
* 说明：
*********************************************/
int main(int argc,char *argv[])
{
    if(argc > 1 && !strcmp(argv[1],"start_server"))
    {
        doServer(argc,argv);
    }
    else
    {
        if(argc <= 2)
        {
            printf("usage:%s server_ip file_path [-r] \r\n",basename(argv[0]));
            return 0;
        }
        doClient(argc,argv);
    }

    return 0;
}

/********************************************
* 函数名：doServer
* 作者：xdz
* 功能：启动服务
* 参数：
* 返回值：
* 说明：
********************************************/
void doServer(int argc, char *argv[])
{
    prctl(PR_SET_NAME,"FileTransServer");
    int listenFd = socket(PF_INET,SOCK_STREAM,0);
    if(listenFd < 0)
    {
        printf("create socket fail! \n");
        return;
    }
    //创建一个Ip4 socket地址
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
//    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(SERVER_PORT);
    //绑定Ip地址和端口
    if(bind(listenFd,(sockaddr *)&address,sizeof(address)) < 0)
    {
        printf("bind fail \n");
        return;
    }
    //开始监听
    if(listen(listenFd,LISTEN_BACK_LOG) < 0)
    {
        printf("listen fail \n");
        return;
    }
    signal(SIGCHLD,sig_child);

    int connfd;
    pid_t childPid;
    for(;;)
    {
        sockaddr_in client;
        socklen_t clilen = sizeof(client);
        if((connfd = accept(listenFd,(sockaddr *)&client,&clilen)) < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            else
            {
                printf("accept error \n");
                return;
            }
        }

        if((childPid = fork()) == 0)//进入子进程
        {
            prctl(PR_SET_PDEATHSIG,SIGKILL);//如果父进程退出，则发信号给自己
            close(listenFd);
            //业务处理
            CServer server(argc,argv);
            server.DoLogicProc(connfd);
            exit(0);
        }
        close(connfd);
    }

    close(listenFd);
}

/********************************************
* 函数名：doClient
* 作者：xdz
* 功能：客户端处理函数
* 参数：
* 返回值：
* 说明：
********************************************/
void doClient(int argc,char *argv[])
{
    signal(SIGPIPE,SIG_IGN);
    struct stat file_stat;
    stat(argv[2],&file_stat);
    if(argc == 3 && !S_ISREG(file_stat.st_mode))
    {
        printf("the input args is not file! \r\n");
        return;
    }
    else
    {
        struct sockaddr_in server_address;
        bzero(&server_address,sizeof(server_address));
        server_address.sin_family = AF_INET;
        inet_pton(AF_INET,argv[1],&server_address.sin_addr);
        server_address.sin_port = htons(SERVER_PORT);

        int sock = socket(PF_INET,SOCK_STREAM,0);
        if(sock < 0)
        {
            printf("socket error! \r\n");
            return;
        }

        if(connect(sock,(struct sockaddr*)&server_address,sizeof(server_address)) < 0)
        {
            printf("connect failed! \r\n");
        }
        else
        {
            printf("connect success! \r\n");
            CClient client(argc,argv);
            client.DoLogicProc(sock);
        }

        close(sock);
        return;
    }
}
