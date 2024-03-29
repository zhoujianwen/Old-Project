//
//  main.cpp
//  httpserver
//
//  Created by 周健文 on 2018/5/11.
//  Copyright © 2018年 Clement. All rights reserved.
//

#include "XTcp/XTcp.h"
//#include "cmmon.h"
#include <string.h>
#include <stdlib.h>
#include <thread>
#include <regex>
#include <map>
#include <fstream>
using namespace std;
static vector<string> Split(const string &strtem,const string &a);
class HttpThread
{
public:
    void Main()
    {
        char buf[65536] = {0}; //recvbuf
        //string cachebuf = "";
        //std::ifstream  input("in",ios::binary);
        //int datalen = 0;
        for(;;)//HTTP1.1 单次连接，多次访问
        {
            //接受http客户端请求
            //int recvLen = 0;
            int recvLen = client.Recv(buf,sizeof(buf)-1);//[0,65535] = 65536
//            while(client.Recv(buf,sizeof(buf)-1)>0)//循环接收数据
//            {
//                recvLen += strlen(buf);
//                cachebuf.append(buf);
//                printf("%s",cachebuf.c_str());
//            }
            
            
            if(recvLen <= 0)
            {
                Close();
                return ;
            }
            buf[recvLen] = '\0';
            printf("=======recv=========\n%s===================\n",buf);
            
            //recv来自客户端的协议请求消息头内容
            //GET /index.html HTTP/1.1
            //Host: 192.168.3.69
            //User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; rv:51.0) Gecko/20100101 Fi
            //Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
            //Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3
            //Accept-Encoding: gzip, deflate
            //DNT: 1
            //Connection: keep-alive
            //Upgrade-Insecure-Requests: 1
            
            string src = buf;
            string pattern = "^([A-Z]+) (.+) HTTP/1";
            std::regex r(pattern);
            std::smatch mas;
            std::regex_search(src,mas,r);
            if(mas.size() == 0)
            {
                printf("%s failed!\n",pattern.c_str());
                Close();
                return ;
            }
            string type = mas[1];
            string path = mas[2];
            if(type != "GET")
            {
                Close();
                return ;
            }
            string filename = path;
            string filepath = "wwwroot";//默认访问目录
//            if(type =="GET")
//            {
//                if(path == "/")   //http://127.0.0.1:8080/ 或 http://127.0.0.1:8080
//                {
//                    filename = "/index.html";   //http://127.0.0.1:8080/index.html
//                }else
//                {
//                    filename = "/404.html";
//                }
//                filepath += filename;
//            }
            if(path == "/")
            {
                filename = "/index.html";
            }
            filepath += filename;
//            if(type == "POST")
//            {
//                
//                const char *splitstr = "\n";
//                char *p = strtok(buf, splitstr);
//                map<string,string> list; //存储key:value
//                vector<string> temp;    //暂存分割字符串
//                
//                temp = Split(buf,"\r\n");
//                while(p!=NULL)
//                {
//                    printf ("%s",p);
//                    temp = Split(p,":");
//                    list.insert(pair<string, string>(temp[0], temp[1]));
//                    p = strtok(NULL, splitstr);
//                }
//            }
            
            FILE *fp = fopen(filepath.c_str(),"rb");
            if(fp == NULL)
            {
                Close();
                return;
            }
            //获取文件大小
            fseek(fp,0,SEEK_END);
            int filesize = ftell(fp);
            fseek(fp,0,0);
            printf("file size is %d\n",filesize);
            
            //回应Http GET请求
            //消息头，协议约定的规则，\r\n也在规则定义范围内。
            string RequestHeaders = "HTTP/1.1 200 OK\r\n"
                                   "Server: Http\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Content-Length: %d"
                                   "\r\n\r\n";
            
            memset(buf,0, strlen(buf));
            sprintf(buf,RequestHeaders.c_str(),filesize);
            
            //发送消息头
            int sendSize = client.Send(buf,strlen(buf));
            printf("sendsize = %d\n",sendSize);
            printf("=======send=========\n%s\n=============\n",buf);
            

            //循环发送正文
            for(;;)
            {
                memset(buf, 0, sizeof(buf));
                int len = fread(buf,1,sizeof(buf),fp); //每次从fp读取64KB
                if(len <=0)break;
                int re = client.Send(buf,len); //每次发送64KB
                if(re<=0)break;
            }
            /*if(system("sh tarobj.sh")<0)//压缩命令
            {
                printf("error");
            }*/

        }
        Close();
    }
    void Close()
    {
        client.Close();
        delete this;
    }
    XTcp client;
};

int main(int argc,char *argv[])
{
    unsigned short port = argc > 1 ? atoi(argv[1]):8082;
    XTcp server;
    server.CreateSocket();
    server.Bind(port);
    for(;;)
    {
        XTcp client = server.Accept();
        HttpThread *th = new HttpThread();
        th->client = client;
        std::thread sth(&HttpThread::Main,th);
        sth.detach();
    }
    server.Close();
    getchar();
    return 0;
}


static vector<string> Split(const string &strtem,const string &a)
{
    vector<string> strvec;
    
    string::size_type pos1, pos2;
    pos2 = strtem.find(a);
    pos1 = 0;
    while (string::npos != pos2)
    {
        strvec.push_back(strtem.substr(pos1, pos2 - pos1));
        
        pos1 = pos2 + 1;
        pos2 = strtem.find(a, pos1);
    }
    strvec.push_back(strtem.substr(pos1));
    return strvec;
}






//static int HttpLog(char *p)
//{
//    FILE *fp = fopen(p,"wb+");
//    if(fp == NULL)
//    {
//        return 0;
//    }
//    int len = fwrite(p, 1, strlen(p), fp);
//};

//HttpResponse   get or post

