



#include <linux/err.h>
#include <linux/crc32.h>
#include <linux/math64.h>
#include "ubi.h"

#ifdef CONFIG_MTD_UBI_DEBUG_PARANOID
static int paranoid_check_si(struct ubi_device *ubi, struct ubi_scan_info *si);
#else
#define paranoid_check_si(ubi, si) 0
#endif


static struct ubi_ec_hdr *ech;
static struct ubi_vid_hdr *vidh;


static int add_to_list(struct ubi_scan_info *si, int pnum, int ec,
		       struct list_head *list)
{
	struct ubi_scan_leb *seb;

	if (list == &si->free)
		dbg_bld("add to free: PEB %d, EC %d", pnum, ec);
	else if (list == &si->erase)
		dbg_bld("add to erase: PEB %d, EC %d", pnum, ec);
	else if (list == &si->corr) {
		dbg_bld("add to corrupted: PEB %d, EC %d", pnum, ec);
		si->corr_count += 1;
	} else if (list == &si->alien)
		dbg_bld("add to alien: PEB %d, EC %d", pnum, ec);
	else
		BUG();

	seb = kmalloc(sizeof(struct ubi_scan_leb), GFP_KERNEL);
	if (!seb)
		return -ENOMEM;

	seb->pnum = pnum;
	seb->ec = ec;
	list_add_tail(&seb->u.list, list);
	return 0;
}


static int validate_vid_hdr(const struct ubi_vid_hdr *vid_hdr,
			    const struct ubi_scan_volume *sv, int pnum)
{
	int vol_type = vid_hdr->vol_type;
	int vol_id = be32_to_cpu(vid_hdr->vol_id);
	int used_ebs = be32_to_cpu(vid_hdr->used_ebs);
	int data_pad = be32_to_cpu(vid_hdr->data_pad);

	if (sv->leb_count != 0) {
		int sv_vol_type;

		

		if (vol_id != sv->vol_id) {
			dbg_err("inconsistent vol_id");
			goto bad;
		}

		if (sv->vol_type == UBI_STATIC_VOLUME)
			sv_vol_type = UBI_VID_STATIC;
		else
			sv_vol_type = UBI_VID_DYNAMIC;

		if (vol_type != sv_vol_type) {
			dbg_err("inconsistent vol_type");
			goto bad;
		}

		if (used_ebs != sv->used_ebs) {
			dbg_err("inconsistent used_ebs");
			goto bad;
		}

		if (data_pad != sv->data_pad) {
			dbg_err("inconsistent data_pad");
			goto bad;
		}
	}

	return 0;

bad:
	ubi_err("inconsistent VID header at PEB %d", pnum);
	ubi_dbg_dump_vid_hdr(vid_hdr);
	ubi_dbg_dump_sv(sv);
	return -EINVAL;
}


static struct ubi_scan_volume *add_volume(struct ubi_scan_info *si, int vol_id,
					  int pnum,
					  const struct ubi_vid_hdr *vid_hdr)
{
	struct ubi_scan_volume *sv;
	struct rb_node **p = &si->volumes.rb_node, *parent = NULL;

	ubi_assert(vol_id == be32_to_cpu(vid_hdr->vol_id));

	
	while (*p) {
		parent = *p;
		sv = rb_entry(parent, struct ubi_scan_volume, rb);

		if (vol_id == sv->vol_id)
			return sv;

		if (vol_id > sv->vol_id)
			p = &(*p)->rb_left;
		else
			p = &(*p)->rb_right;
	}

	
	sv = kmalloc(sizeof(struct ubi_scan_volume), GFP_KERNEL);
	if (!sv)
		return ERR_PTR(-ENOMEM);

	sv->highest_lnum = sv->leb_count = 0;
	sv->vol_id = vol_id;
	sv->root = RB_ROOT;
	sv->used_ebs = be32_to_cpu(vid_hdr->used_ebs);
	sv->data_pad = be32_to_cpu(vid_hdr->data_pad);
	sv->compat = vid_hdr->compat;
	sv->vol_type = vid_hdr->vol_type == UBI_VID_DYNAMIC ? UBI_DYNAMIC_VOLUME
							    : UBI_STATIC_VOLUME;
	if (vol_id > si->highest_vol_id)
		si->highest_vol_id = vol_id;

	rb_link_node(&sv->rb, parent, p);
	rb_insert_color(&sv->rb, &si->volumes);
	si->vols_found += 1;
	dbg_bld("added volume %d", vol_id);
	return sv;
}


static int compare_lebs(struct ubi_device *ubi, const struct ubi_scan_leb *seb,
			int pnum, const struct ubi_vid_hdr *vid_hdr)
{
	void *buf;
	int len, err, second_is_newer, bitflips = 0, corrupted = 0;
	uint32_t data_crc, crc;
	struct ubi_vid_hdr *vh = NULL;
	unsigned long long sqnum2 = be64_to_cpu(vid_hdr->sqnum);

	if (sqnum2 == seb->sqnum) {
		
		ubi_err("unsupported on-flash UBI format\n");
		return -EINVAL;
	}

	
	second_is_newer = !!(sqnum2 > seb->sqnum);

	

	if (second_is_newer) {
		if (!vid_hdr->copy_flag) {
			
			dbg_bld("second PEB %d is newer, copy_flag is unset",
				pnum);
			return 1;
		}
	} else {
		pnum = seb->pnum;

		vh = ubi_zalloc_vid_hdr(ubi, GFP_KERNEL);
		if (!vh)
			return -ENOMEM;

		err = ubi_io_read_vid_hdr(ubi, pnum, vh, 0);
		if (err) {
			if (err == UBI_IO_BITFLIPS)
				bitflips = 1;
			else {
				dbg_err("VID of PEB %d header is bad, but it "
					"was OK earlier", pnum);
				if (err > 0)
					err = -EIO;

				goto out_free_vidh;
			}
		}

		if (!vh->copy_flag) {
			
			dbg_bld("first PEB %d is newer, copy_flag is unset",
				pnum);
			err = bitflips << 1;
			goto out_free_vidh;
		}

		vid_hdr = vh;
	}

	

	len = be32_to_cpu(vid_hdr->data_size);
	buf = vmalloc(len);
	if (!buf) {
		err = -ENOMEM;
		goto out_free_vidh;
	}

	err = ubi_io_read_data(ubi, buf, pnum, 0, len);
	if (err && err != UBI_IO_BITFLIPS && err != -EBADMSG)
		goto out_free_buf;

	data_crc = be32_to_cpu(vid_hdr->data_crc);
	crc = crc32(UBI_CRC32_INIT, buf, len);
	if (crc != data_crc) {
		dbg_bld("PEB %d CRC error: calculated %#08x, must be %#08x",
			pnum, crc, data_crc);
		corrupted = 1;
		bitflips = 0;
		second_is_newer = !second_is_newer;
	} else {
		dbg_bld("PEB %d CRC is OK", pnum);
		bitflips = !!err;
	}

	vfree(buf);
	ubi_free_vid_hdr(ubi, vh);

	if (second_is_newer)
		dbg_bld("second PEB %d is newer, copy_flag is set", pnum);
	else
		dbg_bld("first PEB %d is newer, copy_flag is set", pnum);

	return second_is_newer | (bitflips << 1) | (corrupted << 2);

out_free_buf:
	vfree(buf);
out_free_vidh:
	ubi_free_vid_hdr(ubi, vh);
	return err;
}


int ubi_scan_add_used(struct ubi_device *ubi, struct ubi_scan_info *si,
		      int pnum, int ec, const struct ubi_vid_hdr *vid_hdr,
		      int bitflips)
{
	int err, vol_id, lnum;
	unsigned long long sqnum;
	struct ubi_scan_volume *sv;
	struct ubi_scan_leb *seb;
	struct rb_node **p, *parent = NULL;

	vol_id = be32_to_cpu(vid_hdr->vol_id);
	lnum = be32_to_cpu(vid_hdr->lnum);
	sqnum = be64_to_cpu(vid_hdr->sqnum);

	dbg_bld("PEB %d, LEB %d:%d, EC %d, sqnum %llu, bitflips %d",
		pnum, vol_id, lnum, ec, sqnum, bitflips);

	sv = add_volume(si, vol_id, pnum, vid_hdr);
	if (IS_ERR(sv))
		return PTR_ERR(sv);

	if (si->max_sqnum < sqnum)
		si->max_sqnum = sqnum;

	
	p = &sv->root.rb_node;
	while (*p) {
		int cmp_res;

		parent = *p;
		seb = rb_entry(parent, struct ubi_scan_leb, u.rb);
		if (lnum != seb->lnum) {
			if (lnum < seb->lnum)
				p = &(*p)->rb_left;
			else
				p = &(*p)->rb_right;
			continue;
		}

		

		dbg_bld("this LEB already exists: PEB %d, sqnum %llu, "
			"EC %d", seb->pnum, seb->sqnum, seb->ec);

		
		if (seb->sqnum == sqnum && sqnum != 0) {
			ubi_err("two LEBs with same sequence number %llu",
				sqnum);
			ubi_dbg_dump_seb(seb, 0);
			ubi_dbg_dump_vid_hdr(vid_hdr);
			return -EINVAL;
		}

		
		cmp_res = compare_lebs(ubi, seb, pnum, vid_hdr);
		if (cmp_res < 0)
			return cmp_res;

		if (cmp_res & 1) {
			
			err = validate_vid_hdr(vid_hdr, sv, pnum);
			if (err)
				return err;

			if (cmp_res & 4)
				err = add_to_list(si, seb->pnum, seb->ec,
						  &si->corr);
			else
				err = add_to_list(si, seb->pnum, seb->ec,
						  &si->erase);
			if (err)
				return err;

			seb->ec = ec;
			seb->pnum = pnum;
			seb->scrub = ((cmp_res & 2) || bitflips);
			seb->sqnum = sqnum;

			if (sv->highest_lnum == lnum)
				sv->last_data_size =
					be32_to_cpu(vid_hdr->data_size);

			return 0;
		} else {
			
			if (cmp_res & 4)
				return add_to_list(si, pnum, ec, &si->corr);
			else
				return add_to_list(si, pnum, ec, &si->erase);
		}
	}

	

	err = validate_vid_hdr(vid_hdr, sv, pnum);
	if (err)
		return err;

	seb = kmalloc(sizeof(struct ubi_scan_leb), GFP_KERNEL);
	if (!seb)
		return -ENOMEM;

	seb->ec = ec;
	seb->pnum = pnum;
	seb->lnum = lnum;
	seb->sqnum = sqnum;
	seb->scrub = bitflips;

	if (sv->highest_lnum <= lnum) {
		sv->highest_lnum = lnum;
		sv->last_data_size = be32_to_cpu(vid_hdr->data_size);
	}

	sv->leb_count += 1;
	rb_link_node(&seb->u.rb, parent, p);
	rb_insert_color(&seb->u.rb, &sv->root);
	return 0;
}


struct ubi_scan_volume *ubi_scan_find_sv(const struct ubi_scan_info *si,
					 int vol_id)
{
	struct ubi_scan_volume *sv;
	struct rb_node *p = si->volumes.rb_node;

	while (p) {
		sv = rb_entry(p, struct ubi_scan_volume, rb);

		if (vol_id == sv->vol_id)
			return sv;

		if (vol_id > sv->vol_id)
			p = p->rb_left;
		else
			p = p->rb_right;
	}

	return NULL;
}


struct ubi_scan_leb *ubi_scan_find_seb(const struct ubi_scan_volume *sv,
				       int lnum)
{
	struct ubi_scan_leb *seb;
	struct rb_node *p = sv->root.rb_node;

	while (p) {
		seb = rb_entry(p, struct ubi_scan_leb, u.rb);

		if (lnum == seb->lnum)
			return seb;

		if (lnum > seb->lnum)
			p = p->rb_left;
		else
			p = p->rb_right;
	}

	return NULL;
}


void ubi_scan_rm_volume(struct ubi_scan_info *si, struct ubi_scan_volume *sv)
{
	struct rb_node *rb;
	struct ubi_scan_leb *seb;

	dbg_bld("remove scanning information about volume %d", sv->vol_id);

	while ((rb = rb_first(&sv->root))) {
		seb = rb_entry(rb, struct ubi_scan_leb, u.rb);
		rb_erase(&seb->u.rb, &sv->root);
		list_add_tail(&seb->u.list, &si->erase);
	}

	rb_erase(&sv->rb, &si->volumes);
	kfree(sv);
	si->vols_found -= 1;
}


int ubi_scan_erase_peb(struct ubi_device *ubi, const struct ubi_scan_info *si,
		       int pnum, int ec)
{
	int err;
	struct ubi_ec_hdr *ec_hdr;

	if ((long long)ec >= UBI_MAX_ERASECOUNTER) {
		
		ubi_err("erase counter overflow at PEB %d, EC %d", pnum, ec);
		return -EINVAL;
	}

	ec_hdr = kzalloc(ubi->ec_hdr_alsize, GFP_KERNEL);
	if (!ec_hdr)
		return -ENOMEM;

	ec_hdr->ec = cpu_to_be64(ec);

	err = ubi_io_sync_erase(ubi, pnum, 0);
	if (err < 0)
		goto out_free;

	err = ubi_io_write_ec_hdr(ubi, pnum, ec_hdr);

out_free:
	kfree(ec_hdr);
	return err;
}


struct ubi_scan_leb *ubi_scan_get_free_peb(struct ubi_device *ubi,
					   struct ubi_scan_info *si)
{
	int err = 0, i;
	struct ubi_scan_leb *seb;

	if (!list_empty(&si->free)) {
		seb = list_entry(si->free.next, struct ubi_scan_leb, u.list);
		list_del(&seb->u.list);
		dbg_bld("return free PEB %d, EC %d", seb->pnum, seb->ec);
		return seb;
	}

	for (i = 0; i < 2; i++) {
		struct list_head *head;
		struct ubi_scan_leb *tmp_seb;

		if (i == 0)
			head = &si->erase;
		else
			head = &si->corr;

		
		list_for_each_entry_safe(seb, tmp_seb, head, u.list) {
			if (seb->ec == UBI_SCAN_UNKNOWN_EC)
				seb->ec = si->mean_ec;

			err = ubi_scan_erase_peb(ubi, si, seb->pnum, seb->ec+1);
			if (err)
				continue;

			seb->ec += 1;
			list_del(&seb->u.list);
			dbg_bld("return PEB %d, EC %d", seb->pnum, seb->ec);
			return seb;
		}
	}

	ubi_err("no eraseblocks found");
	return ERR_PTR(-ENOSPC);
}


static int process_eb(struct ubi_device *ubi, struct ubi_scan_info *si,
		      int pnum)
{
	long long uninitialized_var(ec);
	int err, bitflips = 0, vol_id, ec_corr = 0;

	dbg_bld("scan PEB %d", pnum);

	
	err = ubi_io_is_bad(ubi, pnum);
	if (err < 0)
		return err;
	else if (err) {
		
		si->bad_peb_count += 1;
		return 0;
	}

	err = ubi_io_read_ec_hdr(ubi, pnum, ech, 0);
	if (err < 0)
		return err;
	else if (err == UBI_IO_BITFLIPS)
		bitflips = 1;
	else if (err == UBI_IO_PEB_EMPTY)
		return add_to_list(si, pnum, UBI_SCAN_UNKNOWN_EC, &si->erase);
	else if (err == UBI_IO_BAD_EC_HDR) {
		
		ec_corr = 1;
		ec = UBI_SCAN_UNKNOWN_EC;
		bitflips = 1;
	}

	si->is_empty = 0;

	if (!ec_corr) {
		int image_seq;

		
		if (ech->version != UBI_VERSION) {
			ubi_err("this UBI version is %d, image version is %d",
				UBI_VERSION, (int)ech->version);
			return -EINVAL;
		}

		ec = be64_to_cpu(ech->ec);
		if (ec > UBI_MAX_ERASECOUNTER) {
			
			ubi_err("erase counter overflow, max is %d",
				UBI_MAX_ERASECOUNTER);
			ubi_dbg_dump_ec_hdr(ech);
			return -EINVAL;
		}

		
		image_seq = be32_to_cpu(ech->image_seq);
		if (!ubi->image_seq && image_seq)
			ubi->image_seq = image_seq;
		if (ubi->image_seq && image_seq &&
		    ubi->image_seq != image_seq) {
			ubi_err("bad image sequence number %d in PEB %d, "
				"expected %d", image_seq, pnum, ubi->image_seq);
			ubi_dbg_dump_ec_hdr(ech);
			return -EINVAL;
		}
	}

	

	err = ubi_io_read_vid_hdr(ubi, pnum, vidh, 0);
	if (err < 0)
		return err;
	else if (err == UBI_IO_BITFLIPS)
		bitflips = 1;
	else if (err == UBI_IO_BAD_VID_HDR ||
		 (err == UBI_IO_PEB_FREE && ec_corr)) {
		
		err = add_to_list(si, pnum, ec, &si->corr);
		if (err)
			return err;
		goto adjust_mean_ec;
	} else if (err == UBI_IO_PEB_FREE) {
		
		err = add_to_list(si, pnum, ec, &si->free);
		if (err)
			return err;
		goto adjust_mean_ec;
	}

	vol_id = be32_to_cpu(vidh->vol_id);
	if (vol_id > UBI_MAX_VOLUMES && vol_id != UBI_LAYOUT_VOLUME_ID) {
		int lnum = be32_to_cpu(vidh->lnum);

		
		switch (vidh->compat) {
		case UBI_COMPAT_DELETE:
			ubi_msg("\"delete\" compatible internal volume %d:%d"
				" found, remove it", vol_id, lnum);
			err = add_to_list(si, pnum, ec, &si->corr);
			if (err)
				return err;
			break;

		case UBI_COMPAT_RO:
			ubi_msg("read-only compatible internal volume %d:%d"
				" found, switch to read-only mode",
				vol_id, lnum);
			ubi->ro_mode = 1;
			break;

		case UBI_COMPAT_PRESERVE:
			ubi_msg("\"preserve\" compatible internal volume %d:%d"
				" found", vol_id, lnum);
			err = add_to_list(si, pnum, ec, &si->alien);
			if (err)
				return err;
			si->alien_peb_count += 1;
			return 0;

		case UBI_COMPAT_REJECT:
			ubi_err("incompatible internal volume %d:%d found",
				vol_id, lnum);
			return -EINVAL;
		}
	}

	if (ec_corr)
		ubi_warn("valid VID header but corrupted EC header at PEB %d",
			 pnum);
	err = ubi_scan_add_used(ubi, si, pnum, ec, vidh, bitflips);
	if (err)
		return err;

adjust_mean_ec:
	if (!ec_corr) {
		si->ec_sum += ec;
		si->ec_count += 1;
		if (ec > si->max_ec)
			si->max_ec = ec;
		if (ec < si->min_ec)
			si->min_ec = ec;
	}

	return 0;
}


struct ubi_scan_info *ubi_scan(struct ubi_device *ubi)
{
	int err, pnum;
	struct rb_node *rb1, *rb2;
	struct ubi_scan_volume *sv;
	struct ubi_scan_leb *seb;
	struct ubi_scan_info *si;

	si = kzalloc(sizeof(struct ubi_scan_info), GFP_KERNEL);
	if (!si)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&si->corr);
	INIT_LIST_HEAD(&si->free);
	INIT_LIST_HEAD(&si->erase);
	INIT_LIST_HEAD(&si->alien);
	si->volumes = RB_ROOT;
	si->is_empty = 1;

	err = -ENOMEM;
	ech = kzalloc(ubi->ec_hdr_alsize, GFP_KERNEL);
	if (!ech)
		goto out_si;

	vidh = ubi_zalloc_vid_hdr(ubi, GFP_KERNEL);
	if (!vidh)
		goto out_ech;

	for (pnum = 0; pnum < ubi->peb_count; pnum++) {
		cond_resched();

		dbg_gen("process PEB %d", pnum);
		err = process_eb(ubi, si, pnum);
		if (err < 0)
			goto out_vidh;
	}

	dbg_msg("scanning is finished");

	
	if (si->ec_count)
		si->mean_ec = div_u64(si->ec_sum, si->ec_count);

	if (si->is_empty)
		ubi_msg("empty MTD device detected");

	
	if (si->corr_count >= 8 || si->corr_count >= ubi->peb_count / 4) {
		ubi_warn("%d PEBs are corrupted", si->corr_count);
		printk(KERN_WARNING "corrupted PEBs are:");
		list_for_each_entry(seb, &si->corr, u.list)
			printk(KERN_CONT " %d", seb->pnum);
		printk(KERN_CONT "\n");
	}

	
	ubi_rb_for_each_entry(rb1, sv, &si->volumes, rb) {
		ubi_rb_for_each_entry(rb2, seb, &sv->root, u.rb)
			if (seb->ec == UBI_SCAN_UNKNOWN_EC)
				seb->ec = si->mean_ec;
	}

	list_for_each_entry(seb, &si->free, u.list) {
		if (seb->ec == UBI_SCAN_UNKNOWN_EC)
			seb->ec = si->mean_ec;
	}

	list_for_each_entry(seb, &si->corr, u.list)
		if (seb->ec == UBI_SCAN_UNKNOWN_EC)
			seb->ec = si->mean_ec;

	list_for_each_entry(seb, &si->erase, u.list)
		if (seb->ec == UBI_SCAN_UNKNOWN_EC)
			seb->ec = si->mean_ec;

	err = paranoid_check_si(ubi, si);
	if (err) {
		if (err > 0)
			err = -EINVAL;
		goto out_vidh;
	}

	ubi_free_vid_hdr(ubi, vidh);
	kfree(ech);

	return si;

out_vidh:
	ubi_free_vid_hdr(ubi, vidh);
out_ech:
	kfree(ech);
out_si:
	ubi_scan_destroy_si(si);
	return ERR_PTR(err);
}


static void destroy_sv(struct ubi_scan_volume *sv)
{
	struct ubi_scan_leb *seb;
	struct rb_node *this = sv->root.rb_node;

	while (this) {
		if (this->rb_left)
			this = this->rb_left;
		else if (this->rb_right)
			this = this->rb_right;
		else {
			seb = rb_entry(this, struct ubi_scan_leb, u.rb);
			this = rb_parent(this);
			if (this) {
				if (this->rb_left == &seb->u.rb)
					this->rb_left = NULL;
				else
					this->rb_right = NULL;
			}

			kfree(seb);
		}
	}
	kfree(sv);
}


void ubi_scan_destroy_si(struct ubi_scan_info *si)
{
	struct ubi_scan_leb *seb, *seb_tmp;
	struct ubi_scan_volume *sv;
	struct rb_node *rb;

	list_for_each_entry_safe(seb, seb_tmp, &si->alien, u.list) {
		list_del(&seb->u.list);
		kfree(seb);
	}
	list_for_each_entry_safe(seb, seb_tmp, &si->erase, u.list) {
		list_del(&seb->u.list);
		kfree(seb);
	}
	list_for_each_entry_safe(seb, seb_tmp, &si->corr, u.list) {
		list_del(&seb->u.list);
		kfree(seb);
	}
	list_for_each_entry_safe(seb, seb_tmp, &si->free, u.list) {
		list_del(&seb->u.list);
		kfree(seb);
	}

	
	rb = si->volumes.rb_node;
	while (rb) {
		if (rb->rb_left)
			rb = rb->rb_left;
		else if (rb->rb_right)
			rb = rb->rb_right;
		else {
			sv = rb_entry(rb, struct ubi_scan_volume, rb);

			rb = rb_parent(rb);
			if (rb) {
				if (rb->rb_left == &sv->rb)
					rb->rb_left = NULL;
				else
					rb->rb_right = NULL;
			}

			destroy_sv(sv);
		}
	}

	kfree(si);
}

#ifdef CONFIG_MTD_UBI_DEBUG_PARANOID


static int paranoid_check_si(struct ubi_device *ubi, struct ubi_scan_info *si)
{
	int pnum, err, vols_found = 0;
	struct rb_node *rb1, *rb2;
	struct ubi_scan_volume *sv;
	struct ubi_scan_leb *seb, *last_seb;
	uint8_t *buf;

	
	ubi_rb_for_each_entry(rb1, sv, &si->volumes, rb) {
		int leb_count = 0;

		cond_resched();

		vols_found += 1;

		if (si->is_empty) {
			ubi_err("bad is_empty flag");
			goto bad_sv;
		}

		if (sv->vol_id < 0 || sv->highest_lnum < 0 ||
		    sv->leb_count < 0 || sv->vol_type < 0 || sv->used_ebs < 0 ||
		    sv->data_pad < 0 || sv->last_data_size < 0) {
			ubi_err("negative values");
			goto bad_sv;
		}

		if (sv->vol_id >= UBI_MAX_VOLUMES &&
		    sv->vol_id < UBI_INTERNAL_VOL_START) {
			ubi_err("bad vol_id");
			goto bad_sv;
		}

		if (sv->vol_id > si->highest_vol_id) {
			ubi_err("highest_vol_id is %d, but vol_id %d is there",
				si->highest_vol_id, sv->vol_id);
			goto out;
		}

		if (sv->vol_type != UBI_DYNAMIC_VOLUME &&
		    sv->vol_type != UBI_STATIC_VOLUME) {
			ubi_err("bad vol_type");
			goto bad_sv;
		}

		if (sv->data_pad > ubi->leb_size / 2) {
			ubi_err("bad data_pad");
			goto bad_sv;
		}

		last_seb = NULL;
		ubi_rb_for_each_entry(rb2, seb, &sv->root, u.rb) {
			cond_resched();

			last_seb = seb;
			leb_count += 1;

			if (seb->pnum < 0 || seb->ec < 0) {
				ubi_err("negative values");
				goto bad_seb;
			}

			if (seb->ec < si->min_ec) {
				ubi_err("bad si->min_ec (%d), %d found",
					si->min_ec, seb->ec);
				goto bad_seb;
			}

			if (seb->ec > si->max_ec) {
				ubi_err("bad si->max_ec (%d), %d found",
					si->max_ec, seb->ec);
				goto bad_seb;
			}

			if (seb->pnum >= ubi->peb_count) {
				ubi_err("too high PEB number %d, total PEBs %d",
					seb->pnum, ubi->peb_count);
				goto bad_seb;
			}

			if (sv->vol_type == UBI_STATIC_VOLUME) {
				if (seb->lnum >= sv->used_ebs) {
					ubi_err("bad lnum or used_ebs");
					goto bad_seb;
				}
			} else {
				if (sv->used_ebs != 0) {
					ubi_err("non-zero used_ebs");
					goto bad_seb;
				}
			}

			if (seb->lnum > sv->highest_lnum) {
				ubi_err("incorrect highest_lnum or lnum");
				goto bad_seb;
			}
		}

		if (sv->leb_count != leb_count) {
			ubi_err("bad leb_count, %d objects in the tree",
				leb_count);
			goto bad_sv;
		}

		if (!last_seb)
			continue;

		seb = last_seb;

		if (seb->lnum != sv->highest_lnum) {
			ubi_err("bad highest_lnum");
			goto bad_seb;
		}
	}

	if (vols_found != si->vols_found) {
		ubi_err("bad si->vols_found %d, should be %d",
			si->vols_found, vols_found);
		goto out;
	}

	
	ubi_rb_for_each_entry(rb1, sv, &si->volumes, rb) {
		last_seb = NULL;
		ubi_rb_for_each_entry(rb2, seb, &sv->root, u.rb) {
			int vol_type;

			cond_resched();

			last_seb = seb;

			err = ubi_io_read_vid_hdr(ubi, seb->pnum, vidh, 1);
			if (err && err != UBI_IO_BITFLIPS) {
				ubi_err("VID header is not OK (%d)", err);
				if (err > 0)
					err = -EIO;
				return err;
			}

			vol_type = vidh->vol_type == UBI_VID_DYNAMIC ?
				   UBI_DYNAMIC_VOLUME : UBI_STATIC_VOLUME;
			if (sv->vol_type != vol_type) {
				ubi_err("bad vol_type");
				goto bad_vid_hdr;
			}

			if (seb->sqnum != be64_to_cpu(vidh->sqnum)) {
				ubi_err("bad sqnum %llu", seb->sqnum);
				goto bad_vid_hdr;
			}

			if (sv->vol_id != be32_to_cpu(vidh->vol_id)) {
				ubi_err("bad vol_id %d", sv->vol_id);
				goto bad_vid_hdr;
			}

			if (sv->compat != vidh->compat) {
				ubi_err("bad compat %d", vidh->compat);
				goto bad_vid_hdr;
			}

			if (seb->lnum != be32_to_cpu(vidh->lnum)) {
				ubi_err("bad lnum %d", seb->lnum);
				goto bad_vid_hdr;
			}

			if (sv->used_ebs != be32_to_cpu(vidh->used_ebs)) {
				ubi_err("bad used_ebs %d", sv->used_ebs);
				goto bad_vid_hdr;
			}

			if (sv->data_pad != be32_to_cpu(vidh->data_pad)) {
				ubi_err("bad data_pad %d", sv->data_pad);
				goto bad_vid_hdr;
			}
		}

		if (!last_seb)
			continue;

		if (sv->highest_lnum != be32_to_cpu(vidh->lnum)) {
			ubi_err("bad highest_lnum %d", sv->highest_lnum);
			goto bad_vid_hdr;
		}

		if (sv->last_data_size != be32_to_cpu(vidh->data_size)) {
			ubi_err("bad last_data_size %d", sv->last_data_size);
			goto bad_vid_hdr;
		}
	}

	
	buf = kzalloc(ubi->peb_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	for (pnum = 0; pnum < ubi->peb_count; pnum++) {
		err = ubi_io_is_bad(ubi, pnum);
		if (err < 0) {
			kfree(buf);
			return err;
		} else if (err)
			buf[pnum] = 1;
	}

	ubi_rb_for_each_entry(rb1, sv, &si->volumes, rb)
		ubi_rb_for_each_entry(rb2, seb, &sv->root, u.rb)
			buf[seb->pnum] = 1;

	list_for_each_entry(seb, &si->free, u.list)
		buf[seb->pnum] = 1;

	list_for_each_entry(seb, &si->corr, u.list)
		buf[seb->pnum] = 1;

	list_for_each_entry(seb, &si->erase, u.list)
		buf[seb->pnum] = 1;

	list_for_each_entry(seb, &si->alien, u.list)
		buf[seb->pnum] = 1;

	err = 0;
	for (pnum = 0; pnum < ubi->peb_count; pnum++)
		if (!buf[pnum]) {
			ubi_err("PEB %d is not referred", pnum);
			err = 1;
		}

	kfree(buf);
	if (err)
		goto out;
	return 0;

bad_seb:
	ubi_err("bad scanning information about LEB %d", seb->lnum);
	ubi_dbg_dump_seb(seb, 0);
	ubi_dbg_dump_sv(sv);
	goto out;

bad_sv:
	ubi_err("bad scanning information about volume %d", sv->vol_id);
	ubi_dbg_dump_sv(sv);
	goto out;

bad_vid_hdr:
	ubi_err("bad scanning information about volume %d", sv->vol_id);
	ubi_dbg_dump_sv(sv);
	ubi_dbg_dump_vid_hdr(vidh);

out:
	ubi_dbg_dump_stack();
	return 1;
}

#endif 
