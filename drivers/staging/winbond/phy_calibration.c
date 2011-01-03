


#include "sysdef.h"
#include "phy_calibration.h"
#include "wbhal_f.h"





#define LOOP_TIMES      20
#define US              1000

#define AG_CONST        0.6072529350
#define FIXED(X)        ((s32)((X) * 32768.0))
#define DEG2RAD(X)      0.017453 * (X)


typedef s32         fixed; 

static const fixed Angles[]=
{
    FIXED(DEG2RAD(45.0)),    FIXED(DEG2RAD(26.565)),  FIXED(DEG2RAD(14.0362)),
    FIXED(DEG2RAD(7.12502)), FIXED(DEG2RAD(3.57633)), FIXED(DEG2RAD(1.78991)),
    FIXED(DEG2RAD(0.895174)),FIXED(DEG2RAD(0.447614)),FIXED(DEG2RAD(0.223811)),
    FIXED(DEG2RAD(0.111906)),FIXED(DEG2RAD(0.055953)),FIXED(DEG2RAD(0.027977))
};







s32 _s13_to_s32(u32 data)
{
    u32     val;

    val = (data & 0x0FFF);

    if ((data & BIT(12)) != 0)
    {
        val |= 0xFFFFF000;
    }

    return ((s32) val);
}

u32 _s32_to_s13(s32 data)
{
    u32     val;

    if (data > 4095)
    {
        data = 4095;
    }
    else if (data < -4096)
    {
        data = -4096;
    }

    val = data & 0x1FFF;

    return val;
}


s32 _s4_to_s32(u32 data)
{
    s32     val;

    val = (data & 0x0007);

    if ((data & BIT(3)) != 0)
    {
        val |= 0xFFFFFFF8;
    }

    return val;
}

u32 _s32_to_s4(s32 data)
{
    u32     val;

    if (data > 7)
    {
        data = 7;
    }
    else if (data < -8)
    {
        data = -8;
    }

    val = data & 0x000F;

    return val;
}


s32 _s5_to_s32(u32 data)
{
    s32     val;

    val = (data & 0x000F);

    if ((data & BIT(4)) != 0)
    {
        val |= 0xFFFFFFF0;
    }

    return val;
}

u32 _s32_to_s5(s32 data)
{
    u32     val;

    if (data > 15)
    {
        data = 15;
    }
    else if (data < -16)
    {
        data = -16;
    }

    val = data & 0x001F;

    return val;
}


s32 _s6_to_s32(u32 data)
{
    s32     val;

    val = (data & 0x001F);

    if ((data & BIT(5)) != 0)
    {
        val |= 0xFFFFFFE0;
    }

    return val;
}

u32 _s32_to_s6(s32 data)
{
    u32     val;

    if (data > 31)
    {
        data = 31;
    }
    else if (data < -32)
    {
        data = -32;
    }

    val = data & 0x003F;

    return val;
}


s32 _s9_to_s32(u32 data)
{
    s32     val;

    val = data & 0x00FF;

    if ((data & BIT(8)) != 0)
    {
        val |= 0xFFFFFF00;
    }

    return val;
}

u32 _s32_to_s9(s32 data)
{
    u32     val;

    if (data > 255)
    {
        data = 255;
    }
    else if (data < -256)
    {
        data = -256;
    }

    val = data & 0x01FF;

    return val;
}


s32 _floor(s32 n)
{
    if (n > 0)
    {
        n += 5;
    }
    else
    {
        n -= 5;
    }

    return (n/10);
}





u32 _sqrt(u32 sqsum)
{
    u32     sq_rt;

    int     g0, g1, g2, g3, g4;
    int     seed;
    int     next;
    int     step;

    g4 =  sqsum / 100000000;
    g3 = (sqsum - g4*100000000) /1000000;
    g2 = (sqsum - g4*100000000 - g3*1000000) /10000;
    g1 = (sqsum - g4*100000000 - g3*1000000 - g2*10000) /100;
    g0 = (sqsum - g4*100000000 - g3*1000000 - g2*10000 - g1*100);

    next = g4;
    step = 0;
    seed = 0;
    while (((seed+1)*(step+1)) <= next)
    {
    	step++;
    	seed++;
    }

    sq_rt = seed * 10000;
    next = (next-(seed*step))*100 + g3;

    step = 0;
    seed = 2 * seed * 10;
    while (((seed+1)*(step+1)) <= next)
    {
        step++;
    	seed++;
    }

    sq_rt = sq_rt + step * 1000;
    next = (next - seed * step) * 100 + g2;
    seed = (seed + step) * 10;
    step = 0;
    while (((seed+1)*(step+1)) <= next)
    {
        step++;
    	seed++;
    }

    sq_rt = sq_rt + step * 100;
    next = (next - seed * step) * 100 + g1;
    seed = (seed + step) * 10;
    step = 0;

    while (((seed+1)*(step+1)) <= next)
    {
        step++;
    	seed++;
    }

    sq_rt = sq_rt + step * 10;
    next = (next - seed* step) * 100 + g0;
    seed = (seed + step) * 10;
    step = 0;

    while (((seed+1)*(step+1)) <= next)
    {
        step++;
    	seed++;
    }

    sq_rt = sq_rt + step;

    return sq_rt;
}


void _sin_cos(s32 angle, s32 *sin, s32 *cos)
{
    fixed       X, Y, TargetAngle, CurrAngle;
    unsigned    Step;

    X=FIXED(AG_CONST);      
    Y=0;                    
    TargetAngle=abs(angle);
    CurrAngle=0;

    for (Step=0; Step < 12; Step++)
    {
        fixed NewX;

        if(TargetAngle > CurrAngle)
        {
            NewX=X - (Y >> Step);
            Y=(X >> Step) + Y;
            X=NewX;
            CurrAngle += Angles[Step];
        }
        else
        {
            NewX=X + (Y >> Step);
            Y=-(X >> Step) + Y;
            X=NewX;
            CurrAngle -= Angles[Step];
        }
    }

    if (angle > 0)
    {
        *cos = X;
        *sin = Y;
    }
    else
    {
        *cos = X;
        *sin = -Y;
    }
}

static unsigned char hal_get_dxx_reg(struct hw_data *pHwData, u16 number, u32 * pValue)
{
	if (number < 0x1000)
		number += 0x1000;
	return Wb35Reg_ReadSync(pHwData, number, pValue);
}
#define hw_get_dxx_reg( _A, _B, _C ) hal_get_dxx_reg( _A, _B, (u32 *)_C )

static unsigned char hal_set_dxx_reg(struct hw_data *pHwData, u16 number, u32 value)
{
	unsigned char ret;

	if (number < 0x1000)
		number += 0x1000;
	ret = Wb35Reg_WriteSync(pHwData, number, value);
	return ret;
}
#define hw_set_dxx_reg( _A, _B, _C ) hal_set_dxx_reg( _A, _B, (u32)_C )


void _reset_rx_cal(struct hw_data *phw_data)
{
	u32     val;

	hw_get_dxx_reg(phw_data, 0x54, &val);

	if (phw_data->revision == 0x2002) 
	{
		val &= 0xFFFF0000;
	}
	else 
	{
		val &= 0x000003FF;
	}

	hw_set_dxx_reg(phw_data, 0x54, val);
}








void _rxadc_dc_offset_cancellation_winbond(struct hw_data *phw_data, u32 frequency)
{
    u32     reg_agc_ctrl3;
    u32     reg_a_acq_ctrl;
    u32     reg_b_acq_ctrl;
    u32     val;

    PHY_DEBUG(("[CAL] -> [1]_rxadc_dc_offset_cancellation()\n"));
    phy_init_rf(phw_data);

    
    if( (RF_WB_242 == phw_data->phy_type) ||
		(RF_WB_242_1 == phw_data->phy_type) ) 
    {
        if ((frequency >= 2412) && (frequency <= 2484))
        {
            
            PHY_DEBUG(("[CAL] W89RF242/11G/Channel=2390Mhz\n"));
			phy_set_rf_data(phw_data, 3, (3<<24)|0x025586);

        }
    }
    else
	{

	}

	
	hw_get_dxx_reg(phw_data, 0x5C, &val);
	val &= ~(0x03FF);
	hw_set_dxx_reg(phw_data, 0x5C, val);

	
	hw_set_dxx_reg(phw_data, 0x3C, 0);
	hw_set_dxx_reg(phw_data, 0x54, 0);

	hw_set_dxx_reg(phw_data, 0x58, 0x30303030); 

	
	hw_get_dxx_reg(phw_data, REG_AGC_CTRL3, &reg_agc_ctrl3);
	reg_agc_ctrl3 &= ~BIT(2);
	reg_agc_ctrl3 |= (MASK_LNA_FIX_GAIN|MASK_AGC_FIX);
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL3, reg_agc_ctrl3);

	hw_get_dxx_reg(phw_data, REG_AGC_CTRL5, &val);
	val |= MASK_AGC_FIX_GAIN;
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL5, val);

	
	hw_get_dxx_reg(phw_data, REG_A_ACQ_CTRL, &reg_a_acq_ctrl);
	reg_a_acq_ctrl |= MASK_AMER_OFF_REG;
	hw_set_dxx_reg(phw_data, REG_A_ACQ_CTRL, reg_a_acq_ctrl);

	hw_get_dxx_reg(phw_data, REG_B_ACQ_CTRL, &reg_b_acq_ctrl);
	reg_b_acq_ctrl |= MASK_BMER_OFF_REG;
	hw_set_dxx_reg(phw_data, REG_B_ACQ_CTRL, reg_b_acq_ctrl);

	
	
	
	hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &val);
	val &= ~MASK_ADC_DC_CAL_STR;
	hw_set_dxx_reg(phw_data, REG_MODE_CTRL, val);

	val |= MASK_ADC_DC_CAL_STR;
	hw_set_dxx_reg(phw_data, REG_MODE_CTRL, val);

	
#ifdef _DEBUG
	hw_get_dxx_reg(phw_data, REG_OFFSET_READ, &val);
	PHY_DEBUG(("[CAL]    REG_OFFSET_READ = 0x%08X\n", val));

	PHY_DEBUG(("[CAL]    ** adc_dc_cal_i = %d (0x%04X)\n",
			   _s9_to_s32(val&0x000001FF), val&0x000001FF));
	PHY_DEBUG(("[CAL]    ** adc_dc_cal_q = %d (0x%04X)\n",
			   _s9_to_s32((val&0x0003FE00)>>9), (val&0x0003FE00)>>9));
#endif

	hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &val);
	val &= ~MASK_ADC_DC_CAL_STR;
	hw_set_dxx_reg(phw_data, REG_MODE_CTRL, val);

	
	
	reg_a_acq_ctrl &= ~MASK_AMER_OFF_REG;
	hw_set_dxx_reg(phw_data, REG_A_ACQ_CTRL, reg_a_acq_ctrl);

	
	reg_b_acq_ctrl &= ~MASK_BMER_OFF_REG;
	hw_set_dxx_reg(phw_data, REG_B_ACQ_CTRL, reg_b_acq_ctrl);

	
	
	reg_agc_ctrl3 |= BIT(2);
	reg_agc_ctrl3 &= ~(MASK_LNA_FIX_GAIN|MASK_AGC_FIX);
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL3, reg_agc_ctrl3);
}


void _txidac_dc_offset_cancellation_winbond(struct hw_data *phw_data)
{
	u32     reg_agc_ctrl3;
	u32     reg_mode_ctrl;
	u32     reg_dc_cancel;
	s32     iqcal_image_i;
	s32     iqcal_image_q;
	u32     sqsum;
	s32     mag_0;
	s32     mag_1;
	s32     fix_cancel_dc_i = 0;
	u32     val;
	int     loop;

	PHY_DEBUG(("[CAL] -> [2]_txidac_dc_offset_cancellation()\n"));

	

	
	phy_set_rf_data(phw_data, 1, (1<<24)|0xEE3FC2);
	
	phy_set_rf_data(phw_data, 11, (11<<24)|0x1901D6);
	
	phy_set_rf_data(phw_data, 5, (5<<24)|0x24C48A);
    
	phy_set_rf_data(phw_data, 6, (6<<24)|0x06890C);
	
	phy_set_rf_data(phw_data, 0, (0<<24)|0xFDF1C0);

	hw_set_dxx_reg(phw_data, 0x58, 0x30303030); 

	
	hw_get_dxx_reg(phw_data, REG_AGC_CTRL3, &reg_agc_ctrl3);
	reg_agc_ctrl3 &= ~BIT(2);
	reg_agc_ctrl3 |= (MASK_LNA_FIX_GAIN|MASK_AGC_FIX);
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL3, reg_agc_ctrl3);

	hw_get_dxx_reg(phw_data, REG_AGC_CTRL5, &val);
	val |= MASK_AGC_FIX_GAIN;
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL5, val);

	
	hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &reg_mode_ctrl);

	PHY_DEBUG(("[CAL]    MODE_CTRL (read) = 0x%08X\n", reg_mode_ctrl));
	reg_mode_ctrl &= ~(MASK_IQCAL_TONE_SEL|MASK_IQCAL_MODE);

	
	

	
	

	
	reg_mode_ctrl |= (MASK_CALIB_START|2|(2<<2));
	hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
	PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

	hw_get_dxx_reg(phw_data, 0x5C, &reg_dc_cancel);
	PHY_DEBUG(("[CAL]    DC_CANCEL (read) = 0x%08X\n", reg_dc_cancel));

	for (loop = 0; loop < LOOP_TIMES; loop++)
	{
		PHY_DEBUG(("[CAL] [%d.] ==================================\n", loop));

		
		
		reg_dc_cancel &= ~(0x03FF);
		PHY_DEBUG(("[CAL]    DC_CANCEL (write) = 0x%08X\n", reg_dc_cancel));
		hw_set_dxx_reg(phw_data, 0x5C, reg_dc_cancel);

		hw_get_dxx_reg(phw_data, REG_CALIB_READ2, &val);
		PHY_DEBUG(("[CAL]    CALIB_READ2 = 0x%08X\n", val));

		iqcal_image_i = _s13_to_s32(val & 0x00001FFF);
		iqcal_image_q = _s13_to_s32((val & 0x03FFE000) >> 13);
		sqsum = iqcal_image_i*iqcal_image_i + iqcal_image_q*iqcal_image_q;
		mag_0 = (s32) _sqrt(sqsum);
		PHY_DEBUG(("[CAL]    mag_0=%d (iqcal_image_i=%d, iqcal_image_q=%d)\n",
				   mag_0, iqcal_image_i, iqcal_image_q));

		
		reg_dc_cancel |= (1 << CANCEL_DC_I_SHIFT);
		PHY_DEBUG(("[CAL]    DC_CANCEL (write) = 0x%08X\n", reg_dc_cancel));
		hw_set_dxx_reg(phw_data, 0x5C, reg_dc_cancel);

		hw_get_dxx_reg(phw_data, REG_CALIB_READ2, &val);
		PHY_DEBUG(("[CAL]    CALIB_READ2 = 0x%08X\n", val));

		iqcal_image_i = _s13_to_s32(val & 0x00001FFF);
		iqcal_image_q = _s13_to_s32((val & 0x03FFE000) >> 13);
		sqsum = iqcal_image_i*iqcal_image_i + iqcal_image_q*iqcal_image_q;
		mag_1 = (s32) _sqrt(sqsum);
		PHY_DEBUG(("[CAL]    mag_1=%d (iqcal_image_i=%d, iqcal_image_q=%d)\n",
				   mag_1, iqcal_image_i, iqcal_image_q));

		
		if (mag_0 != mag_1)
		{
			fix_cancel_dc_i = (mag_0*10000) / (mag_0*10000 - mag_1*10000);
		}
		else
		{
			if (mag_0 == mag_1)
			{
				PHY_DEBUG(("[CAL]   ***** mag_0 = mag_1 !!\n"));
			}

			fix_cancel_dc_i = 0;
		}

		PHY_DEBUG(("[CAL]    ** fix_cancel_dc_i = %d (0x%04X)\n",
				   fix_cancel_dc_i, _s32_to_s5(fix_cancel_dc_i)));

		if ((abs(mag_1-mag_0)*6) > mag_0)
		{
			break;
		}
	}

	if ( loop >= 19 )
	   fix_cancel_dc_i = 0;

	reg_dc_cancel &= ~(0x03FF);
	reg_dc_cancel |= (_s32_to_s5(fix_cancel_dc_i) << CANCEL_DC_I_SHIFT);
	hw_set_dxx_reg(phw_data, 0x5C, reg_dc_cancel);
	PHY_DEBUG(("[CAL]    DC_CANCEL (write) = 0x%08X\n", reg_dc_cancel));

	
	reg_mode_ctrl &= ~MASK_CALIB_START;
	hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
	PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));
}


void _txqdac_dc_offset_cacellation_winbond(struct hw_data *phw_data)
{
	u32     reg_agc_ctrl3;
	u32     reg_mode_ctrl;
	u32     reg_dc_cancel;
	s32     iqcal_image_i;
	s32     iqcal_image_q;
	u32     sqsum;
	s32     mag_0;
	s32     mag_1;
	s32     fix_cancel_dc_q = 0;
	u32     val;
	int     loop;

	PHY_DEBUG(("[CAL] -> [3]_txqdac_dc_offset_cacellation()\n"));
	
	phy_set_rf_data(phw_data, 1, (1<<24)|0xEE3FC2);
	
	phy_set_rf_data(phw_data, 11, (11<<24)|0x1901D6);
	
	phy_set_rf_data(phw_data, 5, (5<<24)|0x24C48A);
    
	phy_set_rf_data(phw_data, 6, (6<<24)|0x06890C);
	
	phy_set_rf_data(phw_data, 0, (0<<24)|0xFDF1C0);

	hw_set_dxx_reg(phw_data, 0x58, 0x30303030); 

	
	hw_get_dxx_reg(phw_data, REG_AGC_CTRL3, &reg_agc_ctrl3);
	reg_agc_ctrl3 &= ~BIT(2);
	reg_agc_ctrl3 |= (MASK_LNA_FIX_GAIN|MASK_AGC_FIX);
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL3, reg_agc_ctrl3);

	hw_get_dxx_reg(phw_data, REG_AGC_CTRL5, &val);
	val |= MASK_AGC_FIX_GAIN;
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL5, val);

	
	hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &reg_mode_ctrl);
	PHY_DEBUG(("[CAL]    MODE_CTRL (read) = 0x%08X\n", reg_mode_ctrl));

	
	reg_mode_ctrl &= ~(MASK_IQCAL_MODE);
	reg_mode_ctrl |= (MASK_CALIB_START|3);
	hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
	PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

	hw_get_dxx_reg(phw_data, 0x5C, &reg_dc_cancel);
	PHY_DEBUG(("[CAL]    DC_CANCEL (read) = 0x%08X\n", reg_dc_cancel));

	for (loop = 0; loop < LOOP_TIMES; loop++)
	{
		PHY_DEBUG(("[CAL] [%d.] ==================================\n", loop));

		
		
		reg_dc_cancel &= ~(0x001F);
		PHY_DEBUG(("[CAL]    DC_CANCEL (write) = 0x%08X\n", reg_dc_cancel));
		hw_set_dxx_reg(phw_data, 0x5C, reg_dc_cancel);

		hw_get_dxx_reg(phw_data, REG_CALIB_READ2, &val);
		PHY_DEBUG(("[CAL]    CALIB_READ2 = 0x%08X\n", val));

		iqcal_image_i = _s13_to_s32(val & 0x00001FFF);
		iqcal_image_q = _s13_to_s32((val & 0x03FFE000) >> 13);
		sqsum = iqcal_image_i*iqcal_image_i + iqcal_image_q*iqcal_image_q;
		mag_0 = _sqrt(sqsum);
		PHY_DEBUG(("[CAL]    mag_0=%d (iqcal_image_i=%d, iqcal_image_q=%d)\n",
				   mag_0, iqcal_image_i, iqcal_image_q));

		
		reg_dc_cancel |= (1 << CANCEL_DC_Q_SHIFT);
		PHY_DEBUG(("[CAL]    DC_CANCEL (write) = 0x%08X\n", reg_dc_cancel));
		hw_set_dxx_reg(phw_data, 0x5C, reg_dc_cancel);

		hw_get_dxx_reg(phw_data, REG_CALIB_READ2, &val);
		PHY_DEBUG(("[CAL]    CALIB_READ2 = 0x%08X\n", val));

		iqcal_image_i = _s13_to_s32(val & 0x00001FFF);
		iqcal_image_q = _s13_to_s32((val & 0x03FFE000) >> 13);
		sqsum = iqcal_image_i*iqcal_image_i + iqcal_image_q*iqcal_image_q;
		mag_1 = _sqrt(sqsum);
		PHY_DEBUG(("[CAL]    mag_1=%d (iqcal_image_i=%d, iqcal_image_q=%d)\n",
				   mag_1, iqcal_image_i, iqcal_image_q));

		
		if (mag_0 != mag_1)
		{
			fix_cancel_dc_q = (mag_0*10000) / (mag_0*10000 - mag_1*10000);
		}
		else
		{
			if (mag_0 == mag_1)
			{
				PHY_DEBUG(("[CAL]   ***** mag_0 = mag_1 !!\n"));
			}

			fix_cancel_dc_q = 0;
		}

		PHY_DEBUG(("[CAL]    ** fix_cancel_dc_q = %d (0x%04X)\n",
				   fix_cancel_dc_q, _s32_to_s5(fix_cancel_dc_q)));

		if ((abs(mag_1-mag_0)*6) > mag_0)
		{
			break;
		}
	}

	if ( loop >= 19 )
	   fix_cancel_dc_q = 0;

	reg_dc_cancel &= ~(0x001F);
	reg_dc_cancel |= (_s32_to_s5(fix_cancel_dc_q) << CANCEL_DC_Q_SHIFT);
	hw_set_dxx_reg(phw_data, 0x5C, reg_dc_cancel);
	PHY_DEBUG(("[CAL]    DC_CANCEL (write) = 0x%08X\n", reg_dc_cancel));


	
	reg_mode_ctrl &= ~MASK_CALIB_START;
	hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
	PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));
}


u8 _tx_iq_calibration_loop_winbond(struct hw_data *phw_data,
						   s32 a_2_threshold,
						   s32 b_2_threshold)
{
	u32     reg_mode_ctrl;
	s32     iq_mag_0_tx;
	s32     iqcal_tone_i0;
	s32     iqcal_tone_q0;
	s32     iqcal_tone_i;
	s32     iqcal_tone_q;
	u32     sqsum;
	s32     rot_i_b;
	s32     rot_q_b;
	s32     tx_cal_flt_b[4];
	s32     tx_cal[4];
	s32     tx_cal_reg[4];
	s32     a_2, b_2;
	s32     sin_b, sin_2b;
	s32     cos_b, cos_2b;
	s32     divisor;
	s32     temp1, temp2;
	u32     val;
	u16     loop;
	s32     iqcal_tone_i_avg,iqcal_tone_q_avg;
	u8      verify_count;
	int capture_time;

	PHY_DEBUG(("[CAL] -> _tx_iq_calibration_loop()\n"));
	PHY_DEBUG(("[CAL]    ** a_2_threshold = %d\n", a_2_threshold));
	PHY_DEBUG(("[CAL]    ** b_2_threshold = %d\n", b_2_threshold));

	verify_count = 0;

	hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &reg_mode_ctrl);
	PHY_DEBUG(("[CAL]    MODE_CTRL (read) = 0x%08X\n", reg_mode_ctrl));

	loop = LOOP_TIMES;

	while (loop > 0)
	{
		PHY_DEBUG(("[CAL] [%d.] <_tx_iq_calibration_loop>\n", (LOOP_TIMES-loop+1)));

		iqcal_tone_i_avg=0;
		iqcal_tone_q_avg=0;
		if( !hw_set_dxx_reg(phw_data, 0x3C, 0x00) ) 
			return 0;
		for(capture_time=0;capture_time<10;capture_time++)
		{
			
			
			reg_mode_ctrl &= ~(MASK_IQCAL_TONE_SEL|MASK_IQCAL_MODE);
			reg_mode_ctrl &= ~MASK_IQCAL_MODE;
			reg_mode_ctrl |= (MASK_CALIB_START|0x02);
			reg_mode_ctrl |= (MASK_CALIB_START|0x02|2<<2);
			hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
			PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

			
			hw_get_dxx_reg(phw_data, REG_CALIB_READ1, &val);
			PHY_DEBUG(("[CAL]    CALIB_READ1 = 0x%08X\n", val));

			iqcal_tone_i0 = _s13_to_s32(val & 0x00001FFF);
			iqcal_tone_q0 = _s13_to_s32((val & 0x03FFE000) >> 13);
			PHY_DEBUG(("[CAL]    ** iqcal_tone_i0=%d, iqcal_tone_q0=%d\n",
				   iqcal_tone_i0, iqcal_tone_q0));

			sqsum = iqcal_tone_i0*iqcal_tone_i0 +
			iqcal_tone_q0*iqcal_tone_q0;
			iq_mag_0_tx = (s32) _sqrt(sqsum);
			PHY_DEBUG(("[CAL]    ** iq_mag_0_tx=%d\n", iq_mag_0_tx));

			
			reg_mode_ctrl &= ~MASK_CALIB_START;
			hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
			PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

			
			
			
			hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &reg_mode_ctrl);
			reg_mode_ctrl &= ~MASK_IQCAL_MODE;
			reg_mode_ctrl |= (MASK_CALIB_START|0x03);
			hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
			PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

			
			hw_get_dxx_reg(phw_data, REG_CALIB_READ1, &val);
			PHY_DEBUG(("[CAL]    CALIB_READ1 = 0x%08X\n", val));

			iqcal_tone_i = _s13_to_s32(val & 0x00001FFF);
			iqcal_tone_q = _s13_to_s32((val & 0x03FFE000) >> 13);
			PHY_DEBUG(("[CAL]    ** iqcal_tone_i = %d, iqcal_tone_q = %d\n",
			iqcal_tone_i, iqcal_tone_q));
			if( capture_time == 0)
			{
				continue;
			}
			else
			{
				iqcal_tone_i_avg=( iqcal_tone_i_avg*(capture_time-1) +iqcal_tone_i)/capture_time;
				iqcal_tone_q_avg=( iqcal_tone_q_avg*(capture_time-1) +iqcal_tone_q)/capture_time;
			}
		}

		iqcal_tone_i = iqcal_tone_i_avg;
		iqcal_tone_q = iqcal_tone_q_avg;


		rot_i_b = (iqcal_tone_i * iqcal_tone_i0 +
				   iqcal_tone_q * iqcal_tone_q0) / 1024;
		rot_q_b = (iqcal_tone_i * iqcal_tone_q0 * (-1) +
				   iqcal_tone_q * iqcal_tone_i0) / 1024;
		PHY_DEBUG(("[CAL]    ** rot_i_b = %d, rot_q_b = %d\n",
				   rot_i_b, rot_q_b));

		
		divisor = ((iq_mag_0_tx * iq_mag_0_tx * 2)/1024 - rot_i_b) * 2;

		if (divisor == 0)
		{
			PHY_DEBUG(("[CAL] ** <_tx_iq_calibration_loop> ERROR *******\n"));
			PHY_DEBUG(("[CAL] ** divisor=0 to calculate EPS and THETA !!\n"));
			PHY_DEBUG(("[CAL] ******************************************\n"));
			break;
		}

		a_2 = (rot_i_b * 32768) / divisor;
		b_2 = (rot_q_b * (-32768)) / divisor;
		PHY_DEBUG(("[CAL]    ***** EPSILON/2 = %d\n", a_2));
		PHY_DEBUG(("[CAL]    ***** THETA/2   = %d\n", b_2));

		phw_data->iq_rsdl_gain_tx_d2 = a_2;
		phw_data->iq_rsdl_phase_tx_d2 = b_2;

		
		
		if ((abs(a_2) < a_2_threshold) && (abs(b_2) < b_2_threshold))
		{
			verify_count++;

			PHY_DEBUG(("[CAL] ** <_tx_iq_calibration_loop> *************\n"));
			PHY_DEBUG(("[CAL] ** VERIFY OK # %d !!\n", verify_count));
			PHY_DEBUG(("[CAL] ******************************************\n"));

			if (verify_count > 2)
			{
				PHY_DEBUG(("[CAL] ** <_tx_iq_calibration_loop> *********\n"));
				PHY_DEBUG(("[CAL] ** TX_IQ_CALIBRATION (EPS,THETA) OK !!\n"));
				PHY_DEBUG(("[CAL] **************************************\n"));
				return 0;
			}

			continue;
		}
		else
		{
			verify_count = 0;
		}

		_sin_cos(b_2, &sin_b, &cos_b);
		_sin_cos(b_2*2, &sin_2b, &cos_2b);
		PHY_DEBUG(("[CAL]    ** sin(b/2)=%d, cos(b/2)=%d\n", sin_b, cos_b));
		PHY_DEBUG(("[CAL]    ** sin(b)=%d, cos(b)=%d\n", sin_2b, cos_2b));

		if (cos_2b == 0)
		{
			PHY_DEBUG(("[CAL] ** <_tx_iq_calibration_loop> ERROR *******\n"));
			PHY_DEBUG(("[CAL] ** cos(b)=0 !!\n"));
			PHY_DEBUG(("[CAL] ******************************************\n"));
			break;
		}

		
		temp1 = (41943040/cos_2b)*cos_b;

		
		if (phw_data->revision == 0x2002) 
		{
			temp2 = (41943040/cos_2b)*sin_b*(-1);
		}
		else 
		{
			temp2 = (41943040*4/cos_2b)*sin_b*(-1);
		}

		tx_cal_flt_b[0] = _floor(temp1/(32768+a_2));
		tx_cal_flt_b[1] = _floor(temp2/(32768+a_2));
		tx_cal_flt_b[2] = _floor(temp2/(32768-a_2));
		tx_cal_flt_b[3] = _floor(temp1/(32768-a_2));
		PHY_DEBUG(("[CAL]    ** tx_cal_flt_b[0] = %d\n", tx_cal_flt_b[0]));
		PHY_DEBUG(("[CAL]       tx_cal_flt_b[1] = %d\n", tx_cal_flt_b[1]));
		PHY_DEBUG(("[CAL]       tx_cal_flt_b[2] = %d\n", tx_cal_flt_b[2]));
		PHY_DEBUG(("[CAL]       tx_cal_flt_b[3] = %d\n", tx_cal_flt_b[3]));

		tx_cal[2] = tx_cal_flt_b[2];
		tx_cal[2] = tx_cal[2] +3;
		tx_cal[1] = tx_cal[2];
		tx_cal[3] = tx_cal_flt_b[3] - 128;
		tx_cal[0] = -tx_cal[3]+1;

		PHY_DEBUG(("[CAL]       tx_cal[0] = %d\n", tx_cal[0]));
		PHY_DEBUG(("[CAL]       tx_cal[1] = %d\n", tx_cal[1]));
		PHY_DEBUG(("[CAL]       tx_cal[2] = %d\n", tx_cal[2]));
		PHY_DEBUG(("[CAL]       tx_cal[3] = %d\n", tx_cal[3]));

		
		
		
		
		
		
		
		

		
		if (phw_data->revision == 0x2002) 
		{
			hw_get_dxx_reg(phw_data, 0x54, &val);
			PHY_DEBUG(("[CAL]    ** 0x54 = 0x%08X\n", val));
			tx_cal_reg[0] = _s4_to_s32((val & 0xF0000000) >> 28);
			tx_cal_reg[1] = _s4_to_s32((val & 0x0F000000) >> 24);
			tx_cal_reg[2] = _s4_to_s32((val & 0x00F00000) >> 20);
			tx_cal_reg[3] = _s4_to_s32((val & 0x000F0000) >> 16);
		}
		else 
		{
			hw_get_dxx_reg(phw_data, 0x3C, &val);
			PHY_DEBUG(("[CAL]    ** 0x3C = 0x%08X\n", val));
			tx_cal_reg[0] = _s5_to_s32((val & 0xF8000000) >> 27);
			tx_cal_reg[1] = _s6_to_s32((val & 0x07E00000) >> 21);
			tx_cal_reg[2] = _s6_to_s32((val & 0x001F8000) >> 15);
			tx_cal_reg[3] = _s5_to_s32((val & 0x00007C00) >> 10);

		}

		PHY_DEBUG(("[CAL]    ** tx_cal_reg[0] = %d\n", tx_cal_reg[0]));
		PHY_DEBUG(("[CAL]       tx_cal_reg[1] = %d\n", tx_cal_reg[1]));
		PHY_DEBUG(("[CAL]       tx_cal_reg[2] = %d\n", tx_cal_reg[2]));
		PHY_DEBUG(("[CAL]       tx_cal_reg[3] = %d\n", tx_cal_reg[3]));

		if (phw_data->revision == 0x2002) 
		{
			if (((tx_cal_reg[0]==7) || (tx_cal_reg[0]==(-8))) &&
				((tx_cal_reg[3]==7) || (tx_cal_reg[3]==(-8))))
			{
				PHY_DEBUG(("[CAL] ** <_tx_iq_calibration_loop> *********\n"));
				PHY_DEBUG(("[CAL] ** TX_IQ_CALIBRATION SATUATION !!\n"));
				PHY_DEBUG(("[CAL] **************************************\n"));
				break;
			}
		}
		else 
		{
			if (((tx_cal_reg[0]==31) || (tx_cal_reg[0]==(-32))) &&
				((tx_cal_reg[3]==31) || (tx_cal_reg[3]==(-32))))
			{
				PHY_DEBUG(("[CAL] ** <_tx_iq_calibration_loop> *********\n"));
				PHY_DEBUG(("[CAL] ** TX_IQ_CALIBRATION SATUATION !!\n"));
				PHY_DEBUG(("[CAL] **************************************\n"));
				break;
			}
		}

		tx_cal[0] = tx_cal[0] + tx_cal_reg[0];
		tx_cal[1] = tx_cal[1] + tx_cal_reg[1];
		tx_cal[2] = tx_cal[2] + tx_cal_reg[2];
		tx_cal[3] = tx_cal[3] + tx_cal_reg[3];
		PHY_DEBUG(("[CAL]    ** apply tx_cal[0] = %d\n", tx_cal[0]));
		PHY_DEBUG(("[CAL]       apply tx_cal[1] = %d\n", tx_cal[1]));
		PHY_DEBUG(("[CAL]       apply tx_cal[2] = %d\n", tx_cal[2]));
		PHY_DEBUG(("[CAL]       apply tx_cal[3] = %d\n", tx_cal[3]));

		if (phw_data->revision == 0x2002) 
		{
			val &= 0x0000FFFF;
			val |= ((_s32_to_s4(tx_cal[0]) << 28)|
					(_s32_to_s4(tx_cal[1]) << 24)|
					(_s32_to_s4(tx_cal[2]) << 20)|
					(_s32_to_s4(tx_cal[3]) << 16));
			hw_set_dxx_reg(phw_data, 0x54, val);
			PHY_DEBUG(("[CAL]    ** CALIB_DATA = 0x%08X\n", val));
			return 0;
		}
		else 
		{
			val &= 0x000003FF;
			val |= ((_s32_to_s5(tx_cal[0]) << 27)|
					(_s32_to_s6(tx_cal[1]) << 21)|
					(_s32_to_s6(tx_cal[2]) << 15)|
					(_s32_to_s5(tx_cal[3]) << 10));
			hw_set_dxx_reg(phw_data, 0x3C, val);
			PHY_DEBUG(("[CAL]    ** TX_IQ_CALIBRATION = 0x%08X\n", val));
			return 0;
		}

		
		reg_mode_ctrl &= ~MASK_CALIB_START;
		hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
		PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

		loop--;
	}

	return 1;
}

void _tx_iq_calibration_winbond(struct hw_data *phw_data)
{
	u32     reg_agc_ctrl3;
#ifdef _DEBUG
	s32     tx_cal_reg[4];

#endif
	u32     reg_mode_ctrl;
	u32     val;
	u8      result;

	PHY_DEBUG(("[CAL] -> [4]_tx_iq_calibration()\n"));

	
	phy_set_rf_data(phw_data, 1, (1<<24)|0xEE3FC2);
	
	phy_set_rf_data(phw_data, 11, (11<<24)|0x19BDD6); 
	
	phy_set_rf_data(phw_data, 5, (5<<24)|0x24C60A); 
    
	phy_set_rf_data(phw_data, 6, (6<<24)|0x34880C); 
	
	phy_set_rf_data(phw_data, 0, (0<<24)|0xFDF1C0);
	
	
	
	

	msleep(30); 
	
	adjust_TXVGA_for_iq_mag( phw_data );

	
	hw_get_dxx_reg(phw_data, REG_AGC_CTRL3, &reg_agc_ctrl3);
	reg_agc_ctrl3 &= ~BIT(2);
	reg_agc_ctrl3 |= (MASK_LNA_FIX_GAIN|MASK_AGC_FIX);
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL3, reg_agc_ctrl3);

	hw_get_dxx_reg(phw_data, REG_AGC_CTRL5, &val);
	val |= MASK_AGC_FIX_GAIN;
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL5, val);

	result = _tx_iq_calibration_loop_winbond(phw_data, 150, 100);

	if (result > 0)
	{
		if (phw_data->revision == 0x2002) 
		{
			hw_get_dxx_reg(phw_data, 0x54, &val);
			val &= 0x0000FFFF;
			hw_set_dxx_reg(phw_data, 0x54, val);
		}
		else 
		{
			hw_get_dxx_reg(phw_data, 0x3C, &val);
			val &= 0x000003FF;
			hw_set_dxx_reg(phw_data, 0x3C, val);
		}

		result = _tx_iq_calibration_loop_winbond(phw_data, 300, 200);

		if (result > 0)
		{
			if (phw_data->revision == 0x2002) 
			{
				hw_get_dxx_reg(phw_data, 0x54, &val);
				val &= 0x0000FFFF;
				hw_set_dxx_reg(phw_data, 0x54, val);
			}
			else 
			{
				hw_get_dxx_reg(phw_data, 0x3C, &val);
				val &= 0x000003FF;
				hw_set_dxx_reg(phw_data, 0x3C, val);
			}

			result = _tx_iq_calibration_loop_winbond(phw_data, 500, 400);
			if (result > 0)
			{
				if (phw_data->revision == 0x2002) 
				{
					hw_get_dxx_reg(phw_data, 0x54, &val);
					val &= 0x0000FFFF;
					hw_set_dxx_reg(phw_data, 0x54, val);
				}
				else 
				{
					hw_get_dxx_reg(phw_data, 0x3C, &val);
					val &= 0x000003FF;
					hw_set_dxx_reg(phw_data, 0x3C, val);
				}


				result = _tx_iq_calibration_loop_winbond(phw_data, 700, 500);

				if (result > 0)
				{
					PHY_DEBUG(("[CAL] ** <_tx_iq_calibration> **************\n"));
					PHY_DEBUG(("[CAL] ** TX_IQ_CALIBRATION FAILURE !!\n"));
					PHY_DEBUG(("[CAL] **************************************\n"));

					if (phw_data->revision == 0x2002) 
					{
						hw_get_dxx_reg(phw_data, 0x54, &val);
						val &= 0x0000FFFF;
						hw_set_dxx_reg(phw_data, 0x54, val);
					}
					else 
					{
						hw_get_dxx_reg(phw_data, 0x3C, &val);
						val &= 0x000003FF;
						hw_set_dxx_reg(phw_data, 0x3C, val);
					}
				}
			}
		}
	}

	
	hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &reg_mode_ctrl);
	reg_mode_ctrl &= ~MASK_CALIB_START;
	hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
	PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

	
	
	reg_agc_ctrl3 |= BIT(2);
	reg_agc_ctrl3 &= ~(MASK_LNA_FIX_GAIN|MASK_AGC_FIX);
	hw_set_dxx_reg(phw_data, REG_AGC_CTRL3, reg_agc_ctrl3);

#ifdef _DEBUG
	if (phw_data->revision == 0x2002) 
	{
		hw_get_dxx_reg(phw_data, 0x54, &val);
		PHY_DEBUG(("[CAL]    ** 0x54 = 0x%08X\n", val));
		tx_cal_reg[0] = _s4_to_s32((val & 0xF0000000) >> 28);
		tx_cal_reg[1] = _s4_to_s32((val & 0x0F000000) >> 24);
		tx_cal_reg[2] = _s4_to_s32((val & 0x00F00000) >> 20);
		tx_cal_reg[3] = _s4_to_s32((val & 0x000F0000) >> 16);
	}
	else 
	{
		hw_get_dxx_reg(phw_data, 0x3C, &val);
		PHY_DEBUG(("[CAL]    ** 0x3C = 0x%08X\n", val));
		tx_cal_reg[0] = _s5_to_s32((val & 0xF8000000) >> 27);
		tx_cal_reg[1] = _s6_to_s32((val & 0x07E00000) >> 21);
		tx_cal_reg[2] = _s6_to_s32((val & 0x001F8000) >> 15);
		tx_cal_reg[3] = _s5_to_s32((val & 0x00007C00) >> 10);

	}

	PHY_DEBUG(("[CAL]    ** tx_cal_reg[0] = %d\n", tx_cal_reg[0]));
	PHY_DEBUG(("[CAL]       tx_cal_reg[1] = %d\n", tx_cal_reg[1]));
	PHY_DEBUG(("[CAL]       tx_cal_reg[2] = %d\n", tx_cal_reg[2]));
	PHY_DEBUG(("[CAL]       tx_cal_reg[3] = %d\n", tx_cal_reg[3]));
#endif


	
	
}


u8 _rx_iq_calibration_loop_winbond(struct hw_data *phw_data, u16 factor, u32 frequency)
{
	u32     reg_mode_ctrl;
	s32     iqcal_tone_i;
	s32     iqcal_tone_q;
	s32     iqcal_image_i;
	s32     iqcal_image_q;
	s32     rot_tone_i_b;
	s32     rot_tone_q_b;
	s32     rot_image_i_b;
	s32     rot_image_q_b;
	s32     rx_cal_flt_b[4];
	s32     rx_cal[4];
	s32     rx_cal_reg[4];
	s32     a_2, b_2;
	s32     sin_b, sin_2b;
	s32     cos_b, cos_2b;
	s32     temp1, temp2;
	u32     val;
	u16     loop;

	u32     pwr_tone;
	u32     pwr_image;
	u8      verify_count;

	s32     iqcal_tone_i_avg,iqcal_tone_q_avg;
	s32     iqcal_image_i_avg,iqcal_image_q_avg;
	u16		capture_time;

	PHY_DEBUG(("[CAL] -> [5]_rx_iq_calibration_loop()\n"));
	PHY_DEBUG(("[CAL] ** factor = %d\n", factor));



	hw_get_cxx_reg(phw_data, 0x80, &val);
	val |= BIT(19);
	hw_set_cxx_reg(phw_data, 0x80, val);


	hw_get_cxx_reg(phw_data, 0xE4, &val);
	val |= BIT(0);
	hw_set_cxx_reg(phw_data, 0xE4, val);
	PHY_DEBUG(("[CAL] ** RF_CTRL(0xE4) = 0x%08X", val));

	hw_set_dxx_reg(phw_data, 0x58, 0x44444444); 

	

	hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &reg_mode_ctrl);
	PHY_DEBUG(("[CAL]    MODE_CTRL (read) = 0x%08X\n", reg_mode_ctrl));

	verify_count = 0;

	
	
	loop = LOOP_TIMES;
	while (loop > 0)
	{
		PHY_DEBUG(("[CAL] [%d.] <_rx_iq_calibration_loop>\n", (LOOP_TIMES-loop+1)));
		iqcal_tone_i_avg=0;
		iqcal_tone_q_avg=0;
		iqcal_image_i_avg=0;
		iqcal_image_q_avg=0;
		capture_time=0;

		for(capture_time=0; capture_time<10; capture_time++)
		{
		
		reg_mode_ctrl &= ~MASK_CALIB_START;
		if( !hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl) )
			return 0;
		PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

		reg_mode_ctrl &= ~MASK_IQCAL_MODE;
		reg_mode_ctrl |= (MASK_CALIB_START|0x1);
		hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
		PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

		
		hw_get_dxx_reg(phw_data, REG_CALIB_READ1, &val);
		PHY_DEBUG(("[CAL]    CALIB_READ1 = 0x%08X\n", val));

		iqcal_tone_i = _s13_to_s32(val & 0x00001FFF);
		iqcal_tone_q = _s13_to_s32((val & 0x03FFE000) >> 13);
		PHY_DEBUG(("[CAL]    ** iqcal_tone_i = %d, iqcal_tone_q = %d\n",
				   iqcal_tone_i, iqcal_tone_q));

		hw_get_dxx_reg(phw_data, REG_CALIB_READ2, &val);
		PHY_DEBUG(("[CAL]    CALIB_READ2 = 0x%08X\n", val));

		iqcal_image_i = _s13_to_s32(val & 0x00001FFF);
		iqcal_image_q = _s13_to_s32((val & 0x03FFE000) >> 13);
		PHY_DEBUG(("[CAL]    ** iqcal_image_i = %d, iqcal_image_q = %d\n",
				   iqcal_image_i, iqcal_image_q));
			if( capture_time == 0)
			{
				continue;
			}
			else
			{
				iqcal_image_i_avg=( iqcal_image_i_avg*(capture_time-1) +iqcal_image_i)/capture_time;
				iqcal_image_q_avg=( iqcal_image_q_avg*(capture_time-1) +iqcal_image_q)/capture_time;
				iqcal_tone_i_avg=( iqcal_tone_i_avg*(capture_time-1) +iqcal_tone_i)/capture_time;
				iqcal_tone_q_avg=( iqcal_tone_q_avg*(capture_time-1) +iqcal_tone_q)/capture_time;
			}
		}


		iqcal_image_i = iqcal_image_i_avg;
		iqcal_image_q = iqcal_image_q_avg;
		iqcal_tone_i = iqcal_tone_i_avg;
		iqcal_tone_q = iqcal_tone_q_avg;

		
		rot_tone_i_b = (iqcal_tone_i * iqcal_tone_i +
						iqcal_tone_q * iqcal_tone_q) / 1024;
		rot_tone_q_b = (iqcal_tone_i * iqcal_tone_q * (-1) +
						iqcal_tone_q * iqcal_tone_i) / 1024;
		rot_image_i_b = (iqcal_image_i * iqcal_tone_i -
						 iqcal_image_q * iqcal_tone_q) / 1024;
		rot_image_q_b = (iqcal_image_i * iqcal_tone_q +
						 iqcal_image_q * iqcal_tone_i) / 1024;

		PHY_DEBUG(("[CAL]    ** rot_tone_i_b  = %d\n", rot_tone_i_b));
		PHY_DEBUG(("[CAL]    ** rot_tone_q_b  = %d\n", rot_tone_q_b));
		PHY_DEBUG(("[CAL]    ** rot_image_i_b = %d\n", rot_image_i_b));
		PHY_DEBUG(("[CAL]    ** rot_image_q_b = %d\n", rot_image_q_b));

		
		if (rot_tone_i_b == 0)
		{
			PHY_DEBUG(("[CAL] ** <_rx_iq_calibration_loop> ERROR *******\n"));
			PHY_DEBUG(("[CAL] ** rot_tone_i_b=0 to calculate EPS and THETA !!\n"));
			PHY_DEBUG(("[CAL] ******************************************\n"));
			break;
		}

		a_2 = (rot_image_i_b * 32768) / rot_tone_i_b -
			phw_data->iq_rsdl_gain_tx_d2;
		b_2 = (rot_image_q_b * 32768) / rot_tone_i_b -
			phw_data->iq_rsdl_phase_tx_d2;

		PHY_DEBUG(("[CAL]    ** iq_rsdl_gain_tx_d2 = %d\n", phw_data->iq_rsdl_gain_tx_d2));
		PHY_DEBUG(("[CAL]    ** iq_rsdl_phase_tx_d2= %d\n", phw_data->iq_rsdl_phase_tx_d2));
		PHY_DEBUG(("[CAL]    ***** EPSILON/2 = %d\n", a_2));
		PHY_DEBUG(("[CAL]    ***** THETA/2   = %d\n", b_2));

		_sin_cos(b_2, &sin_b, &cos_b);
		_sin_cos(b_2*2, &sin_2b, &cos_2b);
		PHY_DEBUG(("[CAL]    ** sin(b/2)=%d, cos(b/2)=%d\n", sin_b, cos_b));
		PHY_DEBUG(("[CAL]    ** sin(b)=%d, cos(b)=%d\n", sin_2b, cos_2b));

		if (cos_2b == 0)
		{
			PHY_DEBUG(("[CAL] ** <_rx_iq_calibration_loop> ERROR *******\n"));
			PHY_DEBUG(("[CAL] ** cos(b)=0 !!\n"));
			PHY_DEBUG(("[CAL] ******************************************\n"));
			break;
		}

		
		temp1 = (41943040/cos_2b)*cos_b;

		
		if (phw_data->revision == 0x2002) 
		{
			temp2 = (41943040/cos_2b)*sin_b*(-1);
		}
		else 
		{
			temp2 = (41943040*4/cos_2b)*sin_b*(-1);
		}

		rx_cal_flt_b[0] = _floor(temp1/(32768+a_2));
		rx_cal_flt_b[1] = _floor(temp2/(32768-a_2));
		rx_cal_flt_b[2] = _floor(temp2/(32768+a_2));
		rx_cal_flt_b[3] = _floor(temp1/(32768-a_2));

		PHY_DEBUG(("[CAL]    ** rx_cal_flt_b[0] = %d\n", rx_cal_flt_b[0]));
		PHY_DEBUG(("[CAL]       rx_cal_flt_b[1] = %d\n", rx_cal_flt_b[1]));
		PHY_DEBUG(("[CAL]       rx_cal_flt_b[2] = %d\n", rx_cal_flt_b[2]));
		PHY_DEBUG(("[CAL]       rx_cal_flt_b[3] = %d\n", rx_cal_flt_b[3]));

		rx_cal[0] = rx_cal_flt_b[0] - 128;
		rx_cal[1] = rx_cal_flt_b[1];
		rx_cal[2] = rx_cal_flt_b[2];
		rx_cal[3] = rx_cal_flt_b[3] - 128;
		PHY_DEBUG(("[CAL]    ** rx_cal[0] = %d\n", rx_cal[0]));
		PHY_DEBUG(("[CAL]       rx_cal[1] = %d\n", rx_cal[1]));
		PHY_DEBUG(("[CAL]       rx_cal[2] = %d\n", rx_cal[2]));
		PHY_DEBUG(("[CAL]       rx_cal[3] = %d\n", rx_cal[3]));

		
		pwr_tone = (iqcal_tone_i*iqcal_tone_i + iqcal_tone_q*iqcal_tone_q);
		pwr_image = (iqcal_image_i*iqcal_image_i + iqcal_image_q*iqcal_image_q)*factor;

		PHY_DEBUG(("[CAL]    ** pwr_tone  = %d\n", pwr_tone));
		PHY_DEBUG(("[CAL]    ** pwr_image  = %d\n", pwr_image));

		if (pwr_tone > pwr_image)
		{
			verify_count++;

			PHY_DEBUG(("[CAL] ** <_rx_iq_calibration_loop> *************\n"));
			PHY_DEBUG(("[CAL] ** VERIFY OK # %d !!\n", verify_count));
			PHY_DEBUG(("[CAL] ******************************************\n"));

			if (verify_count > 2)
			{
				PHY_DEBUG(("[CAL] ** <_rx_iq_calibration_loop> *********\n"));
				PHY_DEBUG(("[CAL] ** RX_IQ_CALIBRATION OK !!\n"));
				PHY_DEBUG(("[CAL] **************************************\n"));
				return 0;
			}

			continue;
		}
		
		hw_get_dxx_reg(phw_data, 0x54, &val);
		PHY_DEBUG(("[CAL]    ** 0x54 = 0x%08X\n", val));

		if (phw_data->revision == 0x2002) 
		{
			rx_cal_reg[0] = _s4_to_s32((val & 0x0000F000) >> 12);
			rx_cal_reg[1] = _s4_to_s32((val & 0x00000F00) >>  8);
			rx_cal_reg[2] = _s4_to_s32((val & 0x000000F0) >>  4);
			rx_cal_reg[3] = _s4_to_s32((val & 0x0000000F));
		}
		else 
		{
			rx_cal_reg[0] = _s5_to_s32((val & 0xF8000000) >> 27);
			rx_cal_reg[1] = _s6_to_s32((val & 0x07E00000) >> 21);
			rx_cal_reg[2] = _s6_to_s32((val & 0x001F8000) >> 15);
			rx_cal_reg[3] = _s5_to_s32((val & 0x00007C00) >> 10);
		}

		PHY_DEBUG(("[CAL]    ** rx_cal_reg[0] = %d\n", rx_cal_reg[0]));
		PHY_DEBUG(("[CAL]       rx_cal_reg[1] = %d\n", rx_cal_reg[1]));
		PHY_DEBUG(("[CAL]       rx_cal_reg[2] = %d\n", rx_cal_reg[2]));
		PHY_DEBUG(("[CAL]       rx_cal_reg[3] = %d\n", rx_cal_reg[3]));

		if (phw_data->revision == 0x2002) 
		{
			if (((rx_cal_reg[0]==7) || (rx_cal_reg[0]==(-8))) &&
				((rx_cal_reg[3]==7) || (rx_cal_reg[3]==(-8))))
			{
				PHY_DEBUG(("[CAL] ** <_rx_iq_calibration_loop> *********\n"));
				PHY_DEBUG(("[CAL] ** RX_IQ_CALIBRATION SATUATION !!\n"));
				PHY_DEBUG(("[CAL] **************************************\n"));
				break;
			}
		}
		else 
		{
			if (((rx_cal_reg[0]==31) || (rx_cal_reg[0]==(-32))) &&
				((rx_cal_reg[3]==31) || (rx_cal_reg[3]==(-32))))
			{
				PHY_DEBUG(("[CAL] ** <_rx_iq_calibration_loop> *********\n"));
				PHY_DEBUG(("[CAL] ** RX_IQ_CALIBRATION SATUATION !!\n"));
				PHY_DEBUG(("[CAL] **************************************\n"));
				break;
			}
		}

		rx_cal[0] = rx_cal[0] + rx_cal_reg[0];
		rx_cal[1] = rx_cal[1] + rx_cal_reg[1];
		rx_cal[2] = rx_cal[2] + rx_cal_reg[2];
		rx_cal[3] = rx_cal[3] + rx_cal_reg[3];
		PHY_DEBUG(("[CAL]    ** apply rx_cal[0] = %d\n", rx_cal[0]));
		PHY_DEBUG(("[CAL]       apply rx_cal[1] = %d\n", rx_cal[1]));
		PHY_DEBUG(("[CAL]       apply rx_cal[2] = %d\n", rx_cal[2]));
		PHY_DEBUG(("[CAL]       apply rx_cal[3] = %d\n", rx_cal[3]));

		hw_get_dxx_reg(phw_data, 0x54, &val);
		if (phw_data->revision == 0x2002) 
		{
			val &= 0x0000FFFF;
			val |= ((_s32_to_s4(rx_cal[0]) << 12)|
					(_s32_to_s4(rx_cal[1]) <<  8)|
					(_s32_to_s4(rx_cal[2]) <<  4)|
					(_s32_to_s4(rx_cal[3])));
			hw_set_dxx_reg(phw_data, 0x54, val);
		}
		else 
		{
			val &= 0x000003FF;
			val |= ((_s32_to_s5(rx_cal[0]) << 27)|
					(_s32_to_s6(rx_cal[1]) << 21)|
					(_s32_to_s6(rx_cal[2]) << 15)|
					(_s32_to_s5(rx_cal[3]) << 10));
			hw_set_dxx_reg(phw_data, 0x54, val);

			if( loop == 3 )
			return 0;
		}
		PHY_DEBUG(("[CAL]    ** CALIB_DATA = 0x%08X\n", val));

		loop--;
	}

	return 1;
}




void _rx_iq_calibration_winbond(struct hw_data *phw_data, u32 frequency)
{

#ifdef _DEBUG
	s32     rx_cal_reg[4];
	u32     val;
#endif

	u8      result;

	PHY_DEBUG(("[CAL] -> [5]_rx_iq_calibration()\n"));

	
	
	phy_set_rf_data(phw_data, 1, (1<<24)|0xEFBFC2);
	
	phy_set_rf_data(phw_data, 11, (11<<24)|0x1A05D6);
	
	phy_set_rf_data(phw_data, 5, (5<<24)| phw_data->txvga_setting_for_cal);
	
	phy_set_rf_data(phw_data, 6, (6<<24)|0x06834C);
	
	phy_set_rf_data(phw_data, 0, (0<<24)|0xFFF1C0);

	
	
	

	result = _rx_iq_calibration_loop_winbond(phw_data, 12589, frequency);

	if (result > 0)
	{
		_reset_rx_cal(phw_data);
		result = _rx_iq_calibration_loop_winbond(phw_data, 7943, frequency);

		if (result > 0)
		{
			_reset_rx_cal(phw_data);
			result = _rx_iq_calibration_loop_winbond(phw_data, 5011, frequency);

			if (result > 0)
			{
				PHY_DEBUG(("[CAL] ** <_rx_iq_calibration> **************\n"));
				PHY_DEBUG(("[CAL] ** RX_IQ_CALIBRATION FAILURE !!\n"));
				PHY_DEBUG(("[CAL] **************************************\n"));
				_reset_rx_cal(phw_data);
			}
		}
	}

#ifdef _DEBUG
	hw_get_dxx_reg(phw_data, 0x54, &val);
	PHY_DEBUG(("[CAL]    ** 0x54 = 0x%08X\n", val));

	if (phw_data->revision == 0x2002) 
	{
		rx_cal_reg[0] = _s4_to_s32((val & 0x0000F000) >> 12);
		rx_cal_reg[1] = _s4_to_s32((val & 0x00000F00) >>  8);
		rx_cal_reg[2] = _s4_to_s32((val & 0x000000F0) >>  4);
		rx_cal_reg[3] = _s4_to_s32((val & 0x0000000F));
	}
	else 
	{
		rx_cal_reg[0] = _s5_to_s32((val & 0xF8000000) >> 27);
		rx_cal_reg[1] = _s6_to_s32((val & 0x07E00000) >> 21);
		rx_cal_reg[2] = _s6_to_s32((val & 0x001F8000) >> 15);
		rx_cal_reg[3] = _s5_to_s32((val & 0x00007C00) >> 10);
	}

	PHY_DEBUG(("[CAL]    ** rx_cal_reg[0] = %d\n", rx_cal_reg[0]));
	PHY_DEBUG(("[CAL]       rx_cal_reg[1] = %d\n", rx_cal_reg[1]));
	PHY_DEBUG(("[CAL]       rx_cal_reg[2] = %d\n", rx_cal_reg[2]));
	PHY_DEBUG(("[CAL]       rx_cal_reg[3] = %d\n", rx_cal_reg[3]));
#endif

}


void phy_calibration_winbond(struct hw_data *phw_data, u32 frequency)
{
	u32     reg_mode_ctrl;
	u32     iq_alpha;

	PHY_DEBUG(("[CAL] -> phy_calibration_winbond()\n"));

	
	hw_get_cxx_reg(phw_data, 0x80, &mac_ctrl);
	hw_get_cxx_reg(phw_data, 0xE4, &rf_ctrl);
	hw_get_dxx_reg(phw_data, 0x58, &iq_alpha);



	_rxadc_dc_offset_cancellation_winbond(phw_data, frequency);
	
	

	_tx_iq_calibration_winbond(phw_data);
	_rx_iq_calibration_winbond(phw_data, frequency);

	
	hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &reg_mode_ctrl);
	reg_mode_ctrl &= ~(MASK_IQCAL_TONE_SEL|MASK_IQCAL_MODE|MASK_CALIB_START); 
	hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
	PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

	
	hw_set_cxx_reg(phw_data, 0x80, mac_ctrl);
	hw_set_cxx_reg(phw_data, 0xE4, rf_ctrl);
	hw_set_dxx_reg(phw_data, 0x58, iq_alpha);


	
	phy_init_rf(phw_data);

}


void phy_set_rf_data(  struct hw_data * pHwData,  u32 index,  u32 value )
{
   u32 ltmp=0;

    switch( pHwData->phy_type )
	{
		case RF_MAXIM_2825:
		case RF_MAXIM_V1: 
			ltmp = (1 << 31) | (0 << 30) | (18 << 24) | BitReverse( value, 18 );
			break;

		case RF_MAXIM_2827:
			ltmp = (1 << 31) | (0 << 30) | (18 << 24) | BitReverse( value, 18 );
			break;

		case RF_MAXIM_2828:
			ltmp = (1 << 31) | (0 << 30) | (18 << 24) | BitReverse( value, 18 );
			break;

		case RF_MAXIM_2829:
			ltmp = (1 << 31) | (0 << 30) | (18 << 24) | BitReverse( value, 18 );
			break;

		case RF_AIROHA_2230:
		case RF_AIROHA_2230S: 
			ltmp = (1 << 31) | (0 << 30) | (20 << 24) | BitReverse( value, 20 );
			break;

		case RF_AIROHA_7230:
			ltmp = (1 << 31) | (0 << 30) | (24 << 24) | (value&0xffffff);
			break;

		case RF_WB_242:
		case RF_WB_242_1: 
			ltmp = (1 << 31) | (0 << 30) | (24 << 24) | BitReverse( value, 24 );
			break;
	}

	Wb35Reg_WriteSync( pHwData, 0x0864, ltmp );
}


unsigned char adjust_TXVGA_for_iq_mag(struct hw_data *phw_data)
{
	int init_txvga = 0;
	u32     reg_mode_ctrl;
	u32     val;
	s32     iqcal_tone_i0;
	s32     iqcal_tone_q0;
	u32     sqsum;
	s32     iq_mag_0_tx;
	u8		reg_state;
	int		current_txvga;


	reg_state = 0;
	for( init_txvga=0; init_txvga<10; init_txvga++)
	{
		current_txvga = ( 0x24C40A|(init_txvga<<6) );
		phy_set_rf_data(phw_data, 5, ((5<<24)|current_txvga) );
		phw_data->txvga_setting_for_cal = current_txvga;

		msleep(30); 

		if( !hw_get_dxx_reg(phw_data, REG_MODE_CTRL, &reg_mode_ctrl) ) 
			return false;

		PHY_DEBUG(("[CAL]    MODE_CTRL (read) = 0x%08X\n", reg_mode_ctrl));

		
		
		reg_mode_ctrl &= ~(MASK_IQCAL_TONE_SEL|MASK_IQCAL_MODE);
		reg_mode_ctrl &= ~MASK_IQCAL_MODE;
		reg_mode_ctrl |= (MASK_CALIB_START|0x02);
		reg_mode_ctrl |= (MASK_CALIB_START|0x02|2<<2);
		hw_set_dxx_reg(phw_data, REG_MODE_CTRL, reg_mode_ctrl);
		PHY_DEBUG(("[CAL]    MODE_CTRL (write) = 0x%08X\n", reg_mode_ctrl));

		udelay(1); 

		udelay(300); 

		
		hw_get_dxx_reg(phw_data, REG_CALIB_READ1, &val);

		PHY_DEBUG(("[CAL]    CALIB_READ1 = 0x%08X\n", val));
		udelay(300); 

		iqcal_tone_i0 = _s13_to_s32(val & 0x00001FFF);
		iqcal_tone_q0 = _s13_to_s32((val & 0x03FFE000) >> 13);
		PHY_DEBUG(("[CAL]    ** iqcal_tone_i0=%d, iqcal_tone_q0=%d\n",
				   iqcal_tone_i0, iqcal_tone_q0));

		sqsum = iqcal_tone_i0*iqcal_tone_i0 + iqcal_tone_q0*iqcal_tone_q0;
		iq_mag_0_tx = (s32) _sqrt(sqsum);
		PHY_DEBUG(("[CAL]    ** auto_adjust_txvga_for_iq_mag_0_tx=%d\n", iq_mag_0_tx));

		if( iq_mag_0_tx>=700 && iq_mag_0_tx<=1750 )
			break;
		else if(iq_mag_0_tx > 1750)
		{
			init_txvga=-2;
			continue;
		}
		else
			continue;

	}

	if( iq_mag_0_tx>=700 && iq_mag_0_tx<=1750 )
		return true;
	else
		return false;
}



