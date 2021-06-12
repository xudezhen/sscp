#ifndef SERVER_H
#define SERVER_H

class CServer
{
public:
    CServer(int argc,char *argv[]);
    void DoLogicProc(int fd);
};

#endif // SERVER_H
