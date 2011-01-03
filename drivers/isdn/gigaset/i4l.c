

#include "gigaset.h"




static int writebuf_from_LL(int driverID, int channel, int ack,
			    struct sk_buff *skb)
{
	struct cardstate *cs;
	struct bc_state *bcs;
	unsigned len;
	unsigned skblen;

	if (!(cs = gigaset_get_cs_by_id(driverID))) {
		pr_err("%s: invalid driver ID (%d)\n", __func__, driverID);
		return -ENODEV;
	}
	if (channel < 0 || channel >= cs->channels) {
		dev_err(cs->dev, "%s: invalid channel ID (%d)\n",
			__func__, channel);
		return -ENODEV;
	}
	bcs = &cs->bcs[channel];

	
	if (skb_linearize(skb) < 0) {
		dev_err(cs->dev, "%s: skb_linearize failed\n", __func__);
		return -ENOMEM;
	}
	len = skb->len;

	gig_dbg(DEBUG_LLDATA,
		"Receiving data from LL (id: %d, ch: %d, ack: %d, sz: %d)",
		driverID, channel, ack, len);

	if (!len) {
		if (ack)
			dev_notice(cs->dev, "%s: not ACKing empty packet\n",
				   __func__);
		return 0;
	}
	if (len > MAX_BUF_SIZE) {
		dev_err(cs->dev, "%s: packet too large (%d bytes)\n",
			__func__, len);
		return -EINVAL;
	}

	skblen = ack ? len : 0;
	skb->head[0] = skblen & 0xff;
	skb->head[1] = skblen >> 8;
	gig_dbg(DEBUG_MCMD, "skb: len=%u, skblen=%u: %02x %02x",
		len, skblen, (unsigned) skb->head[0], (unsigned) skb->head[1]);

	
	return cs->ops->send_skb(bcs, skb);
}


void gigaset_skb_sent(struct bc_state *bcs, struct sk_buff *skb)
{
	unsigned len;
	isdn_ctrl response;

	++bcs->trans_up;

	if (skb->len)
		dev_warn(bcs->cs->dev, "%s: skb->len==%d\n",
			 __func__, skb->len);

	len = (unsigned char) skb->head[0] |
	      (unsigned) (unsigned char) skb->head[1] << 8;
	if (len) {
		gig_dbg(DEBUG_MCMD, "ACKing to LL (id: %d, ch: %d, sz: %u)",
			bcs->cs->myid, bcs->channel, len);

		response.driver = bcs->cs->myid;
		response.command = ISDN_STAT_BSENT;
		response.arg = bcs->channel;
		response.parm.length = len;
		bcs->cs->iif.statcallb(&response);
	}
}
EXPORT_SYMBOL_GPL(gigaset_skb_sent);


static int command_from_LL(isdn_ctrl *cntrl)
{
	struct cardstate *cs = gigaset_get_cs_by_id(cntrl->driver);
	struct bc_state *bcs;
	int retval = 0;
	struct setup_parm *sp;

	gigaset_debugdrivers();

	if (!cs) {
		pr_err("%s: invalid driver ID (%d)\n", __func__, cntrl->driver);
		return -ENODEV;
	}

	switch (cntrl->command) {
	case ISDN_CMD_IOCTL:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_IOCTL (driver: %d, arg: %ld)",
			cntrl->driver, cntrl->arg);

		dev_warn(cs->dev, "ISDN_CMD_IOCTL not supported\n");
		return -EINVAL;

	case ISDN_CMD_DIAL:
		gig_dbg(DEBUG_ANY,
			"ISDN_CMD_DIAL (driver: %d, ch: %ld, "
			"phone: %s, ownmsn: %s, si1: %d, si2: %d)",
			cntrl->driver, cntrl->arg,
			cntrl->parm.setup.phone, cntrl->parm.setup.eazmsn,
			cntrl->parm.setup.si1, cntrl->parm.setup.si2);

		if (cntrl->arg >= cs->channels) {
			dev_err(cs->dev,
				"ISDN_CMD_DIAL: invalid channel (%d)\n",
				(int) cntrl->arg);
			return -EINVAL;
		}

		bcs = cs->bcs + cntrl->arg;

		if (!gigaset_get_channel(bcs)) {
			dev_err(cs->dev, "ISDN_CMD_DIAL: channel not free\n");
			return -EBUSY;
		}

		sp = kmalloc(sizeof *sp, GFP_ATOMIC);
		if (!sp) {
			gigaset_free_channel(bcs);
			dev_err(cs->dev, "ISDN_CMD_DIAL: out of memory\n");
			return -ENOMEM;
		}
		*sp = cntrl->parm.setup;

		if (!gigaset_add_event(cs, &bcs->at_state, EV_DIAL, sp,
				       bcs->at_state.seq_index, NULL)) {
			
			kfree(sp);
			gigaset_free_channel(bcs);
			return -ENOMEM;
		}

		gig_dbg(DEBUG_CMD, "scheduling DIAL");
		gigaset_schedule_event(cs);
		break;
	case ISDN_CMD_ACCEPTD: 
		gig_dbg(DEBUG_ANY, "ISDN_CMD_ACCEPTD");

		if (cntrl->arg >= cs->channels) {
			dev_err(cs->dev,
				"ISDN_CMD_ACCEPTD: invalid channel (%d)\n",
				(int) cntrl->arg);
			return -EINVAL;
		}

		if (!gigaset_add_event(cs, &cs->bcs[cntrl->arg].at_state,
				       EV_ACCEPT, NULL, 0, NULL)) {
			
			return -ENOMEM;
		}

		gig_dbg(DEBUG_CMD, "scheduling ACCEPT");
		gigaset_schedule_event(cs);

		break;
	case ISDN_CMD_ACCEPTB:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_ACCEPTB");
		break;
	case ISDN_CMD_HANGUP:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_HANGUP (ch: %d)",
			(int) cntrl->arg);

		if (cntrl->arg >= cs->channels) {
			dev_err(cs->dev,
				"ISDN_CMD_HANGUP: invalid channel (%d)\n",
				(int) cntrl->arg);
			return -EINVAL;
		}

		if (!gigaset_add_event(cs, &cs->bcs[cntrl->arg].at_state,
				       EV_HUP, NULL, 0, NULL)) {
			
			return -ENOMEM;
		}

		gig_dbg(DEBUG_CMD, "scheduling HUP");
		gigaset_schedule_event(cs);

		break;
	case ISDN_CMD_CLREAZ:  
		gig_dbg(DEBUG_ANY, "ISDN_CMD_CLREAZ");
		break;
	case ISDN_CMD_SETEAZ:  
		gig_dbg(DEBUG_ANY,
			"ISDN_CMD_SETEAZ (id: %d, ch: %ld, number: %s)",
			cntrl->driver, cntrl->arg, cntrl->parm.num);
		break;
	case ISDN_CMD_SETL2: 
		gig_dbg(DEBUG_ANY, "ISDN_CMD_SETL2 (ch: %ld, proto: %lx)",
			cntrl->arg & 0xff, (cntrl->arg >> 8));

		if ((cntrl->arg & 0xff) >= cs->channels) {
			dev_err(cs->dev,
				"ISDN_CMD_SETL2: invalid channel (%d)\n",
				(int) cntrl->arg & 0xff);
			return -EINVAL;
		}

		if (!gigaset_add_event(cs, &cs->bcs[cntrl->arg & 0xff].at_state,
				       EV_PROTO_L2, NULL, cntrl->arg >> 8,
				       NULL)) {
			
			return -ENOMEM;
		}

		gig_dbg(DEBUG_CMD, "scheduling PROTO_L2");
		gigaset_schedule_event(cs);
		break;
	case ISDN_CMD_SETL3: 
		gig_dbg(DEBUG_ANY, "ISDN_CMD_SETL3 (ch: %ld, proto: %lx)",
			cntrl->arg & 0xff, (cntrl->arg >> 8));

		if ((cntrl->arg & 0xff) >= cs->channels) {
			dev_err(cs->dev,
				"ISDN_CMD_SETL3: invalid channel (%d)\n",
				(int) cntrl->arg & 0xff);
			return -EINVAL;
		}

		if (cntrl->arg >> 8 != ISDN_PROTO_L3_TRANS) {
			dev_err(cs->dev,
				"ISDN_CMD_SETL3: invalid protocol %lu\n",
				cntrl->arg >> 8);
			return -EINVAL;
		}

		break;
	case ISDN_CMD_PROCEED:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_PROCEED"); 
		break;
	case ISDN_CMD_ALERT:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_ALERT"); 
		if (cntrl->arg >= cs->channels) {
			dev_err(cs->dev,
				"ISDN_CMD_ALERT: invalid channel (%d)\n",
				(int) cntrl->arg);
			return -EINVAL;
		}
		
		
		
		break;
	case ISDN_CMD_REDIR:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_REDIR"); 
		break;
	case ISDN_CMD_PROT_IO:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_PROT_IO");
		break;
	case ISDN_CMD_FAXCMD:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_FAXCMD");
		break;
	case ISDN_CMD_GETL2:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_GETL2");
		break;
	case ISDN_CMD_GETL3:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_GETL3");
		break;
	case ISDN_CMD_GETEAZ:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_GETEAZ");
		break;
	case ISDN_CMD_SETSIL:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_SETSIL");
		break;
	case ISDN_CMD_GETSIL:
		gig_dbg(DEBUG_ANY, "ISDN_CMD_GETSIL");
		break;
	default:
		dev_err(cs->dev, "unknown command %d from LL\n",
			cntrl->command);
		return -EINVAL;
	}

	return retval;
}

void gigaset_i4l_cmd(struct cardstate *cs, int cmd)
{
	isdn_ctrl command;

	command.driver = cs->myid;
	command.command = cmd;
	command.arg = 0;
	cs->iif.statcallb(&command);
}

void gigaset_i4l_channel_cmd(struct bc_state *bcs, int cmd)
{
	isdn_ctrl command;

	command.driver = bcs->cs->myid;
	command.command = cmd;
	command.arg = bcs->channel;
	bcs->cs->iif.statcallb(&command);
}

int gigaset_isdn_setup_dial(struct at_state_t *at_state, void *data)
{
	struct bc_state *bcs = at_state->bcs;
	unsigned proto;
	const char *bc;
	size_t length[AT_NUM];
	size_t l;
	int i;
	struct setup_parm *sp = data;

	switch (bcs->proto2) {
	case ISDN_PROTO_L2_HDLC:
		proto = 1; 
		break;
	case ISDN_PROTO_L2_TRANS:
		proto = 2; 
		break;
	default:
		dev_err(bcs->cs->dev, "%s: invalid L2 protocol: %u\n",
			__func__, bcs->proto2);
		return -EINVAL;
	}

	switch (sp->si1) {
	case 1:		
		bc = "9090A3";	
		break;
	case 7:		
	default:	
		bc = "8890";	
	}
	

	length[AT_DIAL ] = 1 + strlen(sp->phone) + 1 + 1;
	l = strlen(sp->eazmsn);
	length[AT_MSN  ] = l ? 6 + l + 1 + 1 : 0;
	length[AT_BC   ] = 5 + strlen(bc) + 1 + 1;
	length[AT_PROTO] = 6 + 1 + 1 + 1; 
	length[AT_ISO  ] = 6 + 1 + 1 + 1; 
	length[AT_TYPE ] = 6 + 1 + 1 + 1; 
	length[AT_HLC  ] = 0;

	for (i = 0; i < AT_NUM; ++i) {
		kfree(bcs->commands[i]);
		bcs->commands[i] = NULL;
		if (length[i] &&
		    !(bcs->commands[i] = kmalloc(length[i], GFP_ATOMIC))) {
			dev_err(bcs->cs->dev, "out of memory\n");
			return -ENOMEM;
		}
	}

	
	if (sp->phone[0] == '*' && sp->phone[1] == '*') {
		
		snprintf(bcs->commands[AT_DIAL], length[AT_DIAL],
			 "D%s\r", sp->phone+2);
		strncpy(bcs->commands[AT_TYPE], "^SCTP=0\r", length[AT_TYPE]);
	} else {
		snprintf(bcs->commands[AT_DIAL], length[AT_DIAL],
			 "D%s\r", sp->phone);
		strncpy(bcs->commands[AT_TYPE], "^SCTP=1\r", length[AT_TYPE]);
	}

	if (bcs->commands[AT_MSN])
		snprintf(bcs->commands[AT_MSN], length[AT_MSN],
			 "^SMSN=%s\r", sp->eazmsn);
	snprintf(bcs->commands[AT_BC   ], length[AT_BC   ],
		 "^SBC=%s\r", bc);
	snprintf(bcs->commands[AT_PROTO], length[AT_PROTO],
		 "^SBPR=%u\r", proto);
	snprintf(bcs->commands[AT_ISO  ], length[AT_ISO  ],
		 "^SISO=%u\r", (unsigned)bcs->channel + 1);

	return 0;
}

int gigaset_isdn_setup_accept(struct at_state_t *at_state)
{
	unsigned proto;
	size_t length[AT_NUM];
	int i;
	struct bc_state *bcs = at_state->bcs;

	switch (bcs->proto2) {
	case ISDN_PROTO_L2_HDLC:
		proto = 1; 
		break;
	case ISDN_PROTO_L2_TRANS:
		proto = 2; 
		break;
	default:
		dev_err(at_state->cs->dev, "%s: invalid protocol: %u\n",
			__func__, bcs->proto2);
		return -EINVAL;
	}

	length[AT_DIAL ] = 0;
	length[AT_MSN  ] = 0;
	length[AT_BC   ] = 0;
	length[AT_PROTO] = 6 + 1 + 1 + 1; 
	length[AT_ISO  ] = 6 + 1 + 1 + 1; 
	length[AT_TYPE ] = 0;
	length[AT_HLC  ] = 0;

	for (i = 0; i < AT_NUM; ++i) {
		kfree(bcs->commands[i]);
		bcs->commands[i] = NULL;
		if (length[i] &&
		    !(bcs->commands[i] = kmalloc(length[i], GFP_ATOMIC))) {
			dev_err(at_state->cs->dev, "out of memory\n");
			return -ENOMEM;
		}
	}

	snprintf(bcs->commands[AT_PROTO], length[AT_PROTO],
		 "^SBPR=%u\r", proto);
	snprintf(bcs->commands[AT_ISO  ], length[AT_ISO  ],
		 "^SISO=%u\r", (unsigned) bcs->channel + 1);

	return 0;
}


int gigaset_isdn_icall(struct at_state_t *at_state)
{
	struct cardstate *cs = at_state->cs;
	struct bc_state *bcs = at_state->bcs;
	isdn_ctrl response;
	int retval;

	
	response.parm.setup.si1 = 0;	
	response.parm.setup.si2 = 0;
	response.parm.setup.screen = 0;	
	response.parm.setup.plan = 0;
	if (!at_state->str_var[STR_ZBC]) {
		
		response.parm.setup.si1 = 1;
	} else if (!strcmp(at_state->str_var[STR_ZBC], "8890")) {
		
		response.parm.setup.si1 = 7;
	} else if (!strcmp(at_state->str_var[STR_ZBC], "8090A3")) {
		
		response.parm.setup.si1 = 1;
	} else if (!strcmp(at_state->str_var[STR_ZBC], "9090A3")) {
		
		response.parm.setup.si1 = 1;
		response.parm.setup.si2 = 2;
	} else {
		dev_warn(cs->dev, "RING ignored - unsupported BC %s\n",
		     at_state->str_var[STR_ZBC]);
		return ICALL_IGNORE;
	}
	if (at_state->str_var[STR_NMBR]) {
		strncpy(response.parm.setup.phone, at_state->str_var[STR_NMBR],
			sizeof response.parm.setup.phone - 1);
		response.parm.setup.phone[sizeof response.parm.setup.phone - 1] = 0;
	} else
		response.parm.setup.phone[0] = 0;
	if (at_state->str_var[STR_ZCPN]) {
		strncpy(response.parm.setup.eazmsn, at_state->str_var[STR_ZCPN],
			sizeof response.parm.setup.eazmsn - 1);
		response.parm.setup.eazmsn[sizeof response.parm.setup.eazmsn - 1] = 0;
	} else
		response.parm.setup.eazmsn[0] = 0;

	if (!bcs) {
		dev_notice(cs->dev, "no channel for incoming call\n");
		response.command = ISDN_STAT_ICALLW;
		response.arg = 0; 
	} else {
		gig_dbg(DEBUG_CMD, "Sending ICALL");
		response.command = ISDN_STAT_ICALL;
		response.arg = bcs->channel; 
	}
	response.driver = cs->myid;
	retval = cs->iif.statcallb(&response);
	gig_dbg(DEBUG_CMD, "Response: %d", retval);
	switch (retval) {
	case 0:	
		return ICALL_IGNORE;
	case 1:	
		bcs->chstate |= CHS_NOTIFY_LL;
		return ICALL_ACCEPT;
	case 2:	
		return ICALL_REJECT;
	case 3:	
		dev_warn(cs->dev,
		       "LL requested unsupported feature: Incomplete Number\n");
		return ICALL_IGNORE;
	case 4:	
		
		return ICALL_ACCEPT;
	case 5:	
		dev_warn(cs->dev,
			 "LL requested unsupported feature: Call Deflection\n");
		return ICALL_IGNORE;
	default:
		dev_err(cs->dev, "LL error %d on ICALL\n", retval);
		return ICALL_IGNORE;
	}
}


int gigaset_register_to_LL(struct cardstate *cs, const char *isdnid)
{
	isdn_if *iif = &cs->iif;

	gig_dbg(DEBUG_ANY, "Register driver capabilities to LL");

	if (snprintf(iif->id, sizeof iif->id, "%s_%u", isdnid, cs->minor_index)
	    >= sizeof iif->id) {
		pr_err("ID too long: %s\n", isdnid);
		return 0;
	}

	iif->owner = THIS_MODULE;
	iif->channels = cs->channels;
	iif->maxbufsize = MAX_BUF_SIZE;
	iif->features = ISDN_FEATURE_L2_TRANS |
		ISDN_FEATURE_L2_HDLC |
#ifdef GIG_X75
		ISDN_FEATURE_L2_X75I |
#endif
		ISDN_FEATURE_L3_TRANS |
		ISDN_FEATURE_P_EURO;
	iif->hl_hdrlen = HW_HDR_LEN;		
	iif->command = command_from_LL;
	iif->writebuf_skb = writebuf_from_LL;
	iif->writecmd = NULL;			
	iif->readstat = NULL;			
	iif->rcvcallb_skb = NULL;		
	iif->statcallb = NULL;			

	if (!register_isdn(iif)) {
		pr_err("register_isdn failed\n");
		return 0;
	}

	cs->myid = iif->channels;		
	return 1;
}
