

#ifndef _MARIMBA_TSADC_H_
#define _MARIMBA_TSADC_H_

struct marimba_tsadc_client;

#define	TSSC_SUSPEND_LEVEL  1
#define	TSADC_SUSPEND_LEVEL 2

int marimba_tsadc_start(struct marimba_tsadc_client *client);

struct marimba_tsadc_client *
marimba_tsadc_register(struct platform_device *pdev, unsigned int is_ts);

void marimba_tsadc_unregister(struct marimba_tsadc_client *client);

#endif 
