#include <stdbool.h>
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/pkt_cls.h>
#include <linux/swab.h>
#include <linux/tcp.h>
#include <linux/udp.h>

/*******
 * The following declarations are usuaully moved to separate headers.
 * They are gathered here to have all the code in a single standalone file.
 */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define __bpf_htons(x)__builtin_bswap16(x)
# define __bpf_constant_htons(x)___constant_swab16(x)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
# define __bpf_htons(x)(x)
#else
# error "Fix your compiler's __BYTE_ORDER__?!"
#endif

#define bpf_htons(x)\
  (__builtin_constant_p(x) ?\
   __bpf_constant_htons(x) : __bpf_htons(x))

/*
 * Helper macro to place programs, maps, license in
 * different sections in elf_bpf file. Section names
 * are interpreted by elf_bpf loader
 */
#define SEC(NAME) __attribute__((section(NAME), used))

/* Helper function called from BPF programs */
static void *(*bpf_map_lookup_elem)(void *map, void *key) =
        (void *) BPF_FUNC_map_lookup_elem;

/* A helper structure used by eBPF C program
 * to describe map attributes to elf_bpf loader
 */
struct bpf_map_def {
        unsigned int type;
        unsigned int key_size;
        unsigned int value_size;
        unsigned int max_entries;
        unsigned int map_flags;
        unsigned int inner_map_idx;
        unsigned int pinning;
};

/******
 * End of generic declarations
 */

/* 0x3FFF mask to check for fragment offset field */
#define IP_FRAGMENTED 65343

struct pkt_meta {
	/* Not used
	__be32 src;
	__be32 dst;
	*/
	__u16 src_port;
	__u16 dst_port;
};

struct stats_val {
	__u64 bytes;
	__u64 pkts;
};

struct bpf_map_def SEC("maps") portmap = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(__u16),
	.max_entries = 1,
};

struct bpf_map_def SEC("maps") stats = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(struct stats_val),
	.max_entries = 1,
};

static __always_inline bool parse_udp(void *data, __u64 off, void *data_end,
				      struct pkt_meta *pkt)
{
	struct udphdr *udp;

	udp = data + off;
	/*
	 * Prevent out-of-bound access: check that packet is long enough to
	 * contain UDP header after data + off (which points to the end of L3
	 * header)
	 */
	if ((void *)(udp + 1) > data_end)
		return false;

	pkt->src_port = udp->source;
	pkt->dst_port = udp->dest;

	return true;
}

static __always_inline bool parse_tcp(void *data, __u64 off, void *data_end,
				      struct pkt_meta *pkt)
{
	struct tcphdr *tcp;

	tcp = data + off;
	/* Check that packet is long enough to have L2 + L3 + TCP header */
	if ((void *)(tcp + 1) > data_end)
		return false;

	pkt->src_port = tcp->source;
	pkt->dst_port = tcp->dest;

	return true;
}

static __always_inline int process_packet(struct __sk_buff *skb, __u64 off)
{
	void *data_end = (void *)(long)skb->data_end;
	void *data = (void *)(long)skb->data;
	struct pkt_meta pkt = {};
	struct stats_val *count;
	__u16 *blocked_port;
	struct iphdr *iph;
	__u32 zero = 0;
	__u8 protocol;

	iph = data + off;
	/* Check packet is long enough to have L2 + IPv4 header */
	if ((void *)(iph + 1) > data_end)
		return TC_ACT_OK;
	/* Do not process packet if IP header has options */
	if (iph->ihl != 5)
		return TC_ACT_OK;
	/* Do not process fragmented packets as L4 headers may be missing */
	if (iph->frag_off & IP_FRAGMENTED)
		return TC_ACT_OK;

	protocol = iph->protocol;
	off += sizeof(struct iphdr);

	/* Collect IP addresses - currently not used */
	// pkt.src = iph->saddr;
	// pkt.dst = iph->daddr;

	/* Obtain port numbers for UDP and TCP traffic */
	if (protocol == IPPROTO_TCP) {
		if (!parse_tcp(data, off, data_end, &pkt))
			return XDP_DROP;
	} else if (protocol == IPPROTO_UDP) {
		if (!parse_udp(data, off, data_end, &pkt))
			return XDP_DROP;
	} else {
		return TC_ACT_OK;
	}

	blocked_port = bpf_map_lookup_elem(&portmap, &zero);
	if (!blocked_port)
		return TC_ACT_OK;

	/* Block desired packets and update statistics */
	if (pkt.src_port == *blocked_port || pkt.dst_port == *blocked_port) {

		count = bpf_map_lookup_elem(&stats, &zero);
		/* Always check map lookup succeeded before using values */
		if (!count)
			return TC_ACT_OK;

		/* Increment map counters */
		count->bytes += (__u16)(data_end - data);
		count->pkts += 1;

		/* Actually drop */
		return TC_ACT_SHOT;
	}

	return TC_ACT_OK;
}

int port_drop(struct __sk_buff *skb)
{
	void *data_end = (void *)(long)skb->data_end;
	void *data = (void *)(long)skb->data;
	struct ethhdr *eth = data;
	__u32 eth_proto;
	__u32 nh_off;

	nh_off = sizeof(struct ethhdr);
	/* Check that packet is at least the size of Ethernet header */
	if (data + nh_off > data_end)
		return XDP_DROP;
	eth_proto = eth->h_proto;

	/* Return immediately on anything else than IPv4 */
	if (eth_proto == bpf_htons(ETH_P_IP))
		return process_packet(skb, nh_off);
	else
		return TC_ACT_OK;
}
