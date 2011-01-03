

#include <linux/errno.h>
#include <linux/string.h>

#include <rdma/ib_pack.h>

#define STRUCT_FIELD(header, field) \
	.struct_offset_bytes = offsetof(struct ib_unpacked_ ## header, field),      \
	.struct_size_bytes   = sizeof ((struct ib_unpacked_ ## header *) 0)->field, \
	.field_name          = #header ":" #field

static const struct ib_field lrh_table[]  = {
	{ STRUCT_FIELD(lrh, virtual_lane),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(lrh, link_version),
	  .offset_words = 0,
	  .offset_bits  = 4,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(lrh, service_level),
	  .offset_words = 0,
	  .offset_bits  = 8,
	  .size_bits    = 4 },
	{ RESERVED,
	  .offset_words = 0,
	  .offset_bits  = 12,
	  .size_bits    = 2 },
	{ STRUCT_FIELD(lrh, link_next_header),
	  .offset_words = 0,
	  .offset_bits  = 14,
	  .size_bits    = 2 },
	{ STRUCT_FIELD(lrh, destination_lid),
	  .offset_words = 0,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ RESERVED,
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 5 },
	{ STRUCT_FIELD(lrh, packet_length),
	  .offset_words = 1,
	  .offset_bits  = 5,
	  .size_bits    = 11 },
	{ STRUCT_FIELD(lrh, source_lid),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 16 }
};

static const struct ib_field grh_table[]  = {
	{ STRUCT_FIELD(grh, ip_version),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(grh, traffic_class),
	  .offset_words = 0,
	  .offset_bits  = 4,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(grh, flow_label),
	  .offset_words = 0,
	  .offset_bits  = 12,
	  .size_bits    = 20 },
	{ STRUCT_FIELD(grh, payload_length),
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 16 },
	{ STRUCT_FIELD(grh, next_header),
	  .offset_words = 1,
	  .offset_bits  = 16,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(grh, hop_limit),
	  .offset_words = 1,
	  .offset_bits  = 24,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(grh, source_gid),
	  .offset_words = 2,
	  .offset_bits  = 0,
	  .size_bits    = 128 },
	{ STRUCT_FIELD(grh, destination_gid),
	  .offset_words = 6,
	  .offset_bits  = 0,
	  .size_bits    = 128 }
};

static const struct ib_field bth_table[]  = {
	{ STRUCT_FIELD(bth, opcode),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(bth, solicited_event),
	  .offset_words = 0,
	  .offset_bits  = 8,
	  .size_bits    = 1 },
	{ STRUCT_FIELD(bth, mig_req),
	  .offset_words = 0,
	  .offset_bits  = 9,
	  .size_bits    = 1 },
	{ STRUCT_FIELD(bth, pad_count),
	  .offset_words = 0,
	  .offset_bits  = 10,
	  .size_bits    = 2 },
	{ STRUCT_FIELD(bth, transport_header_version),
	  .offset_words = 0,
	  .offset_bits  = 12,
	  .size_bits    = 4 },
	{ STRUCT_FIELD(bth, pkey),
	  .offset_words = 0,
	  .offset_bits  = 16,
	  .size_bits    = 16 },
	{ RESERVED,
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(bth, destination_qpn),
	  .offset_words = 1,
	  .offset_bits  = 8,
	  .size_bits    = 24 },
	{ STRUCT_FIELD(bth, ack_req),
	  .offset_words = 2,
	  .offset_bits  = 0,
	  .size_bits    = 1 },
	{ RESERVED,
	  .offset_words = 2,
	  .offset_bits  = 1,
	  .size_bits    = 7 },
	{ STRUCT_FIELD(bth, psn),
	  .offset_words = 2,
	  .offset_bits  = 8,
	  .size_bits    = 24 }
};

static const struct ib_field deth_table[] = {
	{ STRUCT_FIELD(deth, qkey),
	  .offset_words = 0,
	  .offset_bits  = 0,
	  .size_bits    = 32 },
	{ RESERVED,
	  .offset_words = 1,
	  .offset_bits  = 0,
	  .size_bits    = 8 },
	{ STRUCT_FIELD(deth, source_qpn),
	  .offset_words = 1,
	  .offset_bits  = 8,
	  .size_bits    = 24 }
};


void ib_ud_header_init(int     		    payload_bytes,
		       int    		    grh_present,
		       struct ib_ud_header *header)
{
	int header_len;
	u16 packet_length;

	memset(header, 0, sizeof *header);

	header_len =
		IB_LRH_BYTES  +
		IB_BTH_BYTES  +
		IB_DETH_BYTES;
	if (grh_present) {
		header_len += IB_GRH_BYTES;
	}

	header->lrh.link_version     = 0;
	header->lrh.link_next_header =
		grh_present ? IB_LNH_IBA_GLOBAL : IB_LNH_IBA_LOCAL;
	packet_length		     = (IB_LRH_BYTES     +
					IB_BTH_BYTES     +
					IB_DETH_BYTES    +
					payload_bytes    +
					4                + 
					3) / 4;            

	header->grh_present          = grh_present;
	if (grh_present) {
		packet_length		   += IB_GRH_BYTES / 4;
		header->grh.ip_version      = 6;
		header->grh.payload_length  =
			cpu_to_be16((IB_BTH_BYTES     +
				     IB_DETH_BYTES    +
				     payload_bytes    +
				     4                + 
				     3) & ~3);          
		header->grh.next_header     = 0x1b;
	}

	header->lrh.packet_length = cpu_to_be16(packet_length);

	if (header->immediate_present)
		header->bth.opcode           = IB_OPCODE_UD_SEND_ONLY_WITH_IMMEDIATE;
	else
		header->bth.opcode           = IB_OPCODE_UD_SEND_ONLY;
	header->bth.pad_count                = (4 - payload_bytes) & 3;
	header->bth.transport_header_version = 0;
}
EXPORT_SYMBOL(ib_ud_header_init);


int ib_ud_header_pack(struct ib_ud_header *header,
		      void                *buf)
{
	int len = 0;

	ib_pack(lrh_table, ARRAY_SIZE(lrh_table),
		&header->lrh, buf);
	len += IB_LRH_BYTES;

	if (header->grh_present) {
		ib_pack(grh_table, ARRAY_SIZE(grh_table),
			&header->grh, buf + len);
		len += IB_GRH_BYTES;
	}

	ib_pack(bth_table, ARRAY_SIZE(bth_table),
		&header->bth, buf + len);
	len += IB_BTH_BYTES;

	ib_pack(deth_table, ARRAY_SIZE(deth_table),
		&header->deth, buf + len);
	len += IB_DETH_BYTES;

	if (header->immediate_present) {
		memcpy(buf + len, &header->immediate_data, sizeof header->immediate_data);
		len += sizeof header->immediate_data;
	}

	return len;
}
EXPORT_SYMBOL(ib_ud_header_pack);


int ib_ud_header_unpack(void                *buf,
			struct ib_ud_header *header)
{
	ib_unpack(lrh_table, ARRAY_SIZE(lrh_table),
		  buf, &header->lrh);
	buf += IB_LRH_BYTES;

	if (header->lrh.link_version != 0) {
		printk(KERN_WARNING "Invalid LRH.link_version %d\n",
		       header->lrh.link_version);
		return -EINVAL;
	}

	switch (header->lrh.link_next_header) {
	case IB_LNH_IBA_LOCAL:
		header->grh_present = 0;
		break;

	case IB_LNH_IBA_GLOBAL:
		header->grh_present = 1;
		ib_unpack(grh_table, ARRAY_SIZE(grh_table),
			  buf, &header->grh);
		buf += IB_GRH_BYTES;

		if (header->grh.ip_version != 6) {
			printk(KERN_WARNING "Invalid GRH.ip_version %d\n",
			       header->grh.ip_version);
			return -EINVAL;
		}
		if (header->grh.next_header != 0x1b) {
			printk(KERN_WARNING "Invalid GRH.next_header 0x%02x\n",
			       header->grh.next_header);
			return -EINVAL;
		}
		break;

	default:
		printk(KERN_WARNING "Invalid LRH.link_next_header %d\n",
		       header->lrh.link_next_header);
		return -EINVAL;
	}

	ib_unpack(bth_table, ARRAY_SIZE(bth_table),
		  buf, &header->bth);
	buf += IB_BTH_BYTES;

	switch (header->bth.opcode) {
	case IB_OPCODE_UD_SEND_ONLY:
		header->immediate_present = 0;
		break;
	case IB_OPCODE_UD_SEND_ONLY_WITH_IMMEDIATE:
		header->immediate_present = 1;
		break;
	default:
		printk(KERN_WARNING "Invalid BTH.opcode 0x%02x\n",
		       header->bth.opcode);
		return -EINVAL;
	}

	if (header->bth.transport_header_version != 0) {
		printk(KERN_WARNING "Invalid BTH.transport_header_version %d\n",
		       header->bth.transport_header_version);
		return -EINVAL;
	}

	ib_unpack(deth_table, ARRAY_SIZE(deth_table),
		  buf, &header->deth);
	buf += IB_DETH_BYTES;

	if (header->immediate_present)
		memcpy(&header->immediate_data, buf, sizeof header->immediate_data);

	return 0;
}
EXPORT_SYMBOL(ib_ud_header_unpack);
