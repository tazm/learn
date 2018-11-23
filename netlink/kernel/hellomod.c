#include <linux/module.h>
#include <linux/kernel.h>
#include <net/netlink.h>
#include <net/sock.h>

#define  NETLINK_TEST 30
struct sock* netlink_exam_sock = NULL;

int send_usrmsg(char *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;

    int ret;

    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if(!nl_skb)
    {
        printk("netlink alloc failure\n");
        return -1;
    }

    nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_TEST, len, 0);
    if(nlh == NULL)
    {
        printk("nlmsg_put failaure \n");
        nlmsg_free(nl_skb);
        return -1;
    }

    memcpy(nlmsg_data(nlh), pbuf, len);
    ret = netlink_unicast(netlink_exam_sock, nl_skb, 100, 0);

    //nlmsg_free 函数的调用会导致系统崩溃
    nlmsg_free(nl_skb);
    printk("send_usrmsg end\n");

    return ret;
}

void nl_process_data(struct sk_buff *__skb)
{
    struct nlmsghdr *nlh = NULL;
    char *umsg = NULL;
    char *kmsg = "zhuming hello users!!!";

    if(__skb->len >= nlmsg_total_size(0))
    {
        nlh = nlmsg_hdr(__skb);
        umsg = NLMSG_DATA(nlh);
        if(umsg)
        {
            printk("kernel recv from user: %s\n", umsg);

            send_usrmsg(kmsg, strlen(kmsg));
        }
    }
}

static int __init zm_init(void)
{
    printk("my module init\n");
    netlink_exam_sock = netlink_kernel_create(&init_net, NETLINK_TEST , 0, nl_process_data, NULL, THIS_MODULE);

    return 0;
}

static void __exit zm_cleanup(void)
{
    printk("my module MODULE_LICENSE\n");

    if (netlink_exam_sock != NULL)
    {
        netlink_kernel_release(netlink_exam_sock);
    }
}

module_init(zm_init);
module_exit(zm_cleanup);
MODULE_LICENSE("GPL");