#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/if_packet.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <net/netlink.h>
#include <linux/sched.h>
#include <net/sock.h>

int protocol_rcv(
struct sk_buff *skb,
struct net_device *dev,
struct packet_type *pt,
struct net_device *orig_dev
    )
{
    struct iphdr    *ip_hdr1 = NULL;
    struct iphdr    *ip_hdr2 = NULL;
    u32 src_ip1;
    u32 src_ip2;

    skb = skb_share_check(skb, GFP_ATOMIC);
    if (skb == NULL)
    {
        kfree_skb(skb);
        return NET_RX_DROP;
    }

    if(skb->pkt_type != PACKET_HOST)
    {
        kfree_skb(skb);
        return NET_RX_DROP;
    }

    if (skb->protocol != htons(ETH_P_IP))
    {
        kfree_skb(skb);
        return NET_RX_DROP;
    }

 //printk(KERN_INFO"src IP %pI4\n", &iph->saddr);  
// ip_hdr1 = (struct iphdr*)(skb->data);//ip_hdr(skb);

    kfree_skb(skb);    
    return NET_RX_SUCCESS;
}

static struct packet_type pt_packet_type =
{
    .type = __constant_htons(ETH_P_ALL),
    .dev  = NULL,
    .func = protocol_rcv,
};

static int __init lkp_init(void)
{
    printk("hello wrold\n");
    dev_add_pack(&pt_packet_type);
	return 0;
}

static void __exit lkp_cleanup(void)
{
    printk("good bye\n");
    dev_remove_pack(&pt_packet_type);
}

module_init(lkp_init);
module_exit(lkp_cleanup);
MODULE_LICENSE("GPL");