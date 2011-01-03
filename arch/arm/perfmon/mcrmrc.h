



#ifndef __mrcmcr__h_
#define __mrcmcr__h_




#ifdef __ASSEMBLY__


#define MRC(reg, processor, op1, crn, crm, op2) \
(mrc      processor , op1 , reg,  crn , crm , op2)

#define MCR(reg, processor, op1, crn, crm, op2) \
(mcr      processor , op1 , reg,  crn , crm , op2)


#else

#define MRC(reg, processor, op1, crn, crm, op2) \
__asm__ __volatile__ ( \
"   mrc   "   #processor "," #op1 ", %0,"  #crn "," #crm "," #op2 " \n" \
: "=r" (reg))

#define MCR(reg, processor, op1, crn, crm, op2) \
__asm__ __volatile__ ( \
"   mcr   "   #processor "," #op1 ", %0,"  #crn "," #crm "," #op2 " \n" \
: : "r" (reg))
#endif



#define MRC15(reg, op1, crn, crm, op2) MRC(reg, p15, op1, crn, crm, op2)
#define MCR15(reg, op1, crn, crm, op2) MCR(reg, p15, op1, crn, crm, op2)

#endif
