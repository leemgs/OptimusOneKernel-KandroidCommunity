






#include <mach/csp/chipcHw_inline.h>



static const char *gMuxStr[] = {
	"GPIO",			
	"KeyPad",		
	"I2C-Host",		
	"SPI",			
	"Uart",			
	"LED-Mtx-P",		
	"LED-Mtx-S",		
	"SDIO-0",		
	"SDIO-1",		
	"PCM",			
	"I2S",			
	"ETM",			
	"Debug",		
	"Misc",			
	"0xE",			
	"0xF",			
};





const char *chipcHw_getGpioPinFunctionStr(int pin)
{
	if ((pin < 0) || (pin >= chipcHw_GPIO_COUNT)) {
		return "";
	}

	return gMuxStr[chipcHw_getGpioPinFunction(pin)];
}
