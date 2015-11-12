#ifndef _MT9D111_PRESET_H
#define _MT9D111_PRESET_H

#include "mt9d111-nx.h"

#if 0 
static struct reg_val mt9d111_reg_addr_chg[] = {
	//[3H_24MHz_80MHz_720P_30FPS_2013.03.15]
	{0, 0xF0, 0x0000}, 		// Page Register
	{0, 0x65, 0xA000},		// bypassed PLL (prepare for soft reset

	{0, 0xF0, 0x0001},		// Page Register
	{0, 0xC3, 0x0501},		// MCU_BOOT_MODE (MCU reset)

	{0, 0xF0, 0x0000},		// Page Register
	{0, 0x0D, 0x0021},		// RESET_REG (enable soft reset)
	{I2C_DELAY, 0, 250},                                                 
	{0, 0x0D, 0x0400},		// RESET_REG (disable soft reset)
	{I2C_DELAY, 0, 250},

	{I2C_TERM, 0, 0}
};

static struct reg_val mt9d111_reg_reset[] = {
	//[3H_24MHz_80MHz_720P_30FPS_2013.03.15]
	{0, 0xF0, 0x0000}, 		// Page Register
	{0, 0x65, 0xA000},		// bypassed PLL (prepare for soft reset

	{0, 0xF0, 0x0001},		// Page Register
	{0, 0xC3, 0x0501},		// MCU_BOOT_MODE (MCU reset)

	{0, 0xF0, 0x0000},		// Page Register
	{0, 0x0D, 0x0021},		// RESET_REG (enable soft reset)
	{I2C_DELAY, 0, 250},                                                 
	{0, 0x0D, 0x0000},		// RESET_REG (disable soft reset)
	{I2C_DELAY, 0, 250},

	{I2C_TERM, 0, 0}
};
#endif

static struct reg_val mt9d111_reg_init[] = {
	//[3H_24MHz_80MHz_720P_30FPS_2013.03.15]
#if 1
	{1, 0xF0, 0x0000}, 	// Page Register
	{1, 0x65, 0xA000}, 	// bypassed PLL (prepare for soft reset

	{1, 0xF0, 0x0001}, 	// Page Register
	{1, 0xC3, 0x0501}, 	// MCU_BOOT_MODE (MCU reset)              

	{1, 0xF0, 0x0000}, 	// Page Register
	{1, 0x0D, 0x0021}, 	// RESET_REG (enable soft reset)          
	{I2C_DELAY, 0, 250},//DELAY=1                                                 
	{1, 0x0D, 0x0000}, 	// RESET_REG (disable soft reset)         
	{I2C_DELAY, 0, 250},//DELAY=100                                                                                          
#endif
	/* 720p30 Timing Settings 24mhz */
	{0, 0xF0, 0x0000},		//Page 0 setting REG = 0
	{0, 0x05, 0x011E},		//HBLANK (B) = 286 0x011E
	{0, 0x06, 0x00C7},		//VBLANK (B) = 199 0x00C7
	{0, 0x07, 0x011E},		//HBLANK (A) = 286 0x011E
	{0, 0x08, 0x0077},		//VBLANK (A) = 119 0x0077                      
	{0, 0x20, 0x0303},		//Read Mode (B) = 768          ,[1:0] flip, rotate           
	{0, 0x21, 0x0000},		//Read Mode (A) = 0
	{0, 0x66, 0x1402},		//PLL Control 1 = 5122                    
	{0, 0x67, 0x0500},		//PLL Control 2 = 1280
	{0, 0x65, 0xA000},		//Clock CNTRL: PLL ON = 40960
	{I2C_DELAY, 0, 250},                                                      
	{0, 0x65, 0x2000},		//Clock CNTRL: USE PLL = 8192
	{I2C_DELAY, 0, 250},               
	{0, 0xF0, 0x0001},		// Page Register

	{1, 0xC6, 0xA122},		//Enter Preview: Auto Exposure
	{1, 0xC8, 0x0001},		//      = 1
	{1, 0xC6, 0xA123},		//Enter Preview: Flicker Detection
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA124},		//Enter Preview: Auto White Balance
	{1, 0xC8, 0x0001},		//      = 1
	{1, 0xC6, 0xA125},		//Enter Preview: Auto Focus
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA126},		//Enter Preview: Histogram
	{1, 0xC8, 0x0001},		//      = 1
	{1, 0xC6, 0xA127},		//Enter Preview: Strobe Control
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA128},		//Enter Preview: Skip Control
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA129},		//In Preview: Auto Exposure
	{1, 0xC8, 0x0003},		//      = 3
	{1, 0xC6, 0xA12A},		//In Preview: Flicker Detection
	{1, 0xC8, 0x0002},		//      = 2
	{1, 0xC6, 0xA12B},		//In Preview: Auto White Balance
	{1, 0xC8, 0x0003},		//      = 3
	{1, 0xC6, 0xA12C},		//In Preview: Auto Focus
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA12D},		//In Preview: Histogram
	{1, 0xC8, 0x0003},		//      = 3
	{1, 0xC6, 0xA12E},		//In Preview: Strobe Control
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA12F},		//In Preview: Skip Control
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA130},		//Exit Preview: Auto Exposure
	{1, 0xC8, 0x0001},		//      = 4                                     
	{1, 0xC6, 0xA131},		//Exit Preview: Flicker Detection
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA132},		//Exit Preview: Auto White Balance
	{1, 0xC8, 0x0001},		//      = 1
	{1, 0xC6, 0xA133},		//Exit Preview: Auto Focus
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA134},		//Exit Preview: Histogram
	{1, 0xC8, 0x0001},		//      = 1
	{1, 0xC6, 0xA135},		//Exit Preview: Strobe Control
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA136},		//Exit Preview: Skip Control
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA137},		//Capture: Auto Exposure
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA138},		//Capture: Flicker Detection
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA139},		//Capture: Auto White Balance
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA13A},		//Capture: Auto Focus
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA13B},		//Capture: Histogram
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA13C},		//Capture: Strobe Control
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0xA13D},		//Capture: Skip Control
	{1, 0xC8, 0x0000},		//      = 0

	{1, 0xC6, 0x2703},		//Output Width (A)
	{1, 0xC8, 0x0500},		//      = 1280
	{1, 0xC6, 0x2705},		//Output Height (A)
	{1, 0xC8, 0x02D0},		//      = 720

	{1, 0xC6, 0x2707},		//Output Width (B)
	{1, 0xC8, 0x0640}, 		//      = 1600                      
	{1, 0xC6, 0x2709},		//Output Height (B)
	{1, 0xC8, 0x04B0}, 		//      = 1200                      
	{1, 0xC6, 0x270B},		//mode_config
	{1, 0xC8, 0x0030},		//      = 48

	{1, 0xC6, 0x270F},		//Row Start (A)
	{1, 0xC8, 0x010C}, 		//      = 268                       
	{1, 0xC6, 0x2711},		//Column Start (A)
	{1, 0xC8, 0x00DC}, 		//      = 220                       
	{1, 0xC6, 0x2713},		//Row Height (A)
	{1, 0xC8, 0x02D0},		//      = 720
	{1, 0xC6, 0x2715},		//Column Width (A)
	{1, 0xC8, 0x0500},		//      = 1280


	{1, 0xC6, 0x2717}, 		//Extra Delay (A)                   
	{1, 0xC8, 0x009B}, 		//      = 155                        
	{1, 0xC6, 0x2719}, 		//Row Speed (A)                     
	{1, 0xC8, 0x0011}, 		//      = 17                        
	{1, 0xC6, 0x271B},		//Row Start (B)
	{1, 0xC8, 0x001C}, 		//      = 28                         
	{1, 0xC6, 0x271D},		//Column Start (B)
	{1, 0xC8, 0x003C}, 		//      = 60                         
	{1, 0xC6, 0x271F},		//Row Height (B)
	{1, 0xC8, 0x04B0}, 		//      = 1200                       
	{1, 0xC6, 0x2721},		//Column Width (B)
	{1, 0xC8, 0x0640}, 		//      = 1600                       
	{1, 0xC6, 0x2723}, 		//Extra Delay (B)                   
	{1, 0xC8, 0x0710}, 		//      = 1808                        
	{1, 0xC6, 0x2725}, 		//Row Speed (B)                     
	{1, 0xC8, 0x0011}, 		//      = 17                        
	{1, 0xC6, 0x2727},		//Crop_X0 (A)
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0x2729},		//Crop_X1 (A)
	{1, 0xC8, 0x0500},		//      = 1280
	{1, 0xC6, 0x272B},		//Crop_Y0 (A)
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0x272D},		//Crop_Y1 (A)
	{1, 0xC8, 0x02D0},		//      = 720
	{1, 0xC6, 0x2735},		//Crop_X0 (B)
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0x2737},		//Crop_X1 (B)
	{1, 0xC8, 0x0640}, 		//      = 1600                      
	{1, 0xC6, 0x2739},		//Crop_Y0 (B)
	{1, 0xC8, 0x0000},		//      = 0
	{1, 0xC6, 0x273B},		//Crop_Y1 (B)
	{1, 0xC8, 0x04B0}, 		//      = 1200                      
	{1, 0xC6, 0xA743}, 		//Gamma and Contrast Settings (A)   
	{1, 0xC8, 0x0002}, 		//      = 2                           
	{1, 0xC6, 0xA744}, 		//Gamma and Contrast Settings (B)   
	{1, 0xC8, 0x0002}, 		//      = 2                           

	// AE WINDOW
	{1, 0xC6, 0xA202}, 		// MCU_ADDRESS [AE_WINDOW_POS]
	{1, 0xC8, 0x0030}, 		// MCU_DATA_0
	{1, 0xC6, 0xA203}, 		// MCU_ADDRESS [AE_WINDOW_SIZE]
	{1, 0xC8, 0x00AF}, 		// MCU_DATA_0

	//Max 30fps
	{1, 0xC6, 0xA20E},		// MCU_ADDRESS [AE_MAX_INDEX]
	{1, 0xC8, 0x0004},		// MCU_DATA_0 Zone 4
	{1, 0xC6, 0xA217},		//IndexTH23
	{1, 0xC8, 0x0004},		// 	 = 4

	// Analog & Digital Gain
	{1, 0xC6, 0xA20F}, 		// MCU_ADDRESS [AE_MIN_VIRTGAIN]
	{1, 0xC8, 0x0020}, 		// MCU_DATA_0
	{1, 0xC6, 0xA210}, 		// MCU_ADDRESS [AE_MAX_VIRTGAIN]
	{1, 0xC8, 0x0080}, 		// MCU_DATA_0
	{1, 0xC6, 0x2214}, 		// MCU_ADDRESS [AE_MAX_DGAIN_AE1]
	{1, 0xC8, 0x0080}, 		// MCU_DATA_0
	{1, 0xC6, 0xA216}, 		// MCU_ADDRESS [AE_MAX_DGAIN_AE2]
	{1, 0xC8, 0x001C}, 		// MCU_DATA_0
	{1, 0xC6, 0xA218}, 		// MCU_ADDRESS [AE_MAXGAIN23]
	{1, 0xC8, 0x0070}, 		// MCU_DATA_0 

	//Slow down AE & Increase Gate
	{1, 0xC6, 0xA208},		// MCU_ADDRESS [AE_SKIP_FRAMES]
	{1, 0xC8, 0x0002},		// = 2                                 
	{1, 0xC6, 0xA107},		// MCU_ADDRESS [SEQ_AE_CONTBUFF]
	{1, 0xC8, 0x0004},		// MCU_DATA_0

	//slow down AWB
	{1, 0xC6, 0xA10B},		// MCU_ADDRESS [SEQ_AWB_CONTBUFF]
	{1, 0xC8, 0x0004},		// MCU_DATA_0

	// GAMMA 0.45 BLACK CORRECT 5 CONTRAST 1.25
	{1, 0xC6, 0xA743},		// MCU_ADDRESS [MODE_GAM_CONT_A]
	{1, 0xC8, 0x0003},		// MCU_DATA_0
	{1, 0xC6, 0xA745},		// MCU_ADDRESS [MODE_GAM_TABLE_A_0]
	{1, 0xC8, 0x0000},		// MCU_DATA_0
	{1, 0xC6, 0xA746},		// MCU_ADDRESS [MODE_GAM_TABLE_A_1]
	{1, 0xC8, 0x000A},	// MCU_DATA_0
	{1, 0xC6, 0xA747},		// MCU_ADDRESS [MODE_GAM_TABLE_A_2]
	{1, 0xC8, 0x001D},	// MCU_DATA_0
	{1, 0xC6, 0xA748},		// MCU_ADDRESS [MODE_GAM_TABLE_A_3]
	{1, 0xC8, 0x0037},	// MCU_DATA_0
	{1, 0xC6, 0xA749},		// MCU_ADDRESS [MODE_GAM_TABLE_A_4]
	{1, 0xC8, 0x0058},	// MCU_DATA_0
	{1, 0xC6, 0xA74A},		// MCU_ADDRESS [MODE_GAM_TABLE_A_5]
	{1, 0xC8, 0x0071},	// MCU_DATA_0
	{1, 0xC6, 0xA74B},		// MCU_ADDRESS [MODE_GAM_TABLE_A_6]
	{1, 0xC8, 0x0086},	// MCU_DATA_0
	{1, 0xC6, 0xA74C},		// MCU_ADDRESS [MODE_GAM_TABLE_A_7]
	{1, 0xC8, 0x0098},	// MCU_DATA_0
	{1, 0xC6, 0xA74D},		// MCU_ADDRESS [MODE_GAM_TABLE_A_8]
	{1, 0xC8, 0x00A7},	// MCU_DATA_0
	{1, 0xC6, 0xA74E},		// MCU_ADDRESS [MODE_GAM_TABLE_A_9]
	{1, 0xC8, 0x00B5},	// MCU_DATA_0
	{1, 0xC6, 0xA74F},		// MCU_ADDRESS [MODE_GAM_TABLE_A_10]
	{1, 0xC8, 0x00C0},	// MCU_DATA_0
	{1, 0xC6, 0xA750},		// MCU_ADDRESS [MODE_GAM_TABLE_A_11]
	{1, 0xC8, 0x00CB},	// MCU_DATA_0
	{1, 0xC6, 0xA751},		// MCU_ADDRESS [MODE_GAM_TABLE_A_12]
	{1, 0xC8, 0x00D4},	// MCU_DATA_0
	{1, 0xC6, 0xA752},		// MCU_ADDRESS [MODE_GAM_TABLE_A_13]
	{1, 0xC8, 0x00DD},	// MCU_DATA_0
	{1, 0xC6, 0xA753},		// MCU_ADDRESS [MODE_GAM_TABLE_A_14]
	{1, 0xC8, 0x00E4},		// MCU_DATA_0
	{1, 0xC6, 0xA754},		// MCU_ADDRESS [MODE_GAM_TABLE_A_15]
	{1, 0xC8, 0x00EC},	// MCU_DATA_0
	{1, 0xC6, 0xA755},		// MCU_ADDRESS [MODE_GAM_TABLE_A_16]
	{1, 0xC8, 0x00F3},	// MCU_DATA_0
	{1, 0xC6, 0xA756},		// MCU_ADDRESS [MODE_GAM_TABLE_A_17]
	{1, 0xC8, 0x00F9},		// MCU_DATA_0
	{1, 0xC6, 0xA757},		// MCU_ADDRESS [MODE_GAM_TABLE_A_18]
	{1, 0xC8, 0x00FF},		// MCU_DATA_0

	{1, 0xC6, 0xA103},		// MCU_ADDRESS [SEQ_CMD]
	{1, 0xC8, 0x0005},		// MCU_DATA_0

	{1, 0xC6, 0x276D}, 	// FIFO_Conf1 (A)              
	{1, 0xC8, 0xE0E1},	//      = 57569               
	{1, 0xC6, 0xA76F},	//FIFO_Conf2 (A)              
	{1, 0xC8, 0x00E1},	//      = 225                   
	{1, 0xC6, 0x2774},	//FIFO_Conf1 (B)              
	{1, 0xC8, 0xE0E1},	//      = 57569               
	{1, 0xC6, 0xA776},	//FIFO_Conf2 (B)              
	{1, 0xC8, 0x00E1},	//      = 225                   
	{1, 0xC6, 0x220B},	//Max R12 (B)(Shutter Delay)  
	{1, 0xC8, 0x0459},	//      = 1113                
	{1, 0xC6, 0xA217},	//IndexTH23                   
	{1, 0xC8, 0x0004},	//      = 8                     
	{1, 0xC6, 0x2228},	//RowTime (msclk per)/4       
	{1, 0xC8, 0x0187},	//      = 391                 

	{1, 0xC6, 0x222F},	//R9 Step                   
	{1, 0xC8, 0x00D3},	//      = 211               
	{1, 0xC6, 0xA408},	//search_f1_50              
	{1, 0xC8, 0x0031},	//      = 49                  
	{1, 0xC6, 0xA409},	//search_f2_50              
	{1, 0xC8, 0x0033},	//      = 51                  
	{1, 0xC6, 0xA40A},	//search_f1_60              
	{1, 0xC8, 0x0029},	//      = 41                  
	{1, 0xC6, 0xA40B},	//search_f2_60              
	{1, 0xC8, 0x002B},	//      = 43                  
	{1, 0xC6, 0x2411},	//R9_Step_60                
	{1, 0xC8, 0x00D3},	//      = 211               
	{1, 0xC6, 0x2413},	//R9_Step_50                
	{1, 0xC8, 0x00FE},	//      = 254               
	{I2C_DELAY, 0, 250},
	{1, 0xC6, 0xA103},	// Refresh Sequencer Mode
	{1, 0xC8, 0x0006},	
	{I2C_DELAY, 0, 250},
	{1, 0xC6, 0xA103},	// Refresh Sequencer
	{1, 0xC8, 0x0005},	

	//Lens Correction #1

	// AWB 12.09.25 
	{1, 0xC6, 0x2306},		// MCU_ADDRESS [AWB_CCM_L_0]
	{1, 0xC8, 0x0287},	// MCU_DATA_0
	{1, 0xC6, 0x2308},		// MCU_ADDRESS [AWB_CCM_L_1]
	{1, 0xC8, 0xFF47},	// MCU_DATA_0
	{1, 0xC6, 0x230A},		// MCU_ADDRESS [AWB_CCM_L_2]
	{1, 0xC8, 0xFF5D},	// MCU_DATA_0
	{1, 0xC6, 0x230C},		// MCU_ADDRESS [AWB_CCM_L_3]
	{1, 0xC8, 0xFF3B},	// MCU_DATA_0
	{1, 0xC6, 0x230E},		// MCU_ADDRESS [AWB_CCM_L_4]
	{1, 0xC8, 0x02B4},	// MCU_DATA_0
	{1, 0xC6, 0x2310},		// MCU_ADDRESS [AWB_CCM_L_5]
	{1, 0xC8, 0xFF57},	// MCU_DATA_0
	{1, 0xC6, 0x2312},		// MCU_ADDRESS [AWB_CCM_L_6]
	{1, 0xC8, 0xFF49},	// MCU_DATA_0
	{1, 0xC6, 0x2314},		// MCU_ADDRESS [AWB_CCM_L_7]
	{1, 0xC8, 0xFE25},	// MCU_DATA_0
	{1, 0xC6, 0x2316},		// MCU_ADDRESS [AWB_CCM_L_8]
	{1, 0xC8, 0x03DE},	// MCU_DATA_0
	{1, 0xC6, 0x2318},		// MCU_ADDRESS [AWB_CCM_L_9]
	{1, 0xC8, 0x001B},	// MCU_DATA_0
	{1, 0xC6, 0x231A},		// MCU_ADDRESS [AWB_CCM_L_10]
	{1, 0xC8, 0x003B},	// MCU_DATA_0
	{1, 0xC6, 0x231C},		// MCU_ADDRESS [AWB_CCM_RL_0]
	{1, 0xC8, 0x005E},	// MCU_DATA_0
	{1, 0xC6, 0x231E},		// MCU_ADDRESS [AWB_CCM_RL_1]
	{1, 0xC8, 0xFF23},	// MCU_DATA_0
	{1, 0xC6, 0x2320},		// MCU_ADDRESS [AWB_CCM_RL_2]
	{1, 0xC8, 0x008B},	// MCU_DATA_0
	{1, 0xC6, 0x2322},		// MCU_ADDRESS [AWB_CCM_RL_3]
	{1, 0xC8, 0x002F},	// MCU_DATA_0
	{1, 0xC6, 0x2324},		// MCU_ADDRESS [AWB_CCM_RL_4]
	{1, 0xC8, 0xFFAB},	// MCU_DATA_0
	{1, 0xC6, 0x2326},		// MCU_ADDRESS [AWB_CCM_RL_5]
	{1, 0xC8, 0x001D},	// MCU_DATA_0
	{1, 0xC6, 0x2328},		// MCU_ADDRESS [AWB_CCM_RL_6]
	{1, 0xC8, 0x006E},	// MCU_DATA_0
	{1, 0xC6, 0x232A},		// MCU_ADDRESS [AWB_CCM_RL_7]
	{1, 0xC8, 0x0117},	// MCU_DATA_0
	{1, 0xC6, 0x232C},		// MCU_ADDRESS [AWB_CCM_RL_8]
	{1, 0xC8, 0xFE5A},	// MCU_DATA_0
	{1, 0xC6, 0x232E},		// MCU_ADDRESS [AWB_CCM_RL_9]
	{1, 0xC8, 0x000E},	// MCU_DATA_0
	{1, 0xC6, 0x2330},		// MCU_ADDRESS [AWB_CCM_RL_10]
	{1, 0xC8, 0xFFEB},	// MCU_DATA_0

	// AWB Speed up
	{1, 0xC6, 0xA348},	// MCU_ADDRESS [AWB_GAIN_BUFFER_SPEED]
	{1, 0xC8, 0x0008},	// MCU_DATA_0
	{1, 0xC6, 0xA349},	// MCU_ADDRESS [AWB_JUMP_DIVISOR]
	{1, 0xC8, 0x0002},	// MCU_DATA_0

	// Tint Control
	{1, 0xC6, 0xA364},		// MCU_ADDRESS [AWB_KR_L]
	{1, 0xC8, 0x0080},	// MCU_DATA_0
	{1, 0xC6, 0xA365},		// MCU_ADDRESS [AWB_KG_L]
	{1, 0xC8, 0x0083},	// MCU_DATA_0
	{1, 0xC6, 0xA366},		// MCU_ADDRESS [AWB_KB_L]
	{1, 0xC8, 0x0085},	// MCU_DATA_0
	{1, 0xC6, 0xA367},	// MCU_ADDRESS [AWB_KR_R]
	{1, 0xC8, 0x0080},		// MCU_DATA_0
	{1, 0xC6, 0xA368},	// MCU_ADDRESS [AWB_KG_R]
	{1, 0xC8, 0x0083},	// MCU_DATA_0
	{1, 0xC6, 0xA369},	// MCU_ADDRESS [AWB_KB_R]
	{1, 0xC8, 0x0085},	// MCU_DATA_0

	// AWB Saturation
	{1, 0xC6, 0xA352}, 	// MCU_ADDRESS [AWB_SATURATION]
	{1, 0xC8, 0x005A}, 	// MCU_DATA_0


	// AE Speed up
	{1, 0xC6, 0xA209},	// MCU_ADDRESS	[AE_JUMP_DIVISOR]
	{1, 0xC8, 0x0001},	// MCU_DATA_0	
	{1, 0xC6, 0xA20A},	// MCU_ADDRESS	[AE_LUMA_BUFFER_SPEED]
	{1, 0xC8, 0x0018},	// MCU_DATA_0

	// AE TARGET
	{1, 0xC6, 0xA206}, 	// MCU_ADDRESS [AE_TARGET]
	{1, 0xC8, 0x0032}, 	// MCU_DATA_0

	// AE GATE
	{1, 0xC6, 0xA207}, 	// MCU_ADDRESS [AE_GATE]
	{1, 0xC8, 0x000A}, 	// MCU_DATA_0

	// AE WEIGHTS
	{1, 0xC6, 0xA219},    // AE WEIGHT
	{1, 0xC8, 0x008F},    // MCU_DATA_0

	// Sharpness, Aperture
	{0, 0x08, 0x05FC},	// COLOR_PIPELINE_CONTROL
	{0, 0x35, 0x1405},  // 1D_APERTURE_PARAMETERS
	{0, 0x36, 0x1405},	// APERTURE_PARAMETERS
	{0, 0x47, 0x0008}, 	// THRESH_EDGE_DETECT

	// Y_RGB_OFFSET
	{1, 0xBF, 0x0000}, 	// Y_RGB_OFFSET

	{1, 0xF0, 0x0002}, 	// Page Register
	{1, 0x0D, 0x0407}, 	// bypassed PLL (prepare for soft reset

	{I2C_TERM, 0, 0}
};

#if 0 
static struct reg_val mt9d111_reg_init_1[] = {
	//[3H_24MHz_80MHz_720P_30FPS_2014.02.25]
#if 1
	{1, 0xF0, 0x0000}, 	// Page Register
	{1, 0x65, 0xA000}, 	// bypassed PLL (prepare for soft reset

	{1, 0xF0, 0x0001}, 	// Page Register
	{1, 0xC3, 0x0501}, 	// MCU_BOOT_MODE (MCU reset)              

	{1, 0xF0, 0x0000}, 	// Page Register
	{1, 0x0D, 0x0021}, 	// RESET_REG (enable soft reset)          
	{I2C_DELAY, 0, 250},//DELAY=1                                                 
	{1, 0x0D, 0x0000}, 	// RESET_REG (disable soft reset)         
	{I2C_DELAY, 0, 250},//DELAY=100                                                                                          
#endif
	{1, 0x05, 0x011E}, 	//HBLANK (B) = 286                        
	{1, 0x06, 0x000B}, 	//VBLANK (B) = 11                         
	{1, 0x07, 0x011E}, 	//HBLANK (A) = 286                        
	{1, 0x08, 0x000B}, 	//VBLANK (A) = 11                         
	{1, 0x20, 0x0303}, 	//Read Mode (B) = 768                     
	{1, 0x21, 0x0000}, 	//Read Mode (A) = 0                       
	{1, 0x66, 0x2F07}, 	//PLL Control 1 = 12039                    
	{1, 0x67, 0x0500}, 	//PLL Control 2 = 1280                     
	{1, 0x65, 0xA000}, 	//Clock CNTRL: PLL ON = 40960             
	{I2C_DELAY, 0, 250},//DELAY=1                                                      
	{1, 0x65, 0x2000}, 	//Clock CNTRL: USE PLL = 8192             
	{I2C_DELAY, 0, 250},//DELAY=100               
	{1, 0xF0, 0x0001}, 	// Page Register
	{1, 0xC6, 0xA122}, 	//Enter Preview: Auto Exposure                
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA123}, 	//Enter Preview: Flicker Detection            
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA124}, 	//Enter Preview: Auto White Balance           
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA125}, 	//Enter Preview: Auto Focus                   
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA126}, 	//Enter Preview: Histogram                    
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA127}, 	//Enter Preview: Strobe Control               
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA128}, 	//Enter Preview: Skip Control                 
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA129}, 	//In Preview: Auto Exposure                   
	{1, 0xC8, 0x0003}, 	//      = 3                                     
	{1, 0xC6, 0xA12A}, 	//In Preview: Flicker Detection               
	{1, 0xC8, 0x0002}, 	//      = 2                                     
	{1, 0xC6, 0xA12B}, 	//In Preview: Auto White Balance              
	{1, 0xC8, 0x0003}, 	//      = 3                                     
	{1, 0xC6, 0xA12C}, 	//In Preview: Auto Focus                      
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA12D}, 	//In Preview: Histogram                       
	{1, 0xC8, 0x0003}, 	//      = 3                                     
	{1, 0xC6, 0xA12E}, 	//In Preview: Strobe Control                  
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA12F}, 	//In Preview: Skip Control                    
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA130}, 	//Exit Preview: Auto Exposure                 
	{1, 0xC8, 0x0001}, 	//      = 4                                     
	{1, 0xC6, 0xA131}, 	//Exit Preview: Flicker Detection             
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA132}, 	//Exit Preview: Auto White Balance            
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA133}, 	//Exit Preview: Auto Focus                    
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA134}, 	//Exit Preview: Histogram                     
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA135}, 	//Exit Preview: Strobe Control                
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA136}, 	//Exit Preview: Skip Control                  
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA137}, 	//Capture: Auto Exposure                      
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA138}, 	//Capture: Flicker Detection                  
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA139}, 	//Capture: Auto White Balance                 
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA13A}, 	//Capture: Auto Focus                         
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA13B}, 	//Capture: Histogram                          
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA13C}, 	//Capture: Strobe Control                     
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA13D}, 	//Capture: Skip Control                       
	{1, 0xC8, 0x0000}, 	//      = 0                                     

	{1, 0xC6, 0x2703}, 	//Output Width (A)                  
	{1, 0xC8, 0x0500},		//      = 1280
	{1, 0xC6, 0x2705}, 	//Output Height (A)                 
	{1, 0xC8, 0x02D0},		//      = 720
	{1, 0xC6, 0x2707}, 	//Output Width (B)                  
	{1, 0xC8, 0x0640}, 	//      = 1600                      
	{1, 0xC6, 0x2709}, 	//Output Height (B)                 
	{1, 0xC8, 0x04B0}, 	//      = 1200                      
	{1, 0xC6, 0x270B}, 	//mode_config                       
	{1, 0xC8, 0x0030}, 	//      = 48                        
	{1, 0xC6, 0x270F}, 	//Row Start (A)                     
	{1, 0xC8, 0x010C}, 		//      = 268                       
	{1, 0xC6, 0x2711}, 	//Column Start (A)                  
	{1, 0xC8, 0x00DC}, 		//      = 220                       
	{1, 0xC6, 0x2713}, 	//Row Height (A)                    
	{1, 0xC8, 0x02D0},		//      = 720
	{1, 0xC6, 0x2715}, 	//Column Width (A)                  
	{1, 0xC8, 0x0500},		//      = 1280
	{1, 0xC6, 0x2717}, 	//Extra Delay (A)                   
	{1, 0xC8, 0x0000}, 	//      = 0                        
	{1, 0xC6, 0x2719}, 	//Row Speed (A)                     
	{1, 0xC8, 0x0011}, 	//      = 17                        
	{1, 0xC6, 0x271B}, 	//Row Start (B)                     
	{1, 0xC8, 0x001C}, 	//      = 28                         
	{1, 0xC6, 0x271D}, 	//Column Start (B)                  
	{1, 0xC8, 0x003C}, 	//      = 60                         
	{1, 0xC6, 0x271F}, 	//Row Height (B)                    
	{1, 0xC8, 0x04B0}, 	//      = 1200                       
	{1, 0xC6, 0x2721}, 	//Column Width (B)                  
	{1, 0xC8, 0x0640}, 	//      = 1600                       
	{1, 0xC6, 0x2723}, 	//Extra Delay (B)                   
	{1, 0xC8, 0x00E0}, 	//      = 224                        
	{1, 0xC6, 0x2725}, 	//Row Speed (B)                     
	{1, 0xC8, 0x0011}, 	//      = 17                        
	{1, 0xC6, 0x2727}, 	//Crop_X0 (A)                       
	{1, 0xC8, 0x0000}, 	//      = 0                         
	{1, 0xC6, 0x2729}, 	//Crop_X1 (A)                       
	{1, 0xC8, 0x0500},		//      = 1280
	{1, 0xC6, 0x272B}, 	//Crop_Y0 (A)                       
	{1, 0xC8, 0x0000}, 	//      = 0                         
	{1, 0xC6, 0x272D}, 	//Crop_Y1 (A)                       
	{1, 0xC8, 0x02D0},		//      = 720
	{1, 0xC6, 0x2735}, 	//Crop_X0 (B)                       
	{1, 0xC8, 0x0000}, 	//      = 0                         
	{1, 0xC6, 0x2737}, 	//Crop_X1 (B)                       
	{1, 0xC8, 0x0640}, 	//      = 1600                      
	{1, 0xC6, 0x2739}, 	//Crop_Y0 (B)                       
	{1, 0xC8, 0x0000}, 	//      = 0                         
	{1, 0xC6, 0x273B}, 	//Crop_Y1 (B)                       
	{1, 0xC8, 0x04B0}, 	//      = 1200                      
//	{1, 0xC6, 0xA743}, 	//Gamma and Contrast Settings (A)   
//	{1, 0xC8, 0x0002}, 	//      = 2                           
//	{1, 0xC6, 0xA744}, 	//Gamma and Contrast Settings (B)   
//	{1, 0xC8, 0x0002}, 	//      = 2                           
 
// AE WINDOW
	{1, 0xC6, 0xA202}, 	// [AE_WINDOW_POS]
	{1, 0xC8, 0x0040}, 	// 40
	{1, 0xC6, 0xA203}, 	// [AE_WINDOW_SIZE]
	{1, 0xC8, 0x008F}, 	// 8F

//Max 30fps
	{1, 0xC6, 0xA20E}, 	// [AE_MAX_INDEX]      
	{1, 0xC8, 0x0004}, 	// 4               
	{1, 0xC6, 0xA217}, 	// IndexTH23                        
	{1, 0xC8, 0x0004}, 	// 4     

// Analog & Digital Gain
	{1, 0xC6, 0xA20F}, 	// MCU_ADDRESS [AE_MIN_VIRTGAIN]
	{1, 0xC8, 0x0020}, 	// MCU_DATA_0
	{1, 0xC6, 0xA210}, 	// MCU_ADDRESS [AE_MAX_VIRTGAIN]
	{1, 0xC8, 0x0080}, 	// MCU_DATA_0
	{1, 0xC6, 0x2214}, 	// MCU_ADDRESS [AE_MAX_DGAIN_AE1]
	{1, 0xC8, 0x0080}, 	// MCU_DATA_0
	{1, 0xC6, 0xA216}, 	// MCU_ADDRESS [AE_MAX_DGAIN_AE2]
	{1, 0xC8, 0x0020}, 	// MCU_DATA_0
	{1, 0xC6, 0xA218}, 	// MCU_ADDRESS [AE_MAXGAIN23]
	{1, 0xC8, 0x0070}, 	// MCU_DATA_0 

//Slow down AE & Increase Gate
	{1, 0xC6, 0xA208}, 	// MCU_ADDRESS [AE_SKIP_FRAMES]        
	{1, 0xC8, 0x0002},
	{1, 0xC6, 0xA107}, 	// MCU_ADDRESS [SEQ_AE_CONTBUFF]       
	{1, 0xC8, 0x0004}, 	// MCU_DATA_0                          

//slow down AWB
	{1, 0xC6, 0xA10B}, 	// MCU_ADDRESS [SEQ_AWB_CONTBUFF]        
	{1, 0xC8, 0x0004}, 	// MCU_DATA_0                            

// GAMMA 0.45 BLACK CORRECT 5 CONTRAST 1.25
	{1, 0xC6, 0xA743}, 	// MCU_ADDRESS [MODE_GAM_CONT_A]
	{1, 0xC8, 0x0003}, 	// MCU_DATA_0
	{1, 0xC6, 0xA744}, 	// MCU_ADDRESS [MODE_GAM_CONT_A]
	{1, 0xC8, 0x0003}, 	// MCU_DATA_0
	{1, 0xC6, 0xA745}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_0]
	{1, 0xC8, 0x0000}, 	// MCU_DATA_0
	{1, 0xC6, 0xA746}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_1]
	{1, 0xC8, 0x000A}, 	// MCU_DATA_0
	{1, 0xC6, 0xA747}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_2]
	{1, 0xC8, 0x001D}, 	// MCU_DATA_0
	{1, 0xC6, 0xA748}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_3]
	{1, 0xC8, 0x0037}, 	// MCU_DATA_0
	{1, 0xC6, 0xA749}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_4]
	{1, 0xC8, 0x0058}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74A}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_5]
	{1, 0xC8, 0x0071}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74B}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_6]
	{1, 0xC8, 0x0086}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74C}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_7]
	{1, 0xC8, 0x0098}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74D}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_8]
	{1, 0xC8, 0x00A7}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74E}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_9]
	{1, 0xC8, 0x00B5}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74F}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_10]
	{1, 0xC8, 0x00C0}, 	// MCU_DATA_0
	{1, 0xC6, 0xA750}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_11]
	{1, 0xC8, 0x00CB}, 	// MCU_DATA_0
	{1, 0xC6, 0xA751}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_12]
	{1, 0xC8, 0x00D4}, 	// MCU_DATA_0
	{1, 0xC6, 0xA752}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_13]
	{1, 0xC8, 0x00DD}, 	// MCU_DATA_0
	{1, 0xC6, 0xA753}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_14]
	{1, 0xC8, 0x00E4},	// MCU_DATA_0
	{1, 0xC6, 0xA754}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_15]
	{1, 0xC8, 0x00EC}, 	// MCU_DATA_0
	{1, 0xC6, 0xA755}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_16]
	{1, 0xC8, 0x00F3}, 	// MCU_DATA_0
	{1, 0xC6, 0xA756}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_17]
	{1, 0xC8, 0x00F9}, 	// MCU_DATA_0
	{1, 0xC6, 0xA757}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_18]
	{1, 0xC8, 0x00FF}, 	// MCU_DATA_0
	{I2C_DELAY, 0, 250},//DELAY=250
	{1, 0xC6, 0xA103}, 	// MCU_ADDRESS [SEQ_CMD]
	{1, 0xC8, 0x0005}, 	// MCU_DATA_0
	{I2C_DELAY, 0, 250},//DELAY=250

	{1, 0xC6, 0x276D}, 	// FIFO_Conf1 (A)              
	{1, 0xC8, 0xE0E1}, 	//      = 57569               
	{1, 0xC6, 0xA76F}, 	//FIFO_Conf2 (A)              
	{1, 0xC8, 0x00E1}, 	//      = 225                   
	{1, 0xC6, 0x2774}, 	//FIFO_Conf1 (B)              
	{1, 0xC8, 0xE0E1}, 	//      = 57569               
	{1, 0xC6, 0xA776}, 	//FIFO_Conf2 (B)              
	{1, 0xC8, 0x00E1}, 	//      = 225                   
	{1, 0xC6, 0x220B}, 	//Max R12 (B)(Shutter Delay)  
	{1, 0xC8, 0x045E}, 	//      = 1118                
	{1, 0xC6, 0xA217}, 	//IndexTH23                   
	{1, 0xC8, 0x0008}, 	//      = 8                     
	{1, 0xC6, 0x2228}, 	//RowTime (msclk per)/4       
	{1, 0xC8, 0x0188}, 	//      = 392                 

	{1, 0xC6, 0x222F}, 	//R9 Step                   
	{1, 0xC8, 0x0000}, 	//      = 0               
	{1, 0xC6, 0xA408}, 	//search_f1_50              
	{1, 0xC8, 0x00FF}, 	//      = 255                  
	{1, 0xC6, 0xA409}, 	//search_f2_50              
	{1, 0xC8, 0x0001}, 	//      = 1                  
	{1, 0xC6, 0xA40A}, 	//search_f1_60              
	{1, 0xC8, 0x00FF}, 	//      = 255                  
	{1, 0xC6, 0xA40B}, 	//search_f2_60              
	{1, 0xC8, 0x0001}, 	//      = 1                  
	{1, 0xC6, 0x2411}, 	//R9_Step_60                
	{1, 0xC8, 0x0000}, 	//      = 0               
	{1, 0xC6, 0x2413}, 	//R9_Step_50                
	{1, 0xC8, 0x0000}, 	//      = 0               
	{I2C_DELAY, 0, 250},//DELAY=250
	{1, 0xC6, 0xA103}, 	// Refresh Sequencer Mode
	{1, 0xC8, 0x0006}, 	
	{I2C_DELAY, 0, 250},//DELAY=250
	{1, 0xC6, 0xA103}, 	// Refresh Sequencer
	{1, 0xC8, 0x0005}, 	

//Lens Correction 13.03.28
	{1, 0xF0, 0x0002}, 	// PAGE REGISTER
	{1, 0x80, 0x0160}, 	// LENS_CORRECTION_CONTROL
	{1, 0x81, 0x6432}, 	// ZONE_BOUNDS_X1_X2
	{1, 0x82, 0x3296}, 	// ZONE_BOUNDS_X0_X3
	{1, 0x83, 0x9664}, 	// ZONE_BOUNDS_X4_X5
	{1, 0x84, 0x5028}, 	// ZONE_BOUNDS_Y1_Y2
	{1, 0x85, 0x2878}, 	// ZONE_BOUNDS_Y0_Y3
	{1, 0x86, 0x7850}, 	// ZONE_BOUNDS_Y4_Y5
	{1, 0x87, 0x0000}, 	// CENTER_OFFSET
	{1, 0x88, 0x0199}, 	// FX_RED
	{1, 0x8B, 0x005E}, 	// FY_RED
	{1, 0x8E, 0x0AC6}, 	// DF_DX_RED
	{1, 0x91, 0x0E41}, 	// DF_DY_RED
	{1, 0x94, 0x0961}, 	// SECOND_DERIV_ZONE_0_RED
	{1, 0x97, 0x183F}, 	// SECOND_DERIV_ZONE_1_RED
	{1, 0x9A, 0x1F23}, 	// SECOND_DERIV_ZONE_2_RED
	{1, 0x9D, 0x0446}, 	// SECOND_DERIV_ZONE_3_RED
	{1, 0xA0, 0x341B}, 	// SECOND_DERIV_ZONE_4_RED
	{1, 0xA3, 0x124D}, 	// SECOND_DERIV_ZONE_5_RED
	{1, 0xA6, 0x0900}, 	// SECOND_DERIV_ZONE_6_RED
	{1, 0xA9, 0xBE00}, 	// SECOND_DERIV_ZONE_7_RED
	{1, 0x89, 0x01B9}, 	// FX_GREEN
	{1, 0x8C, 0x004F}, 	// FY_GREEN
	{1, 0x8F, 0x0B55}, 	// DF_DX_GREEN
	{1, 0x92, 0x0E55}, 	// DF_DY_GREEN
	{1, 0x95, 0x1154}, 	// SECOND_DERIV_ZONE_0_GREEN
	{1, 0x98, 0x1234}, 	// SECOND_DERIV_ZONE_1_GREEN
	{1, 0x9B, 0x1D24}, 	// SECOND_DERIV_ZONE_2_GREEN
	{1, 0x9E, 0x0D41}, 	// SECOND_DERIV_ZONE_3_GREEN
	{1, 0xA1, 0xEA23}, 	// SECOND_DERIV_ZONE_4_GREEN
	{1, 0xA4, 0x143C}, 	// SECOND_DERIV_ZONE_5_GREEN
	{1, 0xA7, 0xB400}, 	// SECOND_DERIV_ZONE_6_GREEN
	{1, 0xAA, 0x0000}, 	// SECOND_DERIV_ZONE_7_GREEN
	{1, 0x8A, 0x024A}, 	// FX_BLUE
	{1, 0x8D, 0x004B}, 	// FY_BLUE
	{1, 0x90, 0x0AB3}, 	// DF_DX_BLUE
	{1, 0x93, 0x0E5A}, 	// DF_DY_BLUE
	{1, 0x96, 0x0F58}, 	// SECOND_DERIV_ZONE_0_BLUE
	{1, 0x99, 0x183E}, 	// SECOND_DERIV_ZONE_1_BLUE
	{1, 0x9C, 0x173B}, 	// SECOND_DERIV_ZONE_2_BLUE
	{1, 0x9F, 0x0C30}, 	// SECOND_DERIV_ZONE_3_BLUE
	{1, 0xA2, 0x0F2F}, 	// SECOND_DERIV_ZONE_4_BLUE
	{1, 0xA5, 0x9C46}, 	// SECOND_DERIV_ZONE_5_BLUE
	{1, 0xA8, 0x0850}, 	// SECOND_DERIV_ZONE_6_BLUE
	{1, 0xAB, 0x007F}, 	// SECOND_DERIV_ZONE_7_BLUE
	{1, 0xAC, 0x0000}, 	// X2_FACTORS
	{1, 0xAD, 0x0000}, 	// GLOBAL_OFFSET_FXY_FUNCTION
	{1, 0xAE, 0x0000}, 	// K_FACTOR_IN_K_FX_FY

	{1, 0xF0, 0x0001}, 	// PAGE REGISTER
	{1, 0x08, 0x05FC}, 	// COLOR_PIPELINE_CONTROL
 
// AWB Speed up
	{1, 0xC6, 0xA348}, 	// MCU_ADDRESS [AWB_GAIN_BUFFER_SPEED]
	{1, 0xC8, 0x0018}, 	// MCU_DATA_0
	{1, 0xC6, 0xA349}, 	// MCU_ADDRESS [AWB_JUMP_DIVISOR]
	{1, 0xC8, 0x0001}, 	// MCU_DATA_0
 
// AWB Saturation
	{1, 0xC6, 0xA352}, 	// MCU_ADDRESS [AWB_SATURATION]
	{1, 0xC8, 0x005A}, 	// MCU_DATA_0

// AE Speed up
	{1, 0xC6, 0xA209}, 	// MCU_ADDRESS	[AE_JUMP_DIVISOR]
	{1, 0xC8, 0x0001}, 	// MCU_DATA_0
	{1, 0xC6, 0xA20A}, 	// MCU_ADDRESS	[AE_LUMA_BUFFER_SPEED]
	{1, 0xC8, 0x0018}, 	// MCU_DATA_0

// AE TARGET
	{1, 0xC6, 0xA206}, 	// MCU_ADDRESS [AE_TARGET]
	{1, 0xC8, 0x003C}, 	// MCU_DATA_0

// AE GATE
	{1, 0xC6, 0xA207}, 	// MCU_ADDRESS [AE_GATE]
	{1, 0xC8, 0x000A}, 	// MCU_DATA_0

// Sharpness, Aperture
	{1, 0x08, 0x05FC}, 	// COLOR_PIPELINE_CONTROL
	{1, 0x35, 0x1206},
	{1, 0x36, 0x1408},	// APERTURE_PARAMETERS

	{1, 0xF0, 0x0002}, 	// Page Register
	{1, 0x0D, 0x0407}, 	// BT656

	{I2C_TERM, 0, 0}

};

static struct reg_val mt9d111_reg_init_1285x725[] = {
//[3H_24MHz_80MHz_1285x725_30FPS_2014.02.25]
	{1, 0xF0, 0x0000}, 	// Page Register
	{1, 0x65, 0xA000}, 	// bypassed PLL (prepare for soft reset

	{1, 0xF0, 0x0001}, 	// Page Register
	{1, 0xC3, 0x0501}, 	// MCU_BOOT_MODE (MCU reset)              

	{1, 0xF0, 0x0000}, 	// Page Register
	{1, 0x0D, 0x0021}, 	// RESET_REG (enable soft reset)          
	{I2C_DELAY, 0, 250},//DELAY=1                                                 
	{1, 0x0D, 0x0000}, 	// RESET_REG (disable soft reset)         
	{I2C_DELAY, 0, 250},//DELAY=100                                                                                          
	{1, 0x05, 0x011E}, 	//HBLANK (B) = 286                        
	{1, 0x06, 0x000B}, 	//VBLANK (B) = 11                         
	{1, 0x07, 0x011E}, 	//HBLANK (A) = 286                        
	{1, 0x08, 0x000B}, 	//VBLANK (A) = 11                         
	{1, 0x20, 0x0303}, 	//Read Mode (B) = 768                     
	{1, 0x21, 0x0000}, 	//Read Mode (A) = 0                       
	{1, 0x66, 0x2F07}, 	//PLL Control 1 = 12039                    
	{1, 0x67, 0x0500}, 	//PLL Control 2 = 1280                     
	{1, 0x65, 0xA000}, 	//Clock CNTRL: PLL ON = 40960             
	{I2C_DELAY, 0, 250},//DELAY=1                                                      
	{1, 0x65, 0x2000}, 	//Clock CNTRL: USE PLL = 8192             
	{I2C_DELAY, 0, 250},//DELAY=100               
	{1, 0xF0, 0x0001}, 	// Page Register
	{1, 0xC6, 0xA122}, 	//Enter Preview: Auto Exposure                
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA123}, 	//Enter Preview: Flicker Detection            
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA124}, 	//Enter Preview: Auto White Balance           
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA125}, 	//Enter Preview: Auto Focus                   
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA126}, 	//Enter Preview: Histogram                    
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA127}, 	//Enter Preview: Strobe Control               
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA128}, 	//Enter Preview: Skip Control                 
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA129}, 	//In Preview: Auto Exposure                   
	{1, 0xC8, 0x0003}, 	//      = 3                                     
	{1, 0xC6, 0xA12A}, 	//In Preview: Flicker Detection               
	{1, 0xC8, 0x0002}, 	//      = 2                                     
	{1, 0xC6, 0xA12B}, 	//In Preview: Auto White Balance              
	{1, 0xC8, 0x0003}, 	//      = 3                                     
	{1, 0xC6, 0xA12C}, 	//In Preview: Auto Focus                      
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA12D}, 	//In Preview: Histogram                       
	{1, 0xC8, 0x0003}, 	//      = 3                                     
	{1, 0xC6, 0xA12E}, 	//In Preview: Strobe Control                  
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA12F}, 	//In Preview: Skip Control                    
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA130}, 	//Exit Preview: Auto Exposure                 
	{1, 0xC8, 0x0001}, 	//      = 4                                     
	{1, 0xC6, 0xA131}, 	//Exit Preview: Flicker Detection             
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA132}, 	//Exit Preview: Auto White Balance            
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA133}, 	//Exit Preview: Auto Focus                    
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA134}, 	//Exit Preview: Histogram                     
	{1, 0xC8, 0x0001}, 	//      = 1                                     
	{1, 0xC6, 0xA135}, 	//Exit Preview: Strobe Control                
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA136}, 	//Exit Preview: Skip Control                  
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA137}, 	//Capture: Auto Exposure                      
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA138}, 	//Capture: Flicker Detection                  
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA139}, 	//Capture: Auto White Balance                 
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA13A}, 	//Capture: Auto Focus                         
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA13B}, 	//Capture: Histogram                          
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA13C}, 	//Capture: Strobe Control                     
	{1, 0xC8, 0x0000}, 	//      = 0                                     
	{1, 0xC6, 0xA13D}, 	//Capture: Skip Control                       
	{1, 0xC8, 0x0000}, 	//      = 0                                     

	{1, 0xC6, 0x2703}, 	//Output Width (A)                  
	{1, 0xC8, 0x0505}, 	//      = 1285                      
	{1, 0xC6, 0x2705}, 	//Output Height (A)                 
	{1, 0xC8, 0x02D5}, 	//      = 725                       
	{1, 0xC6, 0x2707}, 	//Output Width (B)                  
	{1, 0xC8, 0x0640}, 	//      = 1600                      
	{1, 0xC6, 0x2709}, 	//Output Height (B)                 
	{1, 0xC8, 0x04B0}, 	//      = 1200                      
	{1, 0xC6, 0x270B}, 	//mode_config                       
	{1, 0xC8, 0x0030}, 	//      = 48                        
	{1, 0xC6, 0x270F}, 	//Row Start (A)                     
	{1, 0xC8, 0x001C}, 	//      = 28                       
	{1, 0xC6, 0x2711}, 	//Column Start (A)                  
	{1, 0xC8, 0x003C}, 	//      = 60                       
	{1, 0xC6, 0x2713}, 	//Row Height (A)                    
	{1, 0xC8, 0x02D5}, 	//      = 725                        
	{1, 0xC6, 0x2715}, 	//Column Width (A)                  
	{1, 0xC8, 0x0505}, 	//      = 1285                      
	{1, 0xC6, 0x2717}, 	//Extra Delay (A)                   
	{1, 0xC8, 0x0000}, 	//      = 0                        
	{1, 0xC6, 0x2719}, 	//Row Speed (A)                     
	{1, 0xC8, 0x0011}, 	//      = 17                        
	{1, 0xC6, 0x271B}, 	//Row Start (B)                     
	{1, 0xC8, 0x001C}, 	//      = 28                         
	{1, 0xC6, 0x271D}, 	//Column Start (B)                  
	{1, 0xC8, 0x003C}, 	//      = 60                         
	{1, 0xC6, 0x271F}, 	//Row Height (B)                    
	{1, 0xC8, 0x04B0}, 	//      = 1200                       
	{1, 0xC6, 0x2721}, 	//Column Width (B)                  
	{1, 0xC8, 0x0640}, 	//      = 1600                       
	{1, 0xC6, 0x2723}, 	//Extra Delay (B)                   
	{1, 0xC8, 0x00E0}, 	//      = 224                        
	{1, 0xC6, 0x2725}, 	//Row Speed (B)                     
	{1, 0xC8, 0x0011}, 	//      = 17                        
	{1, 0xC6, 0x2727}, 	//Crop_X0 (A)                       
	{1, 0xC8, 0x0000}, 	//      = 0                         
	{1, 0xC6, 0x2729}, 	//Crop_X1 (A)                       
	{1, 0xC8, 0x0505}, 	//      = 1285                      
	{1, 0xC6, 0x272B}, 	//Crop_Y0 (A)                       
	{1, 0xC8, 0x0000}, 	//      = 0                         
	{1, 0xC6, 0x272D}, 	//Crop_Y1 (A)                       
	{1, 0xC8, 0x02D5}, 	//      = 725                       
	{1, 0xC6, 0x2735}, 	//Crop_X0 (B)                       
	{1, 0xC8, 0x0000}, 	//      = 0                         
	{1, 0xC6, 0x2737}, 	//Crop_X1 (B)                       
	{1, 0xC8, 0x0640}, 	//      = 1600                      
	{1, 0xC6, 0x2739}, 	//Crop_Y0 (B)                       
	{1, 0xC8, 0x0000}, 	//      = 0                         
	{1, 0xC6, 0x273B}, 	//Crop_Y1 (B)                       
	{1, 0xC8, 0x04B0}, 	//      = 1200                      
//	{1, 0xC6, 0xA743}, 	//Gamma and Contrast Settings (A)   
//	{1, 0xC8, 0x0002}, 	//      = 2                           
//	{1, 0xC6, 0xA744}, 	//Gamma and Contrast Settings (B)   
//	{1, 0xC8, 0x0002}, 	//      = 2                           
 
// AE WINDOW
	{1, 0xC6, 0xA202}, 	// [AE_WINDOW_POS]
	{1, 0xC8, 0x0040}, 	// 40
	{1, 0xC6, 0xA203}, 	// [AE_WINDOW_SIZE]
	{1, 0xC8, 0x008F}, 	// 8F

//Max 30fps
	{1, 0xC6, 0xA20E}, 	// [AE_MAX_INDEX]      
	{1, 0xC8, 0x0004}, 	// 4               
	{1, 0xC6, 0xA217}, 	// IndexTH23                        
	{1, 0xC8, 0x0004}, 	// 4     

// Analog & Digital Gain
	{1, 0xC6, 0xA20F}, 	// MCU_ADDRESS [AE_MIN_VIRTGAIN]
	{1, 0xC8, 0x0020}, 	// MCU_DATA_0
	{1, 0xC6, 0xA210}, 	// MCU_ADDRESS [AE_MAX_VIRTGAIN]
	{1, 0xC8, 0x0080}, 	// MCU_DATA_0
	{1, 0xC6, 0x2214}, 	// MCU_ADDRESS [AE_MAX_DGAIN_AE1]
	{1, 0xC8, 0x0080}, 	// MCU_DATA_0
	{1, 0xC6, 0xA216}, 	// MCU_ADDRESS [AE_MAX_DGAIN_AE2]
	{1, 0xC8, 0x0020}, 	// MCU_DATA_0
	{1, 0xC6, 0xA218}, 	// MCU_ADDRESS [AE_MAXGAIN23]
	{1, 0xC8, 0x0070}, 	// MCU_DATA_0 

//Slow down AE & Increase Gate
	{1, 0xC6, 0xA208}, 	// MCU_ADDRESS [AE_SKIP_FRAMES]        
	{1, 0xC8, 0x0002},
	{1, 0xC6, 0xA107}, 	// MCU_ADDRESS [SEQ_AE_CONTBUFF]       
	{1, 0xC8, 0x0004}, 	// MCU_DATA_0                          

//slow down AWB
	{1, 0xC6, 0xA10B}, 	// MCU_ADDRESS [SEQ_AWB_CONTBUFF]        
	{1, 0xC8, 0x0004}, 	// MCU_DATA_0                            

// GAMMA 0.45 BLACK CORRECT 5 CONTRAST 1.25
	{1, 0xC6, 0xA743}, 	// MCU_ADDRESS [MODE_GAM_CONT_A]
	{1, 0xC8, 0x0003}, 	// MCU_DATA_0
	{1, 0xC6, 0xA744}, 	// MCU_ADDRESS [MODE_GAM_CONT_A]
	{1, 0xC8, 0x0003}, 	// MCU_DATA_0
	{1, 0xC6, 0xA745}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_0]
	{1, 0xC8, 0x0000}, 	// MCU_DATA_0
	{1, 0xC6, 0xA746}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_1]
	{1, 0xC8, 0x000A}, 	// MCU_DATA_0
	{1, 0xC6, 0xA747}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_2]
	{1, 0xC8, 0x001D}, 	// MCU_DATA_0
	{1, 0xC6, 0xA748}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_3]
	{1, 0xC8, 0x0037}, 	// MCU_DATA_0
	{1, 0xC6, 0xA749}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_4]
	{1, 0xC8, 0x0058}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74A}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_5]
	{1, 0xC8, 0x0071}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74B}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_6]
	{1, 0xC8, 0x0086}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74C}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_7]
	{1, 0xC8, 0x0098}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74D}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_8]
	{1, 0xC8, 0x00A7}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74E}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_9]
	{1, 0xC8, 0x00B5}, 	// MCU_DATA_0
	{1, 0xC6, 0xA74F}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_10]
	{1, 0xC8, 0x00C0}, 	// MCU_DATA_0
	{1, 0xC6, 0xA750}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_11]
	{1, 0xC8, 0x00CB}, 	// MCU_DATA_0
	{1, 0xC6, 0xA751}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_12]
	{1, 0xC8, 0x00D4}, 	// MCU_DATA_0
	{1, 0xC6, 0xA752}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_13]
	{1, 0xC8, 0x00DD}, 	// MCU_DATA_0
	{1, 0xC6, 0xA753}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_14]
	{1, 0xC8, 0x00E4},	// MCU_DATA_0
	{1, 0xC6, 0xA754}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_15]
	{1, 0xC8, 0x00EC}, 	// MCU_DATA_0
	{1, 0xC6, 0xA755}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_16]
	{1, 0xC8, 0x00F3}, 	// MCU_DATA_0
	{1, 0xC6, 0xA756}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_17]
	{1, 0xC8, 0x00F9}, 	// MCU_DATA_0
	{1, 0xC6, 0xA757}, 	// MCU_ADDRESS [MODE_GAM_TABLE_A_18]
	{1, 0xC8, 0x00FF}, 	// MCU_DATA_0
	{I2C_DELAY, 0, 250},//DELAY=250
	{1, 0xC6, 0xA103}, 	// MCU_ADDRESS [SEQ_CMD]
	{1, 0xC8, 0x0005}, 	// MCU_DATA_0
	{I2C_DELAY, 0, 250},//DELAY=250

	{1, 0xC6, 0x276D}, 	// FIFO_Conf1 (A)              
	{1, 0xC8, 0xE0E1}, 	//      = 57569               
	{1, 0xC6, 0xA76F}, 	//FIFO_Conf2 (A)              
	{1, 0xC8, 0x00E1}, 	//      = 225                   
	{1, 0xC6, 0x2774}, 	//FIFO_Conf1 (B)              
	{1, 0xC8, 0xE0E1}, 	//      = 57569               
	{1, 0xC6, 0xA776}, 	//FIFO_Conf2 (B)              
	{1, 0xC8, 0x00E1}, 	//      = 225                   
	{1, 0xC6, 0x220B}, 	//Max R12 (B)(Shutter Delay)  
	{1, 0xC8, 0x045E}, 	//      = 1118                
	{1, 0xC6, 0xA217}, 	//IndexTH23                   
	{1, 0xC8, 0x0008}, 	//      = 8                     
	{1, 0xC6, 0x2228}, 	//RowTime (msclk per)/4       
	{1, 0xC8, 0x0188}, 	//      = 392                 

	{1, 0xC6, 0x222F}, 	//R9 Step                   
	{1, 0xC8, 0x0000}, 	//      = 0               
	{1, 0xC6, 0xA408}, 	//search_f1_50              
	{1, 0xC8, 0x00FF}, 	//      = 255                  
	{1, 0xC6, 0xA409}, 	//search_f2_50              
	{1, 0xC8, 0x0001}, 	//      = 1                  
	{1, 0xC6, 0xA40A}, 	//search_f1_60              
	{1, 0xC8, 0x00FF}, 	//      = 255                  
	{1, 0xC6, 0xA40B}, 	//search_f2_60              
	{1, 0xC8, 0x0001}, 	//      = 1                  
	{1, 0xC6, 0x2411}, 	//R9_Step_60                
	{1, 0xC8, 0x0000}, 	//      = 0               
	{1, 0xC6, 0x2413}, 	//R9_Step_50                
	{1, 0xC8, 0x0000}, 	//      = 0               
	{I2C_DELAY, 0, 250},//DELAY=250
	{1, 0xC6, 0xA103}, 	// Refresh Sequencer Mode
	{1, 0xC8, 0x0006}, 	
	{I2C_DELAY, 0, 250},//DELAY=250
	{1, 0xC6, 0xA103}, 	// Refresh Sequencer
	{1, 0xC8, 0x0005}, 	

//Lens Correction 13.03.28
	{1, 0xF0, 0x0002}, 	// PAGE REGISTER
	{1, 0x80, 0x0160}, 	// LENS_CORRECTION_CONTROL
	{1, 0x81, 0x6432}, 	// ZONE_BOUNDS_X1_X2
	{1, 0x82, 0x3296}, 	// ZONE_BOUNDS_X0_X3
	{1, 0x83, 0x9664}, 	// ZONE_BOUNDS_X4_X5
	{1, 0x84, 0x5028}, 	// ZONE_BOUNDS_Y1_Y2
	{1, 0x85, 0x2878}, 	// ZONE_BOUNDS_Y0_Y3
	{1, 0x86, 0x7850}, 	// ZONE_BOUNDS_Y4_Y5
	{1, 0x87, 0x0000}, 	// CENTER_OFFSET
	{1, 0x88, 0x0199}, 	// FX_RED
	{1, 0x8B, 0x005E}, 	// FY_RED
	{1, 0x8E, 0x0AC6}, 	// DF_DX_RED
	{1, 0x91, 0x0E41}, 	// DF_DY_RED
	{1, 0x94, 0x0961}, 	// SECOND_DERIV_ZONE_0_RED
	{1, 0x97, 0x183F}, 	// SECOND_DERIV_ZONE_1_RED
	{1, 0x9A, 0x1F23}, 	// SECOND_DERIV_ZONE_2_RED
	{1, 0x9D, 0x0446}, 	// SECOND_DERIV_ZONE_3_RED
	{1, 0xA0, 0x341B}, 	// SECOND_DERIV_ZONE_4_RED
	{1, 0xA3, 0x124D}, 	// SECOND_DERIV_ZONE_5_RED
	{1, 0xA6, 0x0900}, 	// SECOND_DERIV_ZONE_6_RED
	{1, 0xA9, 0xBE00}, 	// SECOND_DERIV_ZONE_7_RED
	{1, 0x89, 0x01B9}, 	// FX_GREEN
	{1, 0x8C, 0x004F}, 	// FY_GREEN
	{1, 0x8F, 0x0B55}, 	// DF_DX_GREEN
	{1, 0x92, 0x0E55}, 	// DF_DY_GREEN
	{1, 0x95, 0x1154}, 	// SECOND_DERIV_ZONE_0_GREEN
	{1, 0x98, 0x1234}, 	// SECOND_DERIV_ZONE_1_GREEN
	{1, 0x9B, 0x1D24}, 	// SECOND_DERIV_ZONE_2_GREEN
	{1, 0x9E, 0x0D41}, 	// SECOND_DERIV_ZONE_3_GREEN
	{1, 0xA1, 0xEA23}, 	// SECOND_DERIV_ZONE_4_GREEN
	{1, 0xA4, 0x143C}, 	// SECOND_DERIV_ZONE_5_GREEN
	{1, 0xA7, 0xB400}, 	// SECOND_DERIV_ZONE_6_GREEN
	{1, 0xAA, 0x0000}, 	// SECOND_DERIV_ZONE_7_GREEN
	{1, 0x8A, 0x024A}, 	// FX_BLUE
	{1, 0x8D, 0x004B}, 	// FY_BLUE
	{1, 0x90, 0x0AB3}, 	// DF_DX_BLUE
	{1, 0x93, 0x0E5A}, 	// DF_DY_BLUE
	{1, 0x96, 0x0F58}, 	// SECOND_DERIV_ZONE_0_BLUE
	{1, 0x99, 0x183E}, 	// SECOND_DERIV_ZONE_1_BLUE
	{1, 0x9C, 0x173B}, 	// SECOND_DERIV_ZONE_2_BLUE
	{1, 0x9F, 0x0C30}, 	// SECOND_DERIV_ZONE_3_BLUE
	{1, 0xA2, 0x0F2F}, 	// SECOND_DERIV_ZONE_4_BLUE
	{1, 0xA5, 0x9C46}, 	// SECOND_DERIV_ZONE_5_BLUE
	{1, 0xA8, 0x0850}, 	// SECOND_DERIV_ZONE_6_BLUE
	{1, 0xAB, 0x007F}, 	// SECOND_DERIV_ZONE_7_BLUE
	{1, 0xAC, 0x0000}, 	// X2_FACTORS
	{1, 0xAD, 0x0000}, 	// GLOBAL_OFFSET_FXY_FUNCTION
	{1, 0xAE, 0x0000}, 	// K_FACTOR_IN_K_FX_FY

	{1, 0xF0, 0x0001}, 	// PAGE REGISTER
	{1, 0x08, 0x05FC}, 	// COLOR_PIPELINE_CONTROL
 
// AWB Speed up
	{1, 0xC6, 0xA348}, 	// MCU_ADDRESS [AWB_GAIN_BUFFER_SPEED]
	{1, 0xC8, 0x0018}, 	// MCU_DATA_0
	{1, 0xC6, 0xA349}, 	// MCU_ADDRESS [AWB_JUMP_DIVISOR]
	{1, 0xC8, 0x0001}, 	// MCU_DATA_0
 
// AWB Saturation
	{1, 0xC6, 0xA352}, 	// MCU_ADDRESS [AWB_SATURATION]
	{1, 0xC8, 0x005A}, 	// MCU_DATA_0

// AE Speed up
	{1, 0xC6, 0xA209}, 	// MCU_ADDRESS	[AE_JUMP_DIVISOR]
	{1, 0xC8, 0x0001}, 	// MCU_DATA_0
	{1, 0xC6, 0xA20A}, 	// MCU_ADDRESS	[AE_LUMA_BUFFER_SPEED]
	{1, 0xC8, 0x0018}, 	// MCU_DATA_0

// AE TARGET
	{1, 0xC6, 0xA206}, 	// MCU_ADDRESS [AE_TARGET]
	{1, 0xC8, 0x003C}, 	// MCU_DATA_0

// AE GATE
	{1, 0xC6, 0xA207}, 	// MCU_ADDRESS [AE_GATE]
	{1, 0xC8, 0x000A}, 	// MCU_DATA_0

// Sharpness, Aperture
	{1, 0x08, 0x05FC}, 	// COLOR_PIPELINE_CONTROL
	{1, 0x35, 0x1206},
	{1, 0x36, 0x1408},	// APERTURE_PARAMETERS

	{1, 0xF0, 0x0002}, 	// Page Register
	{1, 0x0D, 0x0407}, 	// bypassed PLL (prepare for soft reset

	{I2C_TERM, 0, 0}

};
#endif

#endif
