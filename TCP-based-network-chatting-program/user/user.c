#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<conio.h>
#include<string.h>
#include<winsock.h> 
#include<time.h>
/*防报错，引入头文件*/

/*程序中t1代表设置服务器地址位置*/

int main()
/*引入 winsock头文件*/
{
    char Sendbuf[100];              /*发送数据的缓冲区*/
    char Receivebuf[100];           /*接收数据的缓冲区*/
    int SendLen;                    /*发送数据的长度*/
    int ReceiveLen;                 /*接收数据的长度*/
    int Check1;                     /*链接失败重连检查*/
    unsigned short int Point;       /*端口号*/
    char Address[20];               /*存储IP地址*/
    char END[10] = "#END\0";        /*存储结束标志字符*/

    SOCKET socket_send;             /*定义套接字*/
    SOCKADDR_IN Server_add;         /*服务器地址信息结构*/

    WORD wVersionRequested;         /*字(word):unsigned short*/
    WSADATA wsaData;                /*库版本信息结构*/
    int error;                      /*表示错误*/

    FILE* fp;                       /*文件指针*/
    struct tm* sysTime;             /*系统时间指针*/

    /*---------------------------初始化套接字库--------------------------------*/

    /*定义版本类型。将两个字节组合成一个字，前面是低字节，后面是高字节*/
    wVersionRequested = MAKEWORD(2, 2);
    /*加载套接字库，初始化 Ws232.dll动态链接库*/
    error = WSAStartup(wVersionRequested, &wsaData);
    if (error != 0)
    {
        printf("加载套接字失败！\n");
        printf("程序退出，按任意键结束\n");
        _getch();
        return 0;                   /*出错，程序结束*/
    }

    /*判断请求加载的版本号是否符合要求*/
    if(LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();               /*不符合，关闭套接字库*/
        printf("发生错误，程序退出，按任意键结束\n");
        _getch();           
        return 0;                   /*按任意键退出*/
    }
    printf("请输入点分十进制IP地址:");
    scanf("%s", Address);
    strcat(Address, "\0");
    printf("请输入端口号:");
    scanf("%hu", &Point);
    printf("正在连接至%s,端口号：%d\n",Address,Point);

    /*-------------------------------设置服务器地址-----------------------------------*/

 t1:Server_add.sin_family = AF_INET;  /*地址家族，必须是AF _INET,注意只有它不是网络字节顺序*/
    /*服务器的地址，将一个点分十进制表示为IP地址，inet_ntoa是将地址转换成字符串*/
    Server_add.sin_addr.S_un.S_addr = inet_addr(Address);
    Server_add.sin_port = htons(Point);/*端口号*/

    /*--------------------------------进行连接服务器-----------------------------------*/

    /*客户端创建套接字，但是不需要绑定，只需要和服务器建立起连接就可以了*/
    /*socket_ sendr表示的是套接字，Server_ add是服务器的地址结构*/
    socket_send = socket(AF_INET, SOCK_STREAM, 0);

    /*-----------------------------创建用于连接的套接字---------------------------------*/

    /*AF_INET表示指定地址族，SOCK_STREAM表示流式套接字TCP，特定的地址家族相关的协议*/

    /*连接异常情况*/
    if(connect(socket_send,(SOCKADDR*)&Server_add,sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        printf("连接失败! 是否重连？（Y：1/N：0）\n");
        scanf("%d", &Check1);          
        if (Check1 == 1)
            goto t1;                   /*重置客户端*/
        else
        {
            printf("程序退出，按任意键结束\n");
            _getch();                  /*按任意键退出*/
            closesocket(socket_send);
            WSACleanup();              
            return 0;                  /*清空数据，结束程序*/
        }
    }
    /*连接正常情况*/
    else {
        printf("连接成功!\n(请用_代替空格,按#END结束会话)\n");
        fp = fopen("UserData.txt", "a");                                /*打开日志文件*/
        time_t nowTime;                                                 
        time(&nowTime);
        sysTime = localtime(&nowTime);                                  /*获取系统时间*/

        fprintf(fp, "\n\n%d-%d-%d\t%d:%d:%d\nIP:%s\n端口:%u\n",1900+sysTime->tm_year,sysTime->tm_mon+1,sysTime->tm_mday,
            sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec,Address,Point);
        /*输出本次会话基本信息*/
    }
    /*---------------------------------进行聊天-------------------------------------*/

    while (1) /*无限循环*/
    {
        /*发送数据过程*/
        printf("please enter message:");
        scanf("%s", Sendbuf);                              /*读取用户输入的信息*/

        strcat(Sendbuf, "\0");                             /*添加字符串结束符*/

        if (strcmp(Sendbuf, END) == 0)
        {
            printf("会话已结束,按任意键结束\n");            
            fprintf(fp, "会话已结束\n");                    /*写入日志*/
            strcpy(Sendbuf, "会话已结束\0");                /*重置待发送信息*/
            SendLen = send(socket_send, Sendbuf, 100, 0);   /*发送结束信息*/
            _getch();                                      
            break;                                          /*按任意键退出*/
        }

        fprintf(fp,"Client say: %s\n", Sendbuf);            /*写入日志*/
        
        SendLen = send(socket_send, Sendbuf, 100, 0);       /*发送数据*/
        /*检查是否出错*/
        if (SendLen < 0)
        {
            printf("发送失败！是否重连？（Y：1/N：0）\n");
            scanf("%d", &Check1);
            if (Check1 == 1)
                goto t1;                                     /*重置客户端*/
            else
            {
                printf("程序退出，按任意键结束\n");
                _getch();
                break;                                       /*按任意键退出*/
            }
        }

        /*接收数据过程*/
        ReceiveLen = recv(socket_send, Receivebuf, 100, 0); /*接收数据*/
        if (ReceiveLen < 0)
        {
            printf("接收失败! 是否重连？（Y：1/N：0）\n");
            scanf("%d", &Check1);
            if (Check1 == 1)
                goto t1;                                    /*重置客户端*/
            else
            {
                printf("程序退出，按任意键结束\n");
                _getch();                                   /*按任意键退出*/
                break;
            }
        }
        else
        {
            /*检测是否为结束会话信息*/
            if (strcmp(Receivebuf, "会话已结束") == 0)
            {
                printf("服务器已关闭,按任意键关闭客户端\n");
                fprintf(fp, "会话已结束\n");
                _getch();                                   
                break;                                      /*按任意键退出*/
            }
            /*正常情况，输出信息，写入日志*/
            else
            {
                printf("Server say: %s\n", Receivebuf);
                fprintf(fp, "Server say:%s\n", Receivebuf);
            }
        }
    }

    /*-----------------------------结束阶段--------------------------------*/

    /*释放套接字，关闭动态库*/
    closesocket(socket_send);
    WSACleanup();
    fclose(fp);
    return 0;           /*结束程序*/
}