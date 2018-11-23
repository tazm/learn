#include "gtest/gtest.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <stdio.h>

#include <common/taskqueue.h>
#include <common/win_linux.h>
// #include <log4cxx/propertyconfigurator.h>
// 
// log4cxx::LoggerPtr g_logger;
// 
// static void _InitLog4cxx()
// {
//     log4cxx::PropertyConfigurator::configure("log4cxx.conf");
//     g_logger = log4cxx::Logger::getLogger("vdp");
// }

TEST(Example, SendTo)
{
    int netlink_socket = 0;
    if ((netlink_socket = socket(AF_NETLINK, SOCK_RAW, 30)) < 0)
    {
        printf("socket error %s\n", strerror(errno));
        return;
    }

    sockaddr_nl src_addr;
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = 100;
    src_addr.nl_groups = 0;

    //不进行bind的话内核无法向应用层传递消息
    if (bind(netlink_socket, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0)
    {
        printf("bind error %s\n", strerror(errno));
        close(netlink_socket);
        return;
    }

    char buff[4] = {0x31, 0x32, 0x33, 0x34};
    int buffer_len = 4;

    nlmsghdr* nlmsghdr1 = (struct nlmsghdr *)malloc(NLMSG_SPACE(buffer_len));
    memset(nlmsghdr1, 0, NLMSG_SPACE(buffer_len));
    memcpy(NLMSG_DATA(nlmsghdr1), buff, buffer_len);

    nlmsghdr1->nlmsg_len = NLMSG_SPACE(buffer_len);
    nlmsghdr1->nlmsg_type = 0;
    nlmsghdr1->nlmsg_pid = 200;
    nlmsghdr1->nlmsg_flags = 0;

    sockaddr_nl dst_addr;
    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pid = 0;
    dst_addr.nl_groups = 0;

    int ret = sendto(netlink_socket, nlmsghdr1, nlmsghdr1->nlmsg_len, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr));
    if (ret == -1)
    {
        printf("sendto error %s\n", strerror(errno));
    }

    EXPECT_GE(ret, 0);

    char recv_buff[64] = {0};
    socklen_t len = sizeof(struct sockaddr_nl);

    printf("recvfrom  begin\n");
    ret = recvfrom(netlink_socket, recv_buff, 64, 0, (struct sockaddr *)&dst_addr, &len);
    printf("recvfrom  end\n");
    if (ret == -1)
    {
        printf("recvfrom error %s\n", strerror(errno));
    }
    else
    {
        if(ret >= NLMSG_HDRLEN)
        {
            nlmsghdr* nlh = (nlmsghdr*)recv_buff;
            char* umsg = (char*)NLMSG_DATA(nlh);
            if(umsg)
            {
                printf("use recv from kernel: %s\n", umsg);
            }
        }
    }

    free(nlmsghdr1);
    close(netlink_socket);
}

TEST(Example, SendMsg)
{
    int netlink_socket = 0;
    if ((netlink_socket = socket(AF_NETLINK, SOCK_RAW, 26)) < 0)
    {
        printf("m_socket error %s\n", strerror(errno));
        return;
    }

    sockaddr_nl src_addr;
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;

    if (bind(netlink_socket, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0)
    {
        printf("bind error %s\n", strerror(errno));
        close(netlink_socket);
        return;
    }

    char buff[4] = {0x31, 0x32, 0x33, 0x34};
    int buffer_len = 4;

    nlmsghdr* nlmsghdr1 = (struct nlmsghdr *)malloc(NLMSG_SPACE(buffer_len));
    if (NULL == nlmsghdr1)
    {
        close(netlink_socket);
        return;
    }
    memset(nlmsghdr1, 0, NLMSG_SPACE(buffer_len));
    memcpy(NLMSG_DATA(nlmsghdr1), buff, buffer_len);

    nlmsghdr1->nlmsg_len = NLMSG_SPACE(buffer_len);
    nlmsghdr1->nlmsg_type = 0;
    nlmsghdr1->nlmsg_pid = getpid();
    nlmsghdr1->nlmsg_flags = 0;

    sockaddr_nl dst_addr;
    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pid = 0;
    dst_addr.nl_groups = 0;

    iovec iov = { nlmsghdr1, nlmsghdr1->nlmsg_len };
    struct msghdr msg = {&dst_addr, sizeof(dst_addr), &iov, 1, NULL, 0, 0};

    int ret = sendmsg(netlink_socket, &msg, 0);
    if (ret == -1)
    {
        printf("sendmsg error %s\n", strerror(errno));
    }

    EXPECT_GE(ret, 0);

    free(nlmsghdr1);
    close(netlink_socket);
}

//https://www.cnblogs.com/wenqiang/p/6306727.html
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#define NETLINK_TEST    30
#define MSG_LEN            125
#define MAX_PLOAD        125

typedef struct _user_msg_info
{
    struct nlmsghdr hdr;
    char  msg[MSG_LEN];
} user_msg_info;

TEST(Example, Test)
{
    int skfd;
    int ret;
    user_msg_info u_info;
    socklen_t len;
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl saddr, daddr;
    char *umsg = "hello netlink!!";

    /* 创建NETLINK socket */
    skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if(skfd == -1)
    {
        perror("create socket error\n");
        return;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK; //AF_NETLINK
    saddr.nl_pid = 100;  //端口号(port ID) 
    saddr.nl_groups = 0;
    if(bind(skfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        perror("bind() error\n");
        close(skfd);
        return ;
    }

    memset(&daddr, 0, sizeof(daddr));
    daddr.nl_family = AF_NETLINK;
    daddr.nl_pid = 0; // to kernel 
    daddr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PLOAD));
    memset(nlh, 0, sizeof(struct nlmsghdr));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PLOAD);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = 0;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = saddr.nl_pid; //self port

    memcpy(NLMSG_DATA(nlh), umsg, strlen(umsg));
    ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
    if(!ret)
    {
        perror("sendto error\n");
        close(skfd);
        exit(-1);
    }
    printf("send kernel:%s\n", umsg);

    memset(&u_info, 0, sizeof(u_info));
    len = sizeof(struct sockaddr_nl);
    ret = recvfrom(skfd, &u_info, sizeof(user_msg_info), 0, (struct sockaddr *)&daddr, &len);
    if(!ret)
    {
        perror("recv form kernel error\n");
        close(skfd);
        exit(-1);
    }

    printf("from kernel:%s\n", u_info.msg);
    close(skfd);

    free((void *)nlh);
    return;
}
