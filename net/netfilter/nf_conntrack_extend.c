
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <net/netfilter/nf_conntrack_extend.h>

static struct nf_ct_ext_type *nf_ct_ext_types[NF_CT_EXT_NUM];
static DEFINE_MUTEX(nf_ct_ext_type_mutex);

void __nf_ct_ext_destroy(struct nf_conn *ct)
{
	unsigned int i;
	struct nf_ct_ext_type *t;

	for (i = 0; i < NF_CT_EXT_NUM; i++) {
		if (!nf_ct_ext_exist(ct, i))
			continue;

		rcu_read_lock();
		t = rcu_dereference(nf_ct_ext_types[i]);

		
		if (t && t->destroy)
			t->destroy(ct);
		rcu_read_unlock();
	}
}
EXPORT_SYMBOL(__nf_ct_ext_destroy);

static void *
nf_ct_ext_create(struct nf_ct_ext **ext, enum nf_ct_ext_id id, gfp_t gfp)
{
	unsigned int off, len;
	struct nf_ct_ext_type *t;

	rcu_read_lock();
	t = rcu_dereference(nf_ct_ext_types[id]);
	BUG_ON(t == NULL);
	off = ALIGN(sizeof(struct nf_ct_ext), t->align);
	len = off + t->len;
	rcu_read_unlock();

	*ext = kzalloc(t->alloc_size, gfp);
	if (!*ext)
		return NULL;

	INIT_RCU_HEAD(&(*ext)->rcu);
	(*ext)->offset[id] = off;
	(*ext)->len = len;

	return (void *)(*ext) + off;
}

static void __nf_ct_ext_free_rcu(struct rcu_head *head)
{
	struct nf_ct_ext *ext = container_of(head, struct nf_ct_ext, rcu);
	kfree(ext);
}

void *__nf_ct_ext_add(struct nf_conn *ct, enum nf_ct_ext_id id, gfp_t gfp)
{
	struct nf_ct_ext *new;
	int i, newlen, newoff;
	struct nf_ct_ext_type *t;

	
	NF_CT_ASSERT(!nf_ct_is_confirmed(ct));

	if (!ct->ext)
		return nf_ct_ext_create(&ct->ext, id, gfp);

	if (nf_ct_ext_exist(ct, id))
		return NULL;

	rcu_read_lock();
	t = rcu_dereference(nf_ct_ext_types[id]);
	BUG_ON(t == NULL);

	newoff = ALIGN(ct->ext->len, t->align);
	newlen = newoff + t->len;
	rcu_read_unlock();

	new = __krealloc(ct->ext, newlen, gfp);
	if (!new)
		return NULL;

	if (new != ct->ext) {
		for (i = 0; i < NF_CT_EXT_NUM; i++) {
			if (!nf_ct_ext_exist(ct, i))
				continue;

			rcu_read_lock();
			t = rcu_dereference(nf_ct_ext_types[i]);
			if (t && t->move)
				t->move((void *)new + new->offset[i],
					(void *)ct->ext + ct->ext->offset[i]);
			rcu_read_unlock();
		}
		call_rcu(&ct->ext->rcu, __nf_ct_ext_free_rcu);
		ct->ext = new;
	}

	new->offset[id] = newoff;
	new->len = newlen;
	memset((void *)new + newoff, 0, newlen - newoff);
	return (void *)new + newoff;
}
EXPORT_SYMBOL(__nf_ct_ext_add);

static void update_alloc_size(struct nf_ct_ext_type *type)
{
	int i, j;
	struct nf_ct_ext_type *t1, *t2;
	enum nf_ct_ext_id min = 0, max = NF_CT_EXT_NUM - 1;

	
	if ((type->flags & NF_CT_EXT_F_PREALLOC) == 0) {
		min = type->id;
		max = type->id;
	}

	
	for (i = min; i <= max; i++) {
		t1 = nf_ct_ext_types[i];
		if (!t1)
			continue;

		t1->alloc_size = sizeof(struct nf_ct_ext)
				 + ALIGN(sizeof(struct nf_ct_ext), t1->align)
				 + t1->len;
		for (j = 0; j < NF_CT_EXT_NUM; j++) {
			t2 = nf_ct_ext_types[j];
			if (t2 == NULL || t2 == t1 ||
			    (t2->flags & NF_CT_EXT_F_PREALLOC) == 0)
				continue;

			t1->alloc_size = ALIGN(t1->alloc_size, t2->align)
					 + t2->len;
		}
	}
}


int nf_ct_extend_register(struct nf_ct_ext_type *type)
{
	int ret = 0;

	mutex_lock(&nf_ct_ext_type_mutex);
	if (nf_ct_ext_types[type->id]) {
		ret = -EBUSY;
		goto out;
	}

	
	type->alloc_size = ALIGN(sizeof(struct nf_ct_ext), type->align)
			   + type->len;
	rcu_assign_pointer(nf_ct_ext_types[type->id], type);
	update_alloc_size(type);
out:
	mutex_unlock(&nf_ct_ext_type_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(nf_ct_extend_register);


void nf_ct_extend_unregister(struct nf_ct_ext_type *type)
{
	mutex_lock(&nf_ct_ext_type_mutex);
	rcu_assign_pointer(nf_ct_ext_types[type->id], NULL);
	update_alloc_size(type);
	mutex_unlock(&nf_ct_ext_type_mutex);
	rcu_barrier(); 
}
EXPORT_SYMBOL_GPL(nf_ct_extend_unregister);
