#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h> 
#include<conio.h>
#include<string.h>
#include<winsock.h> 
#include<time.h>

/*防报错，引入头文件*/
int main()
{
    char Sendbuf[100];                          /*发送数据的缓冲区*/
    char Receivebuf[100];                       /*接收数据的缓冲区*/
    int SendLen;                                /*发送数据的长度*/
    int ReceiveLen;                             /*接收数据的长度*/
    int Length;                                 /*表示SOCKADDR的大小*/
    unsigned short int Point;                   /*端口号*/
    int Check1;                                 /*链接失败重连检查*/
    char END[10] = "#END\0";                    /*存储结束标志字符*/

    SOCKET socket_server;                       /*定义套接字*/
    SOCKET socket_receive;                      /*用于链接套接字*/

    SOCKADDR_IN Server_add;                     /*服务器地址信息结构*/
    SOCKADDR_IN Client_add;                     /*客户端信息地址结构*/

    WORD wVersionRequested;                     /*字（word）：unsigned short*/
    WSADATA wsaData;                            /*库版本信息结构*/
    int error;                                  /*表示错误*/

    FILE* fp;                                   /*文件指针*/
    struct tm* sysTime;                         /*系统时间指针*/

    /*指定端口*/
t1: printf("请指定端口号:");
    scanf("%hu", &Point);

    /*---------------------------初始化套接字库--------------------------------*/

    /*定义版本类型。将两个字节组合成一个字，前面是低字节，后面是高字节*/
    wVersionRequested = MAKEWORD(2, 2);             /*加载套接字库，初始化 Ws232.dll动态链接库*/
    error = WSAStartup(wVersionRequested, &wsaData);
    if (error != 0)
    {
        printf("加载套接字失败！\n");
        printf("程序退出，按任意键结束\n");
        _getch();
        return 0;                                    /*出错，程序结束*/
    }
    /*判断请求加载的版本号是否符合要求*/
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        printf("加载套接字失败！\n");
        WSACleanup();                               /*不符合，关闭套接字库*/
        printf("程序退出，按任意键结束\n");
        _getch();
        return 0;                                   /*按任意键退出*/
    }

    /*---------------------------------设置连接地址----------------------------------*/

    Server_add.sin_family = AF_INET;                             /*地址家族，必须是AF_INET,注意只有它不是网络字节顺序*/
    Server_add.sin_addr.S_un.S_addr = htonl(INADDR_ANY);         /*主机地址*/
    Server_add.sin_port = htons(Point);                          /*端口号*/
    /*AF_INET表示指定地址族，SOCK_STREAM表示流式套接字TCP，特定的地址家族相关的协议*/
    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    /*---绑定套接字到本地的某个地址和端口上---*/
    /*socket_ server为套接字，(SOCKADDR*)&Server_add为服务器地址*/
    if (bind(socket_server, (SOCKADDR*)&Server_add, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        printf("绑定失败\n");
        printf("程序退出，按任意键结束\n");
        _getch();                                       
        closesocket(socket_server);
        WSACleanup();
        return 0;                                                 /*出错，程序结束*/
    }

    /*----------------------------设置套接字为监听状态------------------------------*/

    /*监听状态，为连接作准备，最大等待的数目为5*/
    if (listen(socket_server, 5) < 0)
    {
        printf("监听失败\n");
        printf("程序退出，按任意键结束\n");
        _getch();
        closesocket(socket_server);
        WSACleanup();
        return 0;                                                 /*出错，程序结束*/
    }

    /*----------------------------------接受连接------------------------------------*/

 t2:printf("等待连接中\n");
    Length = sizeof(SOCKADDR);
    /*接受客户端的发送请求，等待客户端发送connect请求*/
    socket_receive = accept(socket_server, (SOCKADDR*)&Client_add, &Length);

    /*连接异常情况*/
    if (socket_receive == SOCKET_ERROR)
    {
        printf("连接失败! 是否重置服务器？（Y：1/N：0）\n");
        scanf("%d", &Check1);
        if (Check1 == 1)
        {
            closesocket(socket_receive);
            closesocket(socket_server);
            WSACleanup();                           /*释放套接字，关闭动态库*/
            printf("服务器重置成功");
            goto t1;                                /*重置服务器*/
        }
        else
        {
            printf("程序退出，按任意键结束\n");
            _getch();
            closesocket(socket_receive);
            closesocket(socket_server);
            WSACleanup();                           /*释放套接字，关闭动态库*/
            return 0;                               /*程序结束*/
        }
    }
    else {
        printf("连接成功!\n(请用_代替空格,按#END结束会话)\n");
        fp = fopen("ServerData.txt", "a");          /*打开日志文件*/
        time_t nowTime;
        time(&nowTime);
        sysTime = localtime(&nowTime);              /*获取系统时间*/

        fprintf(fp, "\n\n%d-%d-%d\t%d:%d:%d\n端口:%u\n", 1900 + sysTime->tm_year, sysTime->tm_mon + 1, sysTime->tm_mday,
            sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec, Point);
    }
    /*--------------------------------进行聊天--------------------------------------*/
    while (1)
    {
        /*接收数据*/
        ReceiveLen = recv(socket_receive, Receivebuf, 100, 0);
        if (ReceiveLen < 0)
        {
            printf("接收失败！是否重置服务器？（Y：1/N：0/WAIT：2）\n");
            scanf("%d", &Check1);
            if (Check1 == 1)
            {
                closesocket(socket_receive);
                closesocket(socket_server);
                WSACleanup();
                printf("服务器重置成功!\n");
                goto t1;                                        /*重置服务器*/
            }
            else if (Check1 == 2)
                goto t2;                                        /*返回等待*/
            else
            {
                printf("程序退出，按任意键结束\n");
                _getch();
                break;                                          /*按任意键退出*/
            }
        }
        else
        {
            /*检测是否为结束会话信息*/
            if (strcmp(Receivebuf, "会话已结束") == 0)
            {
                printf("会话已结束,是否重置服务器？（Y：1/N：0/WAIT：2）\n");
                scanf("%d", &Check1);
                if (Check1 == 1)
                {
                    closesocket(socket_receive);
                    closesocket(socket_server);
                    WSACleanup();
                    printf("服务器重置成功");
                    goto t1;                                    /*重置服务器*/
                }
                else if (Check1 == 2)
                    goto t2;                                    /*返回等待*/
                else
                {
                    printf("程序退出，按任意键结束\n");
                    _getch();
                    break;                                      /*按任意键退出*/
                }
            }
            /*正常情况，输出信息，写入日志*/
            else
            {
                printf("Client say: %s\n", Receivebuf);
                fprintf(fp, "Client say:%s\n", Receivebuf);
            }
            /*发送数据*/
            printf("please enter message:");
            scanf("%s", Sendbuf);                                   /*读取用户输入的信息*/

            strcat(Sendbuf, "\0");                                  /*添加字符串结束符*/

            if (strcmp(Sendbuf, END) == 0)
            {
                printf("会话已结束,按任意键结束\n");
                fprintf(fp, "会话已结束");                           /*写入日志*/
                strcpy(Sendbuf, "会话已结束\0");                     /*重置待发送信息*/
                SendLen = send(socket_receive, Sendbuf, 100, 0);     /*发送结束信息*/
                _getch();
                break;                                               /*按任意键退出*/
            }

            fprintf(fp, "Server say: %s\n", Sendbuf);                /*写入日志*/

            SendLen = send(socket_receive, Sendbuf, 100, 0);         /*发送数据*/
            if (SendLen < 0)
            {
                printf("发送失败！是否重置服务器？（Y：1/N：0/WAIT：2）\n");
                scanf("%d", &Check1);
                if (Check1 == 1)
                {
                    closesocket(socket_receive);
                    closesocket(socket_server);
                    WSACleanup();                                    /*释放套接字，关闭动态库*/
                    printf("服务器重置成功");
                    goto t1;                                         /*重置服务器*/
                }
                else if (Check1 == 2)
                    goto t2;                                         /*返回等待*/
                else
                {
                    printf("程序退出，按任意键结束\n");
                    _getch();
                    break;                                           /*按任意键退出*/
                }
            }
        }
    }

    /*--------------------------------结束阶段----------------------------------*/

    /*释放套接字，关闭动态库*/
    closesocket(socket_receive);
    closesocket(socket_server);
    WSACleanup();
    return 0;                /*结束程序*/
}