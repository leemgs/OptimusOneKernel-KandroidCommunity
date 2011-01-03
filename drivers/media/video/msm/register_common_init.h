#ifndef	REGISTER_COMMON_INIT_H
#define	REGISTER_COMMON_INIT_H




#undef word
typedef uint16_t word;
#undef byte
typedef uint8_t byte;
#undef uint32
typedef uint32_t uint32;

typedef enum {
  COMMON_REG_REG,        
  COMMON_REG_REG_VAR4,   
  COMMON_REG_MEM,        
  COMMON_REG_MEM_VAR4,   

  COMMON_REG_MAX
} common_reg_enum_type;

typedef union{
  uint32 val32;
  word val16;
  byte val8[4];
} common_reg_data_type;

typedef struct
{
  byte addr;
  byte val;
} common_reg_reg_type;

typedef struct
{
  byte addr;
  common_reg_data_type vals;
  byte len;
} common_reg_reg_var4_type;

typedef struct
{
  word addr;
  word val;
} common_reg_mem_type;

typedef struct
{
  word addr;
  common_reg_data_type vals;
  byte len;
} common_reg_mem_var4_type;

typedef union
{
  common_reg_reg_type reg;
  common_reg_reg_var4_type reg_var4;
  common_reg_mem_type mem;
  common_reg_mem_var4_type mem_var4;
} common_reg_type;

typedef struct
{
  int num_regs;
  common_reg_type list_regs[1]; 
} common_reg_list_type;

extern void common_register_init(common_reg_enum_type , common_reg_list_type** );
#endif 