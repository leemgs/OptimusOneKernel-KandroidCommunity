

#ifndef __ARCH_ARM_MACH_MSM_GPIO_HW_8XXX_H
#define __ARCH_ARM_MACH_MSM_GPIO_HW_8XXX_H


#define GPIO_OUT_0         GPIO1_REG(0x00)  
#define GPIO_OUT_1         GPIO2_REG(0x00)  
#define GPIO_OUT_2         GPIO1_REG(0x04)  
#define GPIO_OUT_3         GPIO1_REG(0x08)  
#define GPIO_OUT_4         GPIO1_REG(0x0C)  
#define GPIO_OUT_5         GPIO1_REG(0x10)  
#define GPIO_OUT_6         GPIO1_REG(0x14)  
#define GPIO_OUT_7         GPIO1_REG(0x18)  


#define GPIO_OE_0          GPIO1_REG(0x20)
#define GPIO_OE_1          GPIO2_REG(0x08)
#define GPIO_OE_2          GPIO1_REG(0x24)
#define GPIO_OE_3          GPIO1_REG(0x28)
#define GPIO_OE_4          GPIO1_REG(0x2C)
#define GPIO_OE_5          GPIO1_REG(0x30)
#define GPIO_OE_6          GPIO1_REG(0x34)
#define GPIO_OE_7          GPIO1_REG(0x38)


#define GPIO_IN_0          GPIO1_REG(0x50)
#define GPIO_IN_1          GPIO2_REG(0x20)
#define GPIO_IN_2          GPIO1_REG(0x54)
#define GPIO_IN_3          GPIO1_REG(0x58)
#define GPIO_IN_4          GPIO1_REG(0x5C)
#define GPIO_IN_5          GPIO1_REG(0x60)
#define GPIO_IN_6          GPIO1_REG(0x64)
#define GPIO_IN_7          GPIO1_REG(0x68)


#define GPIO_INT_EDGE_0    GPIO1_REG(0x70)
#define GPIO_INT_EDGE_1    GPIO2_REG(0x50)
#define GPIO_INT_EDGE_2    GPIO1_REG(0x74)
#define GPIO_INT_EDGE_3    GPIO1_REG(0x78)
#define GPIO_INT_EDGE_4    GPIO1_REG(0x7C)
#define GPIO_INT_EDGE_5    GPIO1_REG(0x80)
#define GPIO_INT_EDGE_6    GPIO1_REG(0x84)
#define GPIO_INT_EDGE_7    GPIO1_REG(0x88)


#define GPIO_INT_POS_0     GPIO1_REG(0x90)
#define GPIO_INT_POS_1     GPIO2_REG(0x58)
#define GPIO_INT_POS_2     GPIO1_REG(0x94)
#define GPIO_INT_POS_3     GPIO1_REG(0x98)
#define GPIO_INT_POS_4     GPIO1_REG(0x9C)
#define GPIO_INT_POS_5     GPIO1_REG(0xA0)
#define GPIO_INT_POS_6     GPIO1_REG(0xA4)
#define GPIO_INT_POS_7     GPIO1_REG(0xA8)


#define GPIO_INT_EN_0      GPIO1_REG(0xB0)
#define GPIO_INT_EN_1      GPIO2_REG(0x60)
#define GPIO_INT_EN_2      GPIO1_REG(0xB4)
#define GPIO_INT_EN_3      GPIO1_REG(0xB8)
#define GPIO_INT_EN_4      GPIO1_REG(0xBC)
#define GPIO_INT_EN_5      GPIO1_REG(0xC0)
#define GPIO_INT_EN_6      GPIO1_REG(0xC4)
#define GPIO_INT_EN_7      GPIO1_REG(0xC8)


#define GPIO_INT_CLEAR_0   GPIO1_REG(0xD0)
#define GPIO_INT_CLEAR_1   GPIO2_REG(0x68)
#define GPIO_INT_CLEAR_2   GPIO1_REG(0xD4)
#define GPIO_INT_CLEAR_3   GPIO1_REG(0xD8)
#define GPIO_INT_CLEAR_4   GPIO1_REG(0xDC)
#define GPIO_INT_CLEAR_5   GPIO1_REG(0xE0)
#define GPIO_INT_CLEAR_6   GPIO1_REG(0xE4)
#define GPIO_INT_CLEAR_7   GPIO1_REG(0xE8)


#define GPIO_INT_STATUS_0  GPIO1_REG(0xF0)
#define GPIO_INT_STATUS_1  GPIO2_REG(0x70)
#define GPIO_INT_STATUS_2  GPIO1_REG(0xF4)
#define GPIO_INT_STATUS_3  GPIO1_REG(0xF8)
#define GPIO_INT_STATUS_4  GPIO1_REG(0xFC)
#define GPIO_INT_STATUS_5  GPIO1_REG(0x100)
#define GPIO_INT_STATUS_6  GPIO1_REG(0x104)
#define GPIO_INT_STATUS_7  GPIO1_REG(0x108)

#endif
