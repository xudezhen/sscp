#ifndef COMM_H
#define COMM_H
#define FILE_NAME_LEN 50
#define SERVER_PORT 0x4050
#define LISTEN_BACK_LOG 20
#define _FILE_OFFSET_BITS 64
const int BUF_SIZE = 5*1024;
#pragma pack(push,1)
struct SNetFileMsg
{
    short info_id;//消息Id 0xf1:文件传送 0xf2:服务器端脚本执行
    int len;//消息长度
    char name[FILE_NAME_LEN];//文件名称
    long long file_size;//文件长度
};
#pragma pack(pop)
#endif // COMM_H
