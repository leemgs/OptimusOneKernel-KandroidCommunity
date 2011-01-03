

#include "bfad_drv.h"
#include "bfad_trcmod.h"

BFA_TRC_FILE(LDRV, INTR);


irqreturn_t bfad_intx(int irq, void *dev_id);
static int msix_disable;
module_param(msix_disable, int, S_IRUGO | S_IWUSR);

irqreturn_t
bfad_intx(int irq, void *dev_id)
{
	struct bfad_s         *bfad = dev_id;
	struct list_head         doneq;
	unsigned long   flags;
	bfa_boolean_t rc;

	spin_lock_irqsave(&bfad->bfad_lock, flags);
	rc = bfa_intx(&bfad->bfa);
	if (!rc) {
		spin_unlock_irqrestore(&bfad->bfad_lock, flags);
		return IRQ_NONE;
	}

	bfa_comp_deq(&bfad->bfa, &doneq);
	spin_unlock_irqrestore(&bfad->bfad_lock, flags);

	if (!list_empty(&doneq)) {
		bfa_comp_process(&bfad->bfa, &doneq);

		spin_lock_irqsave(&bfad->bfad_lock, flags);
		bfa_comp_free(&bfad->bfa, &doneq);
		spin_unlock_irqrestore(&bfad->bfad_lock, flags);
		bfa_trc_fp(bfad, irq);
	}

	return IRQ_HANDLED;

}

static irqreturn_t
bfad_msix(int irq, void *dev_id)
{
	struct bfad_msix_s *vec = dev_id;
	struct bfad_s *bfad = vec->bfad;
	struct list_head doneq;
	unsigned long   flags;

	spin_lock_irqsave(&bfad->bfad_lock, flags);

	bfa_msix(&bfad->bfa, vec->msix.entry);
	bfa_comp_deq(&bfad->bfa, &doneq);
	spin_unlock_irqrestore(&bfad->bfad_lock, flags);

	if (!list_empty(&doneq)) {
		bfa_comp_process(&bfad->bfa, &doneq);

		spin_lock_irqsave(&bfad->bfad_lock, flags);
		bfa_comp_free(&bfad->bfa, &doneq);
		spin_unlock_irqrestore(&bfad->bfad_lock, flags);
	}

	return IRQ_HANDLED;
}


static void
bfad_init_msix_entry(struct bfad_s *bfad, struct msix_entry *msix_entries,
			 int mask, int max_bit)
{
	int             i;
	int             match = 0x00000001;

	for (i = 0, bfad->nvec = 0; i < MAX_MSIX_ENTRY; i++) {
		if (mask & match) {
			bfad->msix_tab[bfad->nvec].msix.entry = i;
			bfad->msix_tab[bfad->nvec].bfad = bfad;
			msix_entries[bfad->nvec].entry = i;
			bfad->nvec++;
		}

		match <<= 1;
	}

}

int
bfad_install_msix_handler(struct bfad_s *bfad)
{
	int             i, error = 0;

	for (i = 0; i < bfad->nvec; i++) {
		error = request_irq(bfad->msix_tab[i].msix.vector,
				    (irq_handler_t) bfad_msix, 0,
				    BFAD_DRIVER_NAME, &bfad->msix_tab[i]);
		bfa_trc(bfad, i);
		bfa_trc(bfad, bfad->msix_tab[i].msix.vector);
		if (error) {
			int             j;

			for (j = 0; j < i; j++)
				free_irq(bfad->msix_tab[j].msix.vector,
						&bfad->msix_tab[j]);

			return 1;
		}
	}

	return 0;
}


int
bfad_setup_intr(struct bfad_s *bfad)
{
	int error = 0;
	u32 mask = 0, i, num_bit = 0, max_bit = 0;
	struct msix_entry msix_entries[MAX_MSIX_ENTRY];

	
	bfa_msix_getvecs(&bfad->bfa, &mask, &num_bit, &max_bit);

	
	bfad_init_msix_entry(bfad, msix_entries, mask, max_bit);

	if (!msix_disable) {
		error = pci_enable_msix(bfad->pcidev, msix_entries, bfad->nvec);
		if (error) {
			

			printk(KERN_WARNING "bfad%d: "
				"pci_enable_msix failed (%d),"
				" use line based.\n", bfad->inst_no, error);

			goto line_based;
		}

		
		for (i = 0; i < bfad->nvec; i++) {
			bfa_trc(bfad, msix_entries[i].vector);
			bfad->msix_tab[i].msix.vector = msix_entries[i].vector;
		}

		bfa_msix_init(&bfad->bfa, bfad->nvec);

		bfad->bfad_flags |= BFAD_MSIX_ON;

		return error;
	}

line_based:
	error = 0;
	if (request_irq
	    (bfad->pcidev->irq, (irq_handler_t) bfad_intx, BFAD_IRQ_FLAGS,
	     BFAD_DRIVER_NAME, bfad) != 0) {
		
		return 1;
	}

	return error;
}

void
bfad_remove_intr(struct bfad_s *bfad)
{
	int             i;

	if (bfad->bfad_flags & BFAD_MSIX_ON) {
		for (i = 0; i < bfad->nvec; i++)
			free_irq(bfad->msix_tab[i].msix.vector,
					&bfad->msix_tab[i]);

		pci_disable_msix(bfad->pcidev);
		bfad->bfad_flags &= ~BFAD_MSIX_ON;
	} else {
		free_irq(bfad->pcidev->irq, bfad);
	}
}


