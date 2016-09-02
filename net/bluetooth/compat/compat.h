#ifndef __BLUETOOTH_COMPAT_H__
#define __BLUETOOTH_COMPAT_H__		1

/* ----------------------------------------------------------
 > af_bluetooth.c */

/* See include/linux/proc_fs.h, fs/proc/generic.c */
static inline void *PDE_DATA(const struct inode *inode)
{
#ifdef CONFIG_PROC_FS
	return PDE(inode)->data;
#else
	BUG();
	return NULL;
#endif
}


/* ----------------------------------------------------------
 > hci_event.c */

/* See include/asm-generic/barrier.h */
#ifndef smp_mb__after_atomic
#define smp_mb__after_atomic()	smp_mb()
#endif


/* ----------------------------------------------------------
 > hci_sock.c */

/* See net/core/skbuff.c */
static inline
struct sk_buff *__pskb_copy_fclone(struct sk_buff *skb, int headroom,
				   gfp_t gfp_mask, bool fclone)
{
	/* TODO: */
	return __pskb_copy(skb, headroom, gfp_mask);
}


/* ----------------------------------------------------------
 > hci_sysfs.c */

/* See include/linux/sysfs.h */
#define __ATTRIBUTE_GROUPS(_name)				\
static const struct attribute_group *_name##_groups[] = {	\
	&_name##_group,						\
	NULL,							\
}

#define ATTRIBUTE_GROUPS(_name)					\
static const struct attribute_group _name##_group = {		\
	.attrs = _name##_attrs,					\
};								\
__ATTRIBUTE_GROUPS(_name)

/* See include/linux/err.h */
static inline int __must_check PTR_ERR_OR_ZERO(__force const void *ptr)
{
	if (IS_ERR(ptr))
		return PTR_ERR(ptr);
	else
		return 0;
}


/* ----------------------------------------------------------
 > mgmt.c */

/* See include/linux/kernel.h */
#define U16_MAX		((u16)~0U)

/* See include/linux/random.h, lib/random32.c */
static inline u32 prandom_u32_max(u32 ep_ro)
{
	return (u32)(((u64) random32() * ep_ro) >> 32);
}


/* ----------------------------------------------------------
 > l2cap_core.c */

/* See include/linux/list.h */
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)


/* ----------------------------------------------------------
 > bnep/core.c */

/* See include/linux/etherdevice.h */
static inline bool ether_addr_equal(const u8 *addr1, const u8 *addr2)
{
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
	u32 fold = ((*(const u32 *)addr1) ^ (*(const u32 *)addr2)) |
		   ((*(const u16 *)(addr1 + 4)) ^ (*(const u16 *)(addr2 + 4)));

	return fold == 0;
#else
	const u16 *a = (const u16 *)addr1;
	const u16 *b = (const u16 *)addr2;

	return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) == 0;
#endif
}


#endif /* __BLUETOOTH_COMPAT_H__ */
