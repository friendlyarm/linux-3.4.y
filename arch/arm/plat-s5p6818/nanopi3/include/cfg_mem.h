/*------------------------------------------------------------------------------
 *
 *	Copyright (C) 2009 Nexell Co., Ltd All Rights Reserved
 *	Nexell Co. Proprietary & Confidential
 *
 *	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *  FOR A PARTICULAR PURPOSE.
 *
 *	Module     : System memory config
 *	Description:
 *	Author     : Platform Team
 *	Export     :
 *	History    :
 *	   2009/05/13 first implementation
 ------------------------------------------------------------------------------*/
#ifndef __CFG_MEM_H__
#define __CFG_MEM_H__

/*------------------------------------------------------------------------------
 * 	 System memory map
 */
#define	CFG_MEM_PHY_SYSTEM_BASE			0x40000000	/* System, must be at an evne 2MB boundary (head.S) */
#define	CFG_MEM_PHY_SYSTEM_SIZE			0x3F000000	/* Total 1G MB */

/*------------------------------------------------------------------------------
 *   DMA zone, if not defined DAM default size is 2M
 */
#define CFG_MEM_PHY_DMAZONE_SIZE        0x01000000  /* 16 MB DMA zone */

#endif /* __CFG_MEM_H__ */
