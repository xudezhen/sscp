#ifndef CLIENT_H
#define CLIENT_H

class CClient
{
public:
    CClient(int argc,char *argv[]);
    void DoLogicProc(int sock);
private:
    int m_nArgc;
    char **m_pArgv;
};

#endif // CLIENT_H
