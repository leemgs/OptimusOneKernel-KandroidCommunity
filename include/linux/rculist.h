#ifndef _LINUX_RCULIST_H
#define _LINUX_RCULIST_H

#ifdef __KERNEL__


#include <linux/list.h>
#include <linux/rcupdate.h>


static inline void __list_add_rcu(struct list_head *new,
		struct list_head *prev, struct list_head *next)
{
	new->next = next;
	new->prev = prev;
	rcu_assign_pointer(prev->next, new);
	next->prev = new;
}


static inline void list_add_rcu(struct list_head *new, struct list_head *head)
{
	__list_add_rcu(new, head, head->next);
}


static inline void list_add_tail_rcu(struct list_head *new,
					struct list_head *head)
{
	__list_add_rcu(new, head->prev, head);
}


static inline void list_del_rcu(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->prev = LIST_POISON2;
}


static inline void hlist_del_init_rcu(struct hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		__hlist_del(n);
		n->pprev = NULL;
	}
}


static inline void list_replace_rcu(struct list_head *old,
				struct list_head *new)
{
	new->next = old->next;
	new->prev = old->prev;
	rcu_assign_pointer(new->prev->next, new);
	new->next->prev = new;
	old->prev = LIST_POISON2;
}


static inline void list_splice_init_rcu(struct list_head *list,
					struct list_head *head,
					void (*sync)(void))
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;
	struct list_head *at = head->next;

	if (list_empty(head))
		return;

	

	INIT_LIST_HEAD(list);

	

	sync();

	

	last->next = at;
	rcu_assign_pointer(head->next, first);
	first->prev = head;
	at->prev = last;
}


#define list_entry_rcu(ptr, type, member) \
	container_of(rcu_dereference(ptr), type, member)


#define list_first_entry_rcu(ptr, type, member) \
	list_entry_rcu((ptr)->next, type, member)

#define __list_for_each_rcu(pos, head) \
	for (pos = rcu_dereference((head)->next); \
		pos != (head); \
		pos = rcu_dereference(pos->next))


#define list_for_each_entry_rcu(pos, head, member) \
	for (pos = list_entry_rcu((head)->next, typeof(*pos), member); \
		prefetch(pos->member.next), &pos->member != (head); \
		pos = list_entry_rcu(pos->member.next, typeof(*pos), member))



#define list_for_each_continue_rcu(pos, head) \
	for ((pos) = rcu_dereference((pos)->next); \
		prefetch((pos)->next), (pos) != (head); \
		(pos) = rcu_dereference((pos)->next))


static inline void hlist_del_rcu(struct hlist_node *n)
{
	__hlist_del(n);
	n->pprev = LIST_POISON2;
}


static inline void hlist_replace_rcu(struct hlist_node *old,
					struct hlist_node *new)
{
	struct hlist_node *next = old->next;

	new->next = next;
	new->pprev = old->pprev;
	rcu_assign_pointer(*new->pprev, new);
	if (next)
		new->next->pprev = &new->next;
	old->pprev = LIST_POISON2;
}


static inline void hlist_add_head_rcu(struct hlist_node *n,
					struct hlist_head *h)
{
	struct hlist_node *first = h->first;

	n->next = first;
	n->pprev = &h->first;
	rcu_assign_pointer(h->first, n);
	if (first)
		first->pprev = &n->next;
}


static inline void hlist_add_before_rcu(struct hlist_node *n,
					struct hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	rcu_assign_pointer(*(n->pprev), n);
	next->pprev = &n->next;
}


static inline void hlist_add_after_rcu(struct hlist_node *prev,
				       struct hlist_node *n)
{
	n->next = prev->next;
	n->pprev = &prev->next;
	rcu_assign_pointer(prev->next, n);
	if (n->next)
		n->next->pprev = &n->next;
}


#define hlist_for_each_entry_rcu(tpos, pos, head, member)		 \
	for (pos = rcu_dereference((head)->first);			 \
		pos && ({ prefetch(pos->next); 1; }) &&			 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1; }); \
		pos = rcu_dereference(pos->next))

#endif	
#endif
