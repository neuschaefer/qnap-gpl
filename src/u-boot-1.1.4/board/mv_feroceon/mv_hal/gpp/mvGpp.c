/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "mvGpp.h"

/* defines  */       
#ifdef MV_DEBUG         
	#define DB(x)	x
#else                
	#define DB(x)    
#endif	             

static MV_VOID gppRegSet(MV_U32 group, MV_U32 regOffs,MV_U32 mask,MV_U32 value);

/* modify by QNAP for recovery button *****************************************/
void QNAP_recovery_init()
{
    /* UART 1: PIC */
    gppRegSet(0, mvCtrlMppRegGet(1), 0x00f00000, 0x03 << 20);
    mvGppTypeSet(0, MV_GPP13, MV_GPP13 & MV_GPP_IN);
//Patch by QNAP:Fix detect DRAM size 
	//MV_GPP36 for DRAM ID
	gppRegSet(0,mvCtrlMppRegGet(4),0x000f0000,0);
	mvGppTypeSet(1,MV_GPP4,MV_GPP4 & MV_GPP_IN);
///////////////////////////////////////	
#if defined(TS119) || defined(TS219) || defined(TS118) || defined(TS218)
//Patch by QNAP:Initial HDD error LED
	gppRegSet(0, mvCtrlMppRegGet(4), 0x0000ff00, 0);
	mvGppTypeSet(1, MV_GPP2 | MV_GPP3, (MV_GPP2 | MV_GPP3)& MV_GPP_OUT);
	mvGppValueSet(1,MV_GPP2 | MV_GPP3,MV_GPP2 | MV_GPP3);
///////////////////////////////////////	
//Patch by QNAP:Fix detect Model:High for TS119/TS219;Low for TS119P/TS219P
	//MV_GPP38 for Model detect
	gppRegSet(0,mvCtrlMppRegGet(4),0x0f000000,0);
	mvGppTypeSet(1,MV_GPP6,MV_GPP6 & MV_GPP_IN);
//////////////////////////////////////
    /* Recovery Button */
    gppRegSet(0, mvCtrlMppRegGet(2), 0x0000000f, 0);
    mvGppTypeSet(0, MV_GPP16, MV_GPP16 & MV_GPP_IN);
#elif defined(TS419)
	//MV_GPP44 for Model detect:High for TS419;Low for TS419U
	gppRegSet(0,mvCtrlMppRegGet(5),0x000f0000,0);
	mvGppTypeSet(1,MV_GPP12,MV_GPP12 & MV_GPP_IN);
	//MV_GPP45 for Console/LCM 0:LCM 1:Console
	gppRegSet(0,mvCtrlMppRegGet(5),0x00f00000,0);
	mvGppTypeSet(1,MV_GPP13,MV_GPP13 & MV_GPP_IN);	
    /* Recovery Button */
    gppRegSet(0, mvCtrlMppRegGet(4), 0x00f00000, 0);
    mvGppTypeSet(1, MV_GPP5, MV_GPP5 & MV_GPP_IN);
#endif
}
 
MV_BOOL QNAP_recovery_detect()
{
#if defined(TS119) || defined(TS219) || defined(TS118) || defined(TS218)
	return !mvGppValueGet(0,MV_GPP16);
#elif defined(TS419)
	return !mvGppValueGet(1,MV_GPP5);
#endif
}
/******************************************************************************/

/*******************************************************************************
* mvGppTypeSet - Enable a GPP (OUT) pin
*
* DESCRIPTION:
*
* INPUT:
*       group - GPP group number
*       mask  - 32bit mask value. Each set bit in the mask means that the type
*               of corresponding GPP will be set. Other GPPs are ignored.
*       value - 32bit value that describes GPP type per pin.
*
* OUTPUT:
*       None.
*
* EXAMPLE:
*       Set GPP8 to input and GPP15 to output.
*       mvGppTypeSet(0, (GPP8 | GPP15), 
*                    ((MV_GPP_IN & GPP8) | (MV_GPP_OUT & GPP15)) );
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_STATUS mvGppTypeSet(MV_U32 group, MV_U32 mask, MV_U32 value)
{
	if (group >= MV_GPP_MAX_GROUP)
	{
		DB(mvOsPrintf("mvGppTypeSet: ERR. invalid group number \n"));
		return MV_BAD_PARAM;
	}

	gppRegSet(group, GPP_DATA_OUT_EN_REG(group), mask, value);

	return MV_OK;

}

/*******************************************************************************
* mvGppBlinkEn - Set a GPP (IN) Pin list to blink every ~100ms
*
* DESCRIPTION:
*
* INPUT:
*       group - GPP group number
*       mask  - 32bit mask value. Each set bit in the mask means that the type
*               of corresponding GPP will be set. Other GPPs are ignored.
*       value - 32bit value that describes GPP blink per pin.
*
* OUTPUT:
*       None.
*
* EXAMPLE:
*       Set GPP8 to be static and GPP15 to be blinking.
*       mvGppBlinkEn(0, (GPP8 | GPP15), 
*                    ((MV_GPP_OUT_STATIC & GPP8) | (MV_GPP_OUT_BLINK & GPP15)) );
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_STATUS mvGppBlinkEn(MV_U32 group, MV_U32 mask, MV_U32 value)
{
	if (group >= MV_GPP_MAX_GROUP)
	{
		DB(mvOsPrintf("mvGppBlinkEn: ERR. invalid group number \n"));
		return MV_BAD_PARAM;
	}

	gppRegSet(group, GPP_BLINK_EN_REG(group), mask, value);

	return MV_OK;

}
/*******************************************************************************
* mvGppPolaritySet - Set a GPP (IN) Pin list Polarity mode
*
* DESCRIPTION:
*
* INPUT:
*       group - GPP group number
*       mask  - 32bit mask value. Each set bit in the mask means that the type
*               of corresponding GPP will be set. Other GPPs are ignored.
*       value - 32bit value that describes GPP polarity per pin.
*
* OUTPUT:
*       None.
*
* EXAMPLE:
*       Set GPP8 to the actual pin value and GPP15 to be inverted.
*       mvGppPolaritySet(0, (GPP8 | GPP15), 
*                    ((MV_GPP_IN_ORIGIN & GPP8) | (MV_GPP_IN_INVERT & GPP15)) );
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_STATUS mvGppPolaritySet(MV_U32 group, MV_U32 mask, MV_U32 value)
{
	if (group >= MV_GPP_MAX_GROUP)
	{
		DB(mvOsPrintf("mvGppPolaritySet: ERR. invalid group number \n"));
		return MV_BAD_PARAM;
	}

	gppRegSet(group, GPP_DATA_IN_POL_REG(group), mask, value);

	return MV_OK;

}

/*******************************************************************************
* mvGppPolarityGet - Get a value of relevant bits from GPP Polarity register.
*
* DESCRIPTION:
*
* INPUT:
*       group - GPP group number
*       mask  - 32bit mask value. Each set bit in the mask means that the 
*               returned value is valid for it.
*
* OUTPUT:
*       None.
*
* EXAMPLE:
*       Get GPP8 and GPP15 value.
*       mvGppPolarityGet(0, (GPP8 | GPP15));
*
* RETURN:
*       32bit value that describes GPP polatity mode per pin.
*
*******************************************************************************/
MV_U32  mvGppPolarityGet(MV_U32 group, MV_U32 mask)
{
    MV_U32  regVal;

  	if (group >= MV_GPP_MAX_GROUP)
	{
		DB(mvOsPrintf("mvGppActiveSet: Error invalid group number \n"));
		return MV_ERROR;
	}
    regVal = MV_REG_READ(GPP_DATA_IN_POL_REG(group));
    
    return (regVal & mask);
}

/*******************************************************************************
* mvGppValueGet - Get a GPP Pin list value.
*
* DESCRIPTION:
*       This function get GPP value.
*
* INPUT:
*       group - GPP group number
*       mask  - 32bit mask value. Each set bit in the mask means that the 
*               returned value is valid for it.
*
* OUTPUT:
*       None.
*
* EXAMPLE:
*       Get GPP8 and GPP15 value.
*       mvGppValueGet(0, (GPP8 | GPP15));
*
* RETURN:
*       32bit value that describes GPP activity mode per pin.
*
*******************************************************************************/
MV_U32 mvGppValueGet(MV_U32 group, MV_U32 mask)
{
	MV_U32 gppData;

	gppData = MV_REG_READ(GPP_DATA_IN_REG(group));

	gppData &= mask;

	return gppData;

}

/*******************************************************************************
* mvGppValueSet - Set a GPP Pin list value.
*
* DESCRIPTION:
*       This function set value for given GPP pin list.
*
* INPUT:
*       group - GPP group number
*       mask  - 32bit mask value. Each set bit in the mask means that the 
*               value of corresponding GPP will be set accordingly. Other GPP 
*               are not affected.
*       value - 32bit value that describes GPP value per pin.
*
* OUTPUT:
*       None.
*
* EXAMPLE:
*       Set GPP8 value of '0' and GPP15 value of '1'.
*       mvGppActiveSet(0, (GPP8 | GPP15), ((0 & GPP8) | (GPP15)) );
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_STATUS mvGppValueSet (MV_U32 group, MV_U32 mask, MV_U32 value)
{
	MV_U32 outEnable;
	MV_U32 i;

	if (group >= MV_GPP_MAX_GROUP)
	{
		DB(mvOsPrintf("mvGppValueSet: Error invalid group number \n"));
		return MV_BAD_PARAM;
	}

	/* verify that the gpp pin is configured as output 		*/
	/* Note that in the register out enabled -> bit = '0'. 	*/
	outEnable = ~MV_REG_READ(GPP_DATA_OUT_EN_REG(group));

	for (i = 0 ; i < 32 ;i++)
	{
		if (((mask & (1 << i)) & (outEnable & (1 << i))) != (mask & (1 << i)))
		{
			mvOsPrintf("mvGppValueSet: Err. An attempt to set output "\
					   "value to GPP %d in input mode.\n", i);
			return MV_ERROR;
		}
	}

	gppRegSet(group, GPP_DATA_OUT_REG(group), mask, value);

	return MV_OK;

}
/*******************************************************************************
* gppRegSet - Set a specific GPP pin on a specific GPP register
*
* DESCRIPTION:
*       This function set a specific GPP pin on a specific GPP register
*
* INPUT:
*		regOffs - GPP Register offset 
*       group - GPP group number
*       mask  - 32bit mask value. Each set bit in the mask means that the 
*               value of corresponding GPP will be set accordingly. Other GPP 
*               are not affected.
*       value - 32bit value that describes GPP value per pin.
*
* OUTPUT:
*       None.
*
* EXAMPLE:
*       Set GPP8 value of '0' and GPP15 value of '1'.
*       mvGppActiveSet(0, (GPP8 | GPP15), ((0 & GPP8) | (1 & GPP15)) );
*
* RETURN:
*       None.
*
*******************************************************************************/
static MV_VOID gppRegSet (MV_U32 group, MV_U32 regOffs,MV_U32 mask,MV_U32 value)
{
	MV_U32 gppData;

	gppData = MV_REG_READ(regOffs);

	gppData &= ~mask;

	gppData |= (value & mask);

	MV_REG_WRITE(regOffs, gppData);
}


