#include "comm.h"
#include "server.h"
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
#include <string>
#include <sys/socket.h>
#include <netinet/tcp.h>
CServer::CServer(int argc,char *argv[])
{
}

void CServer::DoLogicProc(int fd)
{
    int keepAlive = 1; // 开启keepalive属性
    int keepIdle = 120; // 如该连接在60秒内没有任何数据往来,则进行探测
    int keepInterval = 5; // 探测时发包的时间间隔为5 秒
    int keepCount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
    setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
    char buf[BUF_SIZE];
    int recv_size = 0;
    memset(buf,0,BUF_SIZE);
    //接受文件头信息
    recv_size = recv(fd,buf,sizeof(SNetFileMsg),0);
    if(recv_size <= 0)
    {
        printf("client close the conn,quit!\r\n");
        return;
    }
    while(recv_size < (int)sizeof(SNetFileMsg))
    {
        recv_size += recv(fd,buf,sizeof(SNetFileMsg) - recv_size,0);
    }
    SNetFileMsg *file_msg = (SNetFileMsg *)buf;
    if(ntohs(file_msg->info_id) != 0xf1 && ntohs(file_msg->info_id) != 0xf2)
        return;
    if(ntohs(file_msg->info_id) == 0xf1)//文件传送
    {
        printf("now recv file_msg %s \r\n",file_msg->name);
        //开始接受文件，首先创建FileDown目录
        char *home_path = strcat(getenv("HOME"),"/FileDown/");
        struct stat s_buf;
        stat(home_path,&s_buf);
        if(!S_ISDIR(s_buf.st_mode))
        {
            mkdir(home_path,S_IRUSR|S_IWUSR|S_IXUSR);
        }
        strcat(home_path,file_msg->name);
        int file_fd = open(home_path,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
        if(file_fd < 0)
        {
            printf("open file error!\r\n");
            return;
        }
        long long s_recv_size = 0;
        while((recv_size = recv(fd,buf,BUF_SIZE,0)) > 0)
        {
            write(file_fd,buf,recv_size);
            s_recv_size += recv_size;
        }

        if(recv_size < 0)
        {
            printf("recv data error,quit!****\r\n");
        }
        else
        {
            printf("client close the conn,quit!!****\r\n");
        }
        printf("recv size:%d\n",s_recv_size);
        close(file_fd);
    }
    else if(ntohs(file_msg->info_id) == 0xf2)
    {//脚本执行
        dup2(fd,1);
        dup2(fd,2);
        int namelen = file_msg->len - sizeof(SNetFileMsg);
        char *file_buf = new char[namelen];
        memset(file_buf,0,namelen);
        if(namelen != 0)
        {
            recv_size = recv(fd,file_buf,namelen,0);
            if(recv_size == 0)
            {
                printf("client close the conn,quit!\r\n");
                return;
            }
            while(recv_size < namelen)
            {
                recv_size += recv(fd,buf,namelen - recv_size,0);
            }

            printf("recv cmd:%s\n",file_buf);
            fflush(stdout);
            int ret = -1;
            //执行脚本
            ret = system(file_buf);
            printf("system return:%d\n",ret);
            delete [] file_buf;
        }
        else
        {
            printf("cannot find cmd file\n");
        }
        fflush(stdout);
    }
}
