


#define OFFSET_TCR1		0x00	
#define OFFSET_TCR2		0x04	
#define OFFSET_TCLKDIV		0x08	
#define OFFSET_TFSDIV		0x0C	
#define OFFSET_TX		0x10	
#define OFFSET_RX		0x18	
#define OFFSET_RCR1		0x20	
#define OFFSET_RCR2		0x24	
#define OFFSET_RCLKDIV		0x28	
#define OFFSET_RFSDIV		0x2c	
#define OFFSET_STAT		0x30	

#define SPORT_GET_TCR1(sport)		bfin_read16(((sport)->port.membase + OFFSET_TCR1))
#define SPORT_GET_TCR2(sport)		bfin_read16(((sport)->port.membase + OFFSET_TCR2))
#define SPORT_GET_TCLKDIV(sport)	bfin_read16(((sport)->port.membase + OFFSET_TCLKDIV))
#define SPORT_GET_TFSDIV(sport)		bfin_read16(((sport)->port.membase + OFFSET_TFSDIV))
#define SPORT_GET_TX(sport)		bfin_read16(((sport)->port.membase + OFFSET_TX))
#define SPORT_GET_RX(sport)		bfin_read16(((sport)->port.membase + OFFSET_RX))
#define SPORT_GET_RX32(sport)		bfin_read32(((sport)->port.membase + OFFSET_RX))
#define SPORT_GET_RCR1(sport)		bfin_read16(((sport)->port.membase + OFFSET_RCR1))
#define SPORT_GET_RCR2(sport)		bfin_read16(((sport)->port.membase + OFFSET_RCR2))
#define SPORT_GET_RCLKDIV(sport)	bfin_read16(((sport)->port.membase + OFFSET_RCLKDIV))
#define SPORT_GET_RFSDIV(sport)		bfin_read16(((sport)->port.membase + OFFSET_RFSDIV))
#define SPORT_GET_STAT(sport)		bfin_read16(((sport)->port.membase + OFFSET_STAT))

#define SPORT_PUT_TCR1(sport, v)	bfin_write16(((sport)->port.membase + OFFSET_TCR1), v)
#define SPORT_PUT_TCR2(sport, v)	bfin_write16(((sport)->port.membase + OFFSET_TCR2), v)
#define SPORT_PUT_TCLKDIV(sport, v)	bfin_write16(((sport)->port.membase + OFFSET_TCLKDIV), v)
#define SPORT_PUT_TFSDIV(sport, v)	bfin_write16(((sport)->port.membase + OFFSET_TFSDIV), v)
#define SPORT_PUT_TX(sport, v)		bfin_write16(((sport)->port.membase + OFFSET_TX), v)
#define SPORT_PUT_RX(sport, v)		bfin_write16(((sport)->port.membase + OFFSET_RX), v)
#define SPORT_PUT_RCR1(sport, v)	bfin_write16(((sport)->port.membase + OFFSET_RCR1), v)
#define SPORT_PUT_RCR2(sport, v)	bfin_write16(((sport)->port.membase + OFFSET_RCR2), v)
#define SPORT_PUT_RCLKDIV(sport, v)	bfin_write16(((sport)->port.membase + OFFSET_RCLKDIV), v)
#define SPORT_PUT_RFSDIV(sport, v)	bfin_write16(((sport)->port.membase + OFFSET_RFSDIV), v)
#define SPORT_PUT_STAT(sport, v)	bfin_write16(((sport)->port.membase + OFFSET_STAT), v)
