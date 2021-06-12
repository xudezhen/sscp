#include "client.h"
#include "comm.h"
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

CClient::CClient(int argc,char *argv[])
{
    m_nArgc = argc;
    m_pArgv = argv;
}
void CClient::DoLogicProc(int sock)
{
    char *file_path = m_pArgv[2];
    char tmp_file[50] = {'\0'};
    char *p = file_path;
    int i = 0;
    while(*p != '\0')
    {
        if(*p == '/')
            i = 0;
        else
            tmp_file[i++] = *p;
        p++;
    }
    tmp_file[i] = '\0';

    //发送文件的信息
    char tmp_buf[sizeof(SNetFileMsg)];
    memset(tmp_buf,0,sizeof(SNetFileMsg));
    SNetFileMsg *file_msg = (SNetFileMsg *)tmp_buf;
    file_msg->info_id = htons(0xf1);
    file_msg->len = sizeof(SNetFileMsg);

    if((m_nArgc == 4) && (strcmp(m_pArgv[3],"-r") == 0))
    {
        int path_len = strlen(file_path);
        char *path_buf = new char[path_len + 1];
        strcpy(path_buf,file_path);
        path_buf[path_len] = '\0';
        file_msg->info_id = htons(0xf2);
        file_msg->len += path_len+1;
        if(send(sock,tmp_buf,sizeof(SNetFileMsg),0) == sizeof(SNetFileMsg))
        {
            if(send(sock,path_buf,path_len+1,0) == path_len+1)
            {
                printf("send run command ok\n");
            }
        }
        delete [] path_buf;
        printf("--------------server reponse start-----------------\n");
        fflush(stdout);
        char buf[FILE_NAME_LEN+2];
        int len = 0;
        while((len = recv(sock,buf,FILE_NAME_LEN,0)) > 0)
        {
            buf[len] = '\0';
            printf("%s",buf);
        }
        printf("--------------server reponse end-----------------\n");
    }
    else
    {
        strcpy(file_msg->name,tmp_file);
        struct stat file_stat;
        stat(file_path,&file_stat);

        long long file_size = file_stat.st_size;
        file_msg->file_size = htonl(file_size);

        long long send_size = 0;
        if(send(sock,tmp_buf,sizeof(SNetFileMsg),0) > 0)
        {
            int filefd = open(file_path,O_RDONLY);
            while(send_size < file_size)
            {
                int tmp_send_size = sendfile(sock,filefd,NULL,BUF_SIZE);
                if(tmp_send_size < 0)
                {
                    printf("send file error，may be server quit!,%s \r\n",strerror(errno));
                    break;
                }
                else
                {
                    send_size += tmp_send_size;
                    int tmp_pencent = (double)send_size*100 / file_size;
                    printf("send file size pencent------- %d%% \r",tmp_pencent);
                }
            }
        }

        if(send_size == file_size)
        {
            printf("已经全部发送！ \r\n");
        }
        else
        {
            printf("发送失败！ \r\n");
        }
        printf("send file size:%d\n",send_size);fflush(stdout);
    }
}
