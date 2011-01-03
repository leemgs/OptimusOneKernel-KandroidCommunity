
struct RAR_address_struct {
        u32 low;
        u32 high;
};


int get_rar_address(int rar_index,struct RAR_address_struct *addresses);



int lock_rar(int rar_index);



#define RAR_DEBUG_LEVEL_BASIC       0x1

#define RAR_DEBUG_LEVEL_REGISTERS   0x2

#define RAR_DEBUG_LEVEL_EXTENDED    0x4

#define DEBUG_LEVEL	0x7




#define DEBUG_PRINT_0(DEBUG_LEVEL , info) \
do \
{ \
  if(DEBUG_LEVEL) \
  { \
    printk(KERN_WARNING info); \
  } \
}while(0)


#define DEBUG_PRINT_1(DEBUG_LEVEL , info , param1) \
do \
{ \
  if(DEBUG_LEVEL) \
  { \
    printk(KERN_WARNING info , param1); \
  } \
}while(0)


#define DEBUG_PRINT_2(DEBUG_LEVEL , info , param1, param2) \
do \
{ \
  if(DEBUG_LEVEL) \
  { \
    printk(KERN_WARNING info , param1, param2); \
  } \
}while(0)


#define DEBUG_PRINT_3(DEBUG_LEVEL , info , param1, param2 , param3) \
do \
{ \
  if(DEBUG_LEVEL) \
  { \
    printk(KERN_WARNING info , param1, param2 , param3); \
  } \
}while(0)


#define DEBUG_PRINT_4(DEBUG_LEVEL , info , param1, param2 , param3 , param4) \
do \
{ \
  if(DEBUG_LEVEL) \
  { \
    printk(KERN_WARNING info , param1, param2 , param3 , param4); \
  } \
}while(0)

