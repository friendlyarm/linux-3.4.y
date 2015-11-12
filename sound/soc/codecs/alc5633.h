#ifndef __ALC5633_H__
#define __ALC5633_H__


#define ALC5633_RESET						0X00			//RESET CODEC TO DEFAULT
#define ALC5633_SPK_OUT_VOL					0X02			//SPEAKER OUT VOLUME
#define ALC5633_SPK_HP_MIXER_CTRL			0X03			//SPEAKER/HP OUTPUT MIXER CONTROL
#define ALC5633_HP_OUT_VOL					0X04			//HEADPHONE OUTPUT VOLUME
#define ALC5633_AUXOUT_VOL					0X06			//AUXOUT OUTPUT VOLUME
#define ALC5633_LINE_IN_1_VOL				0X08			//LINE IN 1 VOLUME
#define ALC5633_LINE_IN_2_VOL				0X0A			//LINE IN 2 VOLUME
#define ALC5633_DAC_CTRL						0X0C			//DAC CONTROL
#define ALC5633_DAC_DIG_VOL					0X0E			//DAC DIGITAL VOLUME CONTROL
#define ALC5633_MIC_CTRL_1					0X10			//MICPHONE INPUT CONTROL
#define ALC5633_ADC_CTRL						0X12			//ADC CONTROL
#define ALC5633_REC_MIXER_CTRL				0X14			//RECORD MIXER CONTROL
#define ALC5633_ADC_DIG_VOL					0X16			//ADC DIGITAL VOL CONTROL
#define ALC5633_HPMIXER_CTRL					0X18			//HP MIXER CONTROL
#define ALC5633_AUXMIXER_CTRL				0X1A			//AUX MIXER CONTROL
#define ALC5633_SPKMIXER_CTRL				0X1C			//SPEAKER MIXER CONTROL
#define ALC5633_SPK_AMP_CTRL					0X1E			//SPEAKER AMPLIFIER CONTROL
#define ALC5633_MIC_CTRL_2					0X22			//MICROPHONE CONTROL 2
#define	ALC5633_SDP_CTRL						0X34			//STEREO I2S SERIAL DATA PORT CONTROL
#define ALC5633_STEREO_AD_DA_CLK_CTRL		0X38			//STEREO AD/DA CLOCK CONTROL
#define	ALC5633_PWR_MANAG_ADD1				0X3A			//POWER MANAGMENT ADDITION 1
#define	ALC5633_PWR_MANAG_ADD2				0X3B			//POWER MANAGMENT ADDITION 2
#define ALC5633_PWR_MANAG_ADD3				0X3C			//POWER MANAGMENT ADDITION 3
#define ALC5633_PWR_MANAG_ADD4				0X3E			//POWER MANAGMENT ADDITION 4
#define ALC5633_GEN_PUR_CTRL_1				0X40			//GENERAL PURPOSE CONTROL REGISTER
#define	ALC5633_GBL_CLK_CTRL					0X42			//GLOBAL CLOCK CONTROL
#define ALC5633_PLL_CTRL						0X44			//PLL CONTROL
#define ALC5633_DIG_BEEP_IRQ_CTRL			0X48			//DIGITAL BEEP GEN AND IRQ CONTROL
#define ALC5633_INT_ST_STICKY_CTRL			0X4A			//INTERNAL STATUS AND STICKY CONTROL
#define ALC5633_GPIO_CTRL_1					0X4C			//GPIO CONTROL 1
#define ALC5633_GPIO_CTRL_2					0X4D			//GPIO CONTROL 2
#define ALC5633_GEN_PUR_CTRL_2				0X52			//GENERAL PURPOSE CONTROL REGISTER
#define ALC5633_DEPOP_CTRL_1					0X54			//DEPOP MODE CONTROL 1
#define ALC5633_DEPOP_CTRL_2					0X56			//DEPOP MODE CONTROL 2
#define ALC5633_JACK_DET_CTRL				0X5A			//JACK DETECT CONTROL REGISTER
#define ALC5633_ZC_SM_CTRL_1					0X5C			//ZERO CROSS AND SOFT VOLUME CONTROL 1
#define ALC5633_ZC_SM_CTRL_2					0X5D			//ZERO CROSS AND SOFT VOLUME CONTROL 2
#define ALC5633_ALC_CTRL_1					0X64			//ALC CONTROL 1
#define ALC5633_ALC_CTRL_2					0X65			//ALC CONTROL 2
#define ALC5633_ALC_CTRL_3					0X66			//ALC CONTROL 3
#define ALC5633_PSEUDO_SPATL_CTRL			0X68			//PSEUDO STEREO AND SPATIAL EFFECT CONTROL
#define ALC5633_PRI_REG_ADD					0X6A			//INDEX ADDRESS
#define ALC5633_PRI_REG_DATA					0X6C			//INDEX DATA
#define ALC5633_EQ_CTRL_1					0X6E			//EQ CONTROL 1
#define ALC5633_EQ_CTRL_2					0X70			//EQ CONTROL 2
#define ALC5633_VERSION		  		    	0x7A			//VERSION ID
#define ALC5633_VENDOR_ID1	  		    	0x7C			//VENDOR ID1
#define ALC5633_VENDOR_ID2	  		    	0x7E			//VENDOR ID2


//global definition
#define RT_L_MUTE						(0x1<<15)		//MUTE LEFT CONTROL BIT
#define RT_R_MUTE						(0x1<<7)		//MUTE RIGHT CONTROL BIT
#define RT_SPKL_VOL_MASK				(0x1f<<5)		//SPEAKER LEFT VOLUME MASK
#define RT_SPKR_VOL_MASK				(0x1f)			//SPEAKER RIGHT VOLUME MASK

//Speaker Output Control(0x02)
#define SPK_RN_PIN_SEL_MASK			(0x3<<13)		//SPO_RN PIN SELECT MASK
#define SPK_RN_PIN_SEL_RN			(0x0<<13)		//SPO_RN PIN SELECT RN
#define SPK_RN_PIN_SEL_RP			(0x1<<13)		//SPO_RN PIN SELECT RP
#define SPK_RN_PIN_SEL_LN			(0x2<<13)		//SPO_RN PIN SELECT LN
#define SPK_RN_PIN_SEL_VMID			(0x3<<13)		//SPO_RN PIN SELECT VMID

//Speaker/HP Output Mixer Control(0x03)
#define UM_DACL_TO_SPOL_MIXER		(0x1<<8)		//UNMUTE DAC LEFT TO SPOL_MIXER
#define UM_DACR_TO_SPOR_MIXER		(0x1<<6)		//UNMUTE DAC RIGHT TO SPOR_MIXER 
#define UM_DAC_TO_HPO_MIXER			(0x1<<5)		//UNMUTE DAC TO HPOUT MIXER
#define UM_HPVOL_TO_HPO_MIXER		(0x1<<4)		//UNMUTE HP VOL TO HPOUT MIXER

//Headphone Output Control(0x04)
#define HP_L_VOL_SEL_MASK			(0x1<<14)		//HP left channel volume input select MASK
#define HP_L_VOL_SEL_VMID			(0x0<<14)		//HP left channel volume input select VMID
#define HP_L_VOL_SEL_HPMIX_L		(0x1<<14)		//HP left channel volume input select HPMIXER LEFT
#define HP_R_VOL_SEL_MASK			(0x1<< 6)		//HP right channel volume input select MASK
#define HP_R_VOL_SEL_VMID			(0x0<< 6)		//HP right channel volume input select VMID
#define HP_R_VOL_SEL_HPMIX_R		(0x1<< 6)		//HP right channel volume input select HPMIXER RIGHT 


//Output Control for AUXOUT(0x06)
#define AUXOUT_MODE_SEL_MASK			(0x1<<14)		//AUXOUT OUTPUT MODE MASK
#define AUXOUT_MODE_SEL_MONO			(0x0<<14)		//AUXOUT OUTPUT MODE SELECT MONO
#define AUXOUT_MODE_SEL_STEREO			(0x1<<14)		//AUXOUT OUTPUT MODE SELECT STEREO


//Microphone Input Control(0x10)
#define MIC_1_MODE_SEL_MASK				(0x1<<15)		//MIC 1 INPUT MODE MASK
#define MIC_1_MODE_SEL_SE				(0x0<<15)		//MIC 1 INPUT MODE SEL SINGLE END
#define MIC_1_MODE_SEL_DIFF				(0x1<<15)		//MIC 1 INPUT MODE SEL DIFF

#define MIC_2_MODE_SEL_MASK				(0x1<<7)		//MIC 2 INPUT MODE MASK
#define MIC_2_MODE_SEL_SE				(0x0<<7)		//MIC 2 INPUT MODE SEL SINGLE END
#define MIC_2_MODE_SEL_DIFF				(0x1<<7)		//MIC 2 INPUT MODE SEL DIFF


//ADC Recording Mixer Control(0x14)
#define M_HPMIXER_L_TO_RECMIXER_L		(0x1<<14)		//Mute left HPMIXER to left RECMIXER
#define M_AUXMIXER_L_TO_RECMIXER_L		(0x1<<13)		//Mute left AUXMIXER to left RECMIXER
#define M_SPKMIXER_TO_RECMIXER_L		(0x1<<12)		//Mute SPKMIXER to left RECMIXER
#define M_LINEIN1_TO_RECMIXER_L			(0x1<<11)		//Mute LINEIN 1 to left RECMIXER
#define M_LINEIN2_TO_RECMIXER_L			(0x1<<10)		//Mute LINEIN 2 to left RECMIXER
#define M_MIC1_TO_RECMIXER_L			(0x1<<11)		//Mute MIC 1 to left RECMIXER
#define M_MIC2_TO_RECMIXER_L			(0x1<<11)		//Mute MIC 2  to left RECMIXER
#define M_HPMIXER_R_TO_RECMIXER_R		(0x1<<6)		//Mute left HPMIXER to right RECMIXER
#define M_AUXMIXER_R_TO_RECMIXER_R		(0x1<<5)		//Mute left AUXMIXER to right RECMIXER
#define M_SPKMIXER_TO_RECMIXER_R		(0x1<<4)		//Mute SPKMIXER to right RECMIXER
#define M_LINEIN1_TO_RECMIXER_R			(0x1<<3)		//Mute LINEIN 1 to right RECMIXER
#define M_LINEIN2_TO_RECMIXER_R			(0x1<<2)		//Mute LINEIN 2 to right RECMIXER
#define M_MIC1_TO_RECMIXER_R			(0x1<<1)		//Mute MIC 1 to righ RECMIXER
#define M_MIC2_TO_RECMIXER_R			(0x1<<0)		//Mute MIC 2  to right RECMIXER


//Aux Mixer Control(0x1A)
#define	M_RECMIXER_L_TO_AUXMIXER_L		(0x1<<13)		//Mute LEFT RecMixer to Left AUXMIXER
#define	M_MIC1_TO_AUXMIXER_L			(0x1<<12)		//Mute MIC1 to Left AUXMIXER	
#define	M_MIC2_TO_AUXMIXER_L			(0x1<<11)		//Mute MIC2 to Left AUXMIXER	
#define	M_LINEIN1_L_TO_AUXMIXER_L		(0x1<<10)		//Mute LEFT LINE 1 to Left AUXMIXER	
#define M_DAC_L_TO_AUXMIXER_L			(0x1<<8)		//Mute LEFT DAC to Left AUXMIXER	
#define	M_RECMIXER_R_TO_AUXMIXER_L		(0x1<<5)		//Mute RIGHT RecMixer to Right AUXMIXER
#define	M_MIC1_TO_AUXMIXER_R			(0x1<<4)		//Mute MIC1 to Right AUXMIXER	
#define	M_MIC2_TO_AUXMIXER_R			(0x1<<3)		//Mute MIC2 to Right AUXMIXER	
#define	M_LINEIN1_R_TO_AUXMIXER_R		(0x1<<2)		//Mute RIGHT LINE 1 to Right AUXMIXER	
#define M_DAC_R_TO_AUXMIXER_R			(0x1<<0)		//Mute RIGHT DAC to Right AUXMIXER	


//Speaker Mixer Control(0x1C)
#define SPK_VOL_INPUT_SEL_MASK			(0x3<<10)		//Speaker volume input select mask
#define	SPK_VOL_INPUT_SEL_VMID			(0x0<<10)		//Speaker volume input select VMID
#define	SPK_VOL_INPUT_SEL_SPKMIXER		(0x2<<10)		//Speaker volume input select Speaker Mixer
#define	SPK_VOL_INPUT_SEL_AUXMIXER		(0x3<<10)		//Speaker volume input select Aux MIxer
#define	M_MIC1_TO_SPKMIXER				(0x1<<7)		//Mute Mic1 to SPKMixer	
#define M_MIC2_TO_SPKMIXER				(0x1<<6)		//Mute Mic2 to SPKMixer	
#define M_LINEIN_L_TO_SPKMIXER			(0x1<<5)		//Mute LEFT LINE IN 1 TO SPKMixer
#define M_LINEIN_R_TO_SPKMIXER			(0x1<<4)		//Mute RIGHT LINE IN 1 TO SPKMixer
#define M_DAC_L_TO_SPKMIXER				(0x1<<1)		//Mute LEFT DAC to SPKMixer	
#define M_DAC_R_TO_SPKMIXER				(0x1<<0)		//Mute RIGHT DAC to SPKMixer	



//Speaker Amplifier Control(0x1E)
#define SPK_AMP_MODE_SEL_MASK			(0x1<<15)		//Speaker Amplifier Mode Control Mask
#define SPK_AMP_MODE_SEL_AB				(0x0<<15)		//Speaker Amplifier MODE SELECT CLASS AB
#define SPK_AMP_MODE_SEL_D				(0x1<<15)		//Speaker Amplifier MODE SELECT CLASS D
#define SPK_CLASS_AB_SEL_MASK			(0x1<<14)		//Class AB Amplifier Dirving Control Mask
#define SPK_CLASS_AB_SEL_STRONG			(0x0<<14)		//Class AB Amplifier Dirving select strong AMP
#define SPK_CLASS_AB_SEL_WEAK			(0x1<<14)		//Class AB Amplifier Dirving select weak AMP


//Micphone Input Control 2(0x22)
#define MIC_BIAS_90_PRECNET_AVDD	1
#define	MIC_BIAS_75_PRECNET_AVDD	2

#define MIC1_BOOST_CTRL_MASK		(0xf<<12)
#define MIC1_BOOST_CTRL_BYPASS		(0x0<<12)
#define MIC1_BOOST_CTRL_20DB		(0x1<<12)
#define MIC1_BOOST_CTRL_24DB		(0x2<<12)
#define MIC1_BOOST_CTRL_30DB		(0x3<<12)
#define MIC1_BOOST_CTRL_35DB		(0x4<<12)
#define MIC1_BOOST_CTRL_40DB		(0x5<<12)
#define MIC1_BOOST_CTRL_44DB		(0x6<<12)
#define MIC1_BOOST_CTRL_50DB		(0x7<<12)
#define MIC1_BOOST_CTRL_52DB		(0x8<<12)			

#define MIC2_BOOST_CTRL_MASK		(0xf<< 8)
#define MIC2_BOOST_CTRL_BYPASS		(0x0<< 8)
#define MIC2_BOOST_CTRL_20DB		(0x1<< 8)
#define MIC2_BOOST_CTRL_24DB		(0x2<< 8)
#define MIC2_BOOST_CTRL_30DB		(0x3<< 8)
#define MIC2_BOOST_CTRL_35DB		(0x4<< 8)
#define MIC2_BOOST_CTRL_40DB		(0x5<< 8)
#define MIC2_BOOST_CTRL_44DB		(0x6<< 8)
#define MIC2_BOOST_CTRL_50DB		(0x7<< 8)
#define MIC2_BOOST_CTRL_52DB		(0x8<< 8)

#define MICBIAS1_VOLT_CTRL_MASK		(0x1<< 7)
#define MICBIAS1_VOLT_CTRL_90P		(0x0<< 7)
#define MICBIAS1_VOLT_CTRL_75P		(0x1<< 7)

#define MICBIAS1_S_C_DET_MASK		(0x1<< 6)
#define MICBIAS1_S_C_DET_DIS		(0x0<< 6)
#define MICBIAS1_S_C_DET_ENA		(0x1<< 6)

#define MICBIAS1_SHORT_CURR_DET_MASK	(0x3<< 4)
#define MICBIAS1_SHORT_CURR_DET_600UA	(0x0<< 4)
#define MICBIAS1_SHORT_CURR_DET_1500UA	(0x1<< 4)
#define MICBIAS1_SHORT_CURR_DET_2000UA	(0x2<< 4)

#define MICBIAS2_VOLT_CTRL_MASK		(0x1<< 3)
#define MICBIAS2_VOLT_CTRL_90P		(0x0<< 3)
#define MICBIAS2_VOLT_CTRL_75P		(0x1<< 3)

#define MICBIAS2_S_C_DET_MASK		(0x1<< 2)
#define MICBIAS2_S_C_DET_DIS		(0x0<< 2)
#define MICBIAS2_S_C_DET_ENA		(0x1<< 2)

#define MICBIAS2_SHORT_CURR_DET_MASK	(0x3)
#define MICBIAS2_SHORT_CURR_DET_600UA	(0x0)
#define MICBIAS2_SHORT_CURR_DET_1500UA	(0x1)
#define MICBIAS2_SHORT_CURR_DET_2000UA	(0x2)


//Digital Microphone Control(0x24)
#define DMIC_ENA_MASK				(0x1<<15)	
#define DMIC_ENA					(0x1<<15)		
#define DMIC_DIS					(0x0<<15)		

#define M_ADC_TO_DIGITAL_MIXER      (0x1<<14)

#define DMIC_L_CH_MUTE_MASK			(0x1<<13)
#define DMIC_L_CH_UNMUTE			(0x0<<13)
#define DMIC_L_CH_MUTE	 			(0x1<<13)

#define DMIC_R_CH_MUTE_MASK			(0x1<<12)
#define DMIC_R_CH_UNMUTE			(0x0<<12)
#define DMIC_R_CH_MUTE				(0x1<<12)

#define DMIC_L_CH_LATCH_MASK		(0x1<< 9)
#define DMIC_L_CH_LATCH_RISING		(0x1<< 9)
#define DMIC_L_CH_LATCH_FALLING		(0x0<< 9)

#define DMIC_R_CH_LATCH_MASK		(0x1<< 8)
#define DMIC_R_CH_LATCH_RISING		(0x1<< 8)
#define DMIC_R_CH_LATCH_FALLING		(0x0<< 8)

#define DMIC_CLK_SET_MASK			(0x7<<3)
#define DMIC_CLK_SET_256FS_DIV2		(0x0<<3)
#define DMIC_CLK_SET_256FS_DIV4		(0x1<<3)
#define DMIC_CLK_SET_256FS_DIV6		(0x2<<3)
#define DMIC_CLK_SET_256FS_DIV12	(0x3<<3)
#define DMIC_CLK_SET_256FS_DIV24	(0x4<<3)



//Stereo I2S Serial Data Port Control(0x34)
#define SDP_MODE_SEL_MASK				(0x1<<15)		//Main I2S interface select MASK		
#define SDP_MODE_SEL_MASTER				(0x0<<15)		//Main I2S interface select MASTER MODE
#define SDP_MODE_SEL_SLAVE				(0x1<<15)		//Main I2S interface select SLAVE MODE

#define SDP_CP_ADC_L_TO_ADC_R			(0x1<<14)		//COPY ADC LEFT CHANNEL DIGITAL DATA TO RIGHT CHANNEL
#define SDP_CP_ADC_R_TO_ADC_L			(0x1<<13)		//COPY ADC RIGHT CHANNEL DIGITAL DATA TO LEFT CHANNEL

#define SDP_ADC_CPS_SEL_MASK			(0x3<<10)		//ADC Compress select Mask
#define SDP_ADC_CPS_SEL_OFF				(0x0<<10)		//ADC Compress select OFF
#define SDP_ADC_CPS_SEL_U_LAW			(0x1<<10)		//ADC Compress select u_law
#define SDP_ADC_CPS_SEL_A_LAW			(0x2<<10)		//ADC Compress select a_law

#define SDP_DAC_CPS_SEL_MASK			(0x3<< 8)		//DAC Compress select Mask
#define SDP_DAC_CPS_SEL_OFF				(0x0<< 8)		//DAC Compress select OFF
#define SDP_DAC_CPS_SEL_U_LAW			(0x1<< 8)		//DAC Compress select u_law
#define SDP_DAC_CPS_SEL_A_LAW			(0x2<< 8)		//DAC Compress select a_law
												 	
#define SDP_I2S_BCLK_POL_INV			(0x1<<7)		//0:Normal 1:Invert

#define SDP_DAC_R_INV					(0x1<<6)		//0:Normal 1:Invert

#define SDP_ADC_DATA_L_R_SWAP			(0x1<<5)		//0:ADC data appear at left phase of LRCK
														//1:ADC data appear at right phase of LRCK
#define SDP_DAC_DATA_L_R_SWAP			(0x1<<4)		//0:DAC data appear at left phase of LRCK
														//1:DAC data appear at right phase of LRCK	
//Data Length Slection
#define SDP_I2S_DL_MASK					(0x3<<2)		//Stereo Serial Data Length mask	
#define SDP_I2S_DL_16					(0x0<<2)		//16 bits
#define SDP_I2S_DL_20					(0x1<<2)		//20 bits
#define	SDP_I2S_DL_24					(0x2<<2)		//24 bits
#define SDP_I2S_DL_8					(0x3<<2)		//8  bits
													
//PCM Data Format Selection
#define SDP_I2S_DF_MASK					(0x3)			//main i2s Data Format mask
#define SDP_I2S_DF_I2S					(0x0)			//I2S FORMAT 
#define SDP_I2S_DF_RIGHT				(0x1)			//Left JUSTIFIED
#define	SDP_I2S_DF_LEFT					(0x2)			//PCM format A
#define SDP_I2S_DF_PCM					(0x3)			//PCM format B

//Stereo AD/DA Clock Control(0x38h)
#define I2S_PRE_DIV_MASK				(0x7<<13)			
#define I2S_PRE_DIV_1					(0x0<<13)			//DIV 1
#define I2S_PRE_DIV_2					(0x1<<13)			//DIV 2
#define I2S_PRE_DIV_4					(0x2<<13)			//DIV 4
#define I2S_PRE_DIV_8					(0x3<<13)			//DIV 8
#define I2S_PRE_DIV_16					(0x4<<13)			//DIV 16
#define I2S_PRE_DIV_32					(0x5<<13)			//DIV 32

#define I2S_LRCK_SEL_N_BCLK_MASK		(0x1<<12)			//CLOCK RELATIVE OF BCLK AND LCRK
#define I2S_LRCK_SEL_64_BCLK			(0x0<<12)			//64FS
#define I2S_LRCK_SEL_32_BCLK			(0x1<<12)			//32FS

#define DAC_OSR_SEL_MASK				(0x3<<10)			
#define DAC_OSR_SEL_128FS				(0x3<<10)
#define DAC_OSR_SEL_64FS				(0x3<<10)
#define DAC_OSR_SEL_32FS				(0x3<<10)
#define DAC_OSR_SEL_16FS				(0x3<<10)

#define ADC_OSR_SEL_MASK				(0x3<< 8)
#define ADC_OSR_SEL_128FS				(0x3<< 8)	
#define ADC_OSR_SEL_64FS				(0x3<< 8)	
#define ADC_OSR_SEL_32FS				(0x3<< 8)	
#define ADC_OSR_SEL_16FS				(0x3<< 8)						

#define ADDA_FILTER_CLK_SEL_MASK		(1<<7)			//MASK
#define ADDA_FILTER_CLK_SEL_256FS		(0<<7)			//256FS
#define ADDA_FILTER_CLK_SEL_384FS		(1<<7)			//384FS



//Power managment addition 1 (0x3A),0:Disable,1:Enable
#define PWR_MAIN_I2S_EN					(0x1<<15)
#define PWR_CLASS_D						(0x1<<13)	
#define PWR_ADC_L_CLK					(0x1<<12)
#define PWR_ADC_R_CLK					(0x1<<11)
#define PWR_DAC_L_CLK					(0x1<<10)
#define	PWR_DAC_R_CLK					(0x1<< 9)
#define	PWR_DAC_REF						(0x1<< 8)
#define	PWR_DAC_L_TO_MIXER				(0x1<< 7)
#define	PWR_DAC_R_TO_MIXER				(0x1<< 6)
#define PWR_CLASS_AB					(0x1<< 5)
#define PWR_PRE_SOU_CUR_ENA				(0x1<< 4)


//Power managment addition 2 (0x3B),0:Disable,1:Enable
#define PWR_HPMIXER_L					(0x1<<15)		
#define PWR_HPMIXER_R					(0x1<<14)
#define PWR_AUXMIXER_L					(0x1<<13)
#define PWR_AUXMIXER_R					(0x1<<12)
#define PWR_RECMIXER_L					(0x1<<11)
#define PWR_RECMIXER_R					(0x1<<10)
#define PWR_LINEIN1_L					(0x1<< 9)
#define PWR_LINEIN1_R					(0x1<< 8)
#define PWR_LINEIN2_L					(0x1<< 7)
#define PWR_LINEIN2_R					(0x1<< 6)
#define PWR_MIC1_BOOT					(0x1<< 5)
#define PWR_MIC2_BOOT					(0x1<< 4)
#define PWR_MICBIAS1					(0x1<< 3)
#define PWR_MICBIAS2					(0x1<< 2)
#define PWR_PLL							(0x1<< 1)
#define PWR_SPKMIXER					(0x1<< 0)


//Power managment addition 3(0x3C),0:Disable,1:Enable
#define PWR_VREF					(0x1<<15)
#define PWR_DIS_FAST_VREF			(0x1<<14)
#define PWR_MAIN_BIAS				(0x1<<13)
#define PWR_TP_ADC					(0x1<<12)
#define PWR_HP_AMP					(0x1<< 3)
#define PWR_HP_BUF					(0x1<< 2)
#define PWR_HP_DIS_DEPOP			(0x1<< 1)
#define PWR_HP_AMP_DRI				(0x1<< 0)


//Power managment addition 4(0x3E),0:Disable,1:Enable
#define PWR_SPK_L_VOL				(0x1<<15)
#define PWR_SPK_R_VOL				(0x1<<14)
#define PWR_HP_L_VOL				(0x1<<11)
#define PWR_HP_R_VOL				(0x1<<10)
#define PWR_AUXOUT_L_VOL			(0x1<< 9)
#define PWR_AUXOUT_R_VOL			(0x1<< 8)
#define PWR_LDO						(0x1<< 7)


//General Purpose Control Register(0x40)
#define SPK_AMP_AUTO_RATIO_EN			(0x1<<15)	//Speaker Amplifier Auto Ratio Gain Control

#define SPK_AMP_RATIO_CTRL_MASK			(0x7<<12)
#define SPK_AMP_RATIO_CTRL_2_34			(0x7<<12)		
#define SPK_AMP_RATIO_CTRL_2_00			(0x6<<12)		
#define SPK_AMP_RATIO_CTRL_1_68			(0x5<<12)		
#define SPK_AMP_RATIO_CTRL_1_56			(0x4<<12)		
#define SPK_AMP_RATIO_CTRL_1_44			(0x3<<12)		
#define SPK_AMP_RATIO_CTRL_1_27			(0x2<<12)		
#define SPK_AMP_RATIO_CTRL_1_10			(0x1<<12)		
#define SPK_AMP_RATIO_CTRL_1_00			(0x0<<12)		

#define STEREO_DAC_HI_PASS_FILT_EN		(0x1<<11)		//Stereo DAC high pass filter enable
#define STEREO_ADC_HI_PASS_FILT_EN		(0x1<<10)		//Stereo ADC high pass filter enable

#define ADC_WIND_FILT_COFF_MASK			(0x3F<<2)		//Select ADC Wind Filter Clock type

#define ADC_WIND_FILT_CTRL_MASK			(0x3<<0)		//ADC WIND FILTER CTRL MASK
#define ADC_WIND_FILT_CTRL_DISABLE		(0x0<<0)		//ADC WIND FILTER Disable
#define ADC_WIND_FILT_CTRL_1S_HPF		(0x1<<0)		//ADC WIND FILTER CTRL 1 Order HPF
#define ADC_WIND_FILT_CTRL_2S_HPF		(0x2<<0)		//ADC WIND FILTER CTRL 2 Order HPF


//Global Clock Control Register(0x42)
#define SYSCLK_SOUR_SEL_MASK			(0x1<<14)
#define SYSCLK_SOUR_SEL_MCLK			(0x0<<14)		//system Clock source from MCLK
#define SYSCLK_SOUR_SEL_PLL				(0x1<<14)		//system Clock source from PLL
#define SYSCLK_SOUR_SEL_PLL_TCK			(0x2<<14)		//system Clock source from PLL track

#define PLLCLK_SOUR_SEL_MCLK			(0x0<<12)		//PLL clock source from MCLK
#define PLLCLK_SOUR_SEL_BITCLK			(0x1<<12)		//PLL clock source from BITCLK

#define PLLCLK_PRE_DIV1					(0x0<<11)			//DIV 1
#define PLLCLK_PRE_DIV2					(0x1<<11)			//DIV 2

//PLL Control(0x44)

#define PLL_CTRL_M_VAL(m)				((m)&0xf)			//M code for analog PLL
#define PLL_CTRL_K_VAL(k)				(((k)&0x7)<<4)		//K code for analog PLL
#define PLL_CTRL_N_VAL(n)				(((n)&0xff)<<8)		//N code for analog PLL

//Video Buffer Control(0x46)
#define VIDEO_BUF_ENA					(0x1<<15)			//Enable Video Buffer
#define VIDEO_BUF_CLAMP_CTRL			(0x1<<14)			//Enable Video Clamp 
#define VIDEO_BUF_FAST_CLAMP_CTRL		(0x1<<13)			//Enable Video Fast Clamp 

#define VIDEO_CLAMP_REF_SEL_MASK		(0x7<<10)			//CLAMP REFERENCE SELECT MASK
#define VIDEO_CLAMP_REF_SEL_0V			(0x0<<10)			//CLAMP REFERENCE SELECT 0 V 
#define VIDEO_CLAMP_REF_SEL_20MV		(0x1<<10)			//CLAMP REFERENCE SELECT 20 MV
#define VIDEO_CLAMP_REF_SEL_40MV		(0x2<<10)			//CLAMP REFERENCE SELECT 40 MV
#define VIDEO_CLAMP_REF_SEL_60MV		(0x3<<10)			//CLAMP REFERENCE SELECT 60 MV
#define VIDEO_CLAMP_REF_SEL_80MV		(0x4<<10)			//CLAMP REFERENCE SELECT 80 MV
#define VIDEO_CLAMP_REF_SEL_100MV		(0x5<<10)			//CLAMP REFERENCE SELECT 100 MV
#define VIDEO_CLAMP_REF_SEL_120MV		(0x6<<10)			//CLAMP REFERENCE SELECT 120 MV
#define VIDEO_CLAMP_REF_SEL_140MV		(0x7<<10)			//CLAMP REFERENCE SELECT 140 MV

#define VIDEO_PULL_DOWN_RES_SEL_MASK	(0x7<<0)			//PULL DOWN RESISTANCE SELECT MASK
#define VIDEO_PULL_DOWN_RES_SEL_1_6K	(0x0<<0)			//PULL DOWN RESISTANCE SELECT 1.6K
#define VIDEO_PULL_DOWN_RES_SEL_9K	 	(0x1<<0)			//PULL DOWN RESISTANCE SELECT 9K
#define VIDEO_PULL_DOWN_RES_SEL_150K	(0x2<<0)			//PULL DOWN RESISTANCE SELECT 150K
#define VIDEO_PULL_DOWN_RES_SEL_440K	(0x3<<0)			//PULL DOWN RESISTANCE SELECT 440K
#define VIDEO_PULL_DOWN_RES_SEL_730K	(0x4<<0)			//PULL DOWN RESISTANCE SELECT 730K
#define VIDEO_PULL_DOWN_RES_SEL_1_46M	(0x5<<0)			//PULL DOWN RESISTANCE SELECT 1.46M
#define VIDEO_PULL_DOWN_RES_SEL_3M	 	(0x6<<0)			//PULL DOWN RESISTANCE SELECT 3M
#define VIDEO_PULL_DOWN_RES_SEL_4_3M	(0x7<<0)			//PULL DOWN RESISTANCE SELECT 4.3M


//GPIO Control 1(0x4C)
#define GPIO1_PIN_FUN_SEL_MASK			(0x3<<1)
#define GPIO1_PIN_FUN_SEL_GPIO1			(0x0<<1)			//GPIO PIN SELECT GPIO
#define GPIO1_PIN_FUN_SEL_DIMC			(0x1<<1)			//GPIO PIN SELECT DMIC
#define GPIO1_PIN_FUN_SEL_IRQ			(0x2<<1)			//GPIO pin SELECT IRQ


//GPIO Control 2(0x4D)
#define GPIO_PIN_CONF_MASK				(0x1<<11)
#define GPIO_PIN_CONF_INPUT				(0x0<<11)
#define GPIO_PIN_CONF_OUTPUT			(0x1<<11)

#define GPIO_OUTPUT_DRI_MASK			(0x1<<10)
#define GPIO_OUTPUT_DRI_LOW				(0x0<<10)
#define GPIO_OUTPUT_DRI_HIGH			(0x1<<10)


//DEPOP MODE CONTROL 1(0x54)
#define PW_SOFT_GEN						(0x1<<15)
#define EN_SOFT_FOR_S_M_DEPOP			(0x1<<14)
#define EN_DEPOP_2						(0x1<<7)
#define EN_DEPOP_1						(0x1<<6)
#define EN_ONE_BIT_HP_DEPOP				(0x1<<3)
#define EN_HP_L_M_UM_DEPOP				(0x1<<1)
#define EN_HP_R_M_UM_DEPOP 				(0x1<<0)



//Jack Detect Control Register(0x5A)
#define JD_USE_MASK					(0x7<<13)				//JD Pin select
#define JD_USE_JD2					(0x5<<13)				//select JD2 from Line1_L
#define JD_USE_JD1					(0x4<<13)				//select JD1 from Line1_R
#define JD_OFF						(0x0<<13)				//off

#define JD_HP_EN					(0x1<<11)				//JD trigger enable for HP

#define JD_HP_TRI_MASK				(0x1<<10)				//Trigger mask
#define JD_HP_TRI_HI				(0x1<<10)				//high trigger
#define JD_HP_TRI_LO				(0x0<<10)				//low trigger


#define JD_SPK_ENA_MASK				(0x3<<8)				//Enable Jack Detect Trigger for SPK
#define JD_SPK_DIS					(0x0<<8)				//Disable JD for Speaker
#define JD_SPK_ENA_SPK_LP_LN		(0x1<<8)				//Enable Jack Detect Trigger for SPK_LP/LN
#define JD_SPK_ENA_SPK_LP_RP		(0x2<<8)				//Enable Jack Detect Trigger for SPK_LP/RP

#define JD_SPK_TRI_MASK				(0x1<<7)				//Trigger mask
#define JD_SPK_TRI_HI				(0x1<<7)				//high trigger
#define JD_SPK_TRI_LO				(0x0<<7)				//low trigger

#define JD_LINE1_L_SHARE_JD1		(0x1<<3)				//LINE IN1 LEFT PIN SHARE TO JD1
#define JD_LINE1_R_SHARE_JD2		(0x1<<2)				//LINE IN1 RIGHT PIN SHARE TO JD2

#define JD_AUXOUT_EN				(0x1<<1)				//JD trigger enable for AUXOUT

#define JD_AUXOUT_MASK				(0x1<<0)				//Trigger mask
#define JD_AUXOUT_TRI_HI			(0x1<<0)				//high trigger
#define JD_AUXOUT_TRI_LO			(0x0<<0)				//low trigger



////ALC CONTROL 1(0x64)
#define ALC_ATTACK_RATE_MASK		(0x1F<<8)				//select ALC attack rate
#define ALC_RECOVERY_RATE_MASK		(0x1F<<0)				//select ALC Recovery rate


////ALC CONTROL 2(0x65)
#define ALC_COM_NOISE_GATE_MASK		(0xF<<0)				//select Compensation gain for Noise gate function


////ALC CONTROL 3(0x66)
#define ALC_FUN_MASK				(0x3<<14)				//select ALC path 
#define ALC_FUN_DIS					(0x0<<14)				//disable
#define ALC_ENA_DAC_PATH			(0x1<<14)				//DAC path
#define ALC_ENA_ADC_PATH			(0x3<<14)				//ADC path

#define ALC_PARA_UPDATE				(0x1<<13)				//update ALC parameter

#define ALC_LIMIT_LEVEL_MASK		(0x1F<<8)				//ALC limit level

#define ALC_NOISE_GATE_FUN_MASK		(0x1<<7)				//ALC noise gate function
#define ALC_NOISE_GATE_FUN_DIS		(0x0<<7)				//disable
#define ALC_NOISE_GATE_FUN_ENA		(0x1<<7)				//enable

#define ALC_NOISE_GATE_H_D_MASK		(0x1<<6)				//ALC noise gate hold data function
#define ALC_NOISE_GATE_H_D_DIS		(0x0<<6)				//disable
#define ALC_NOISE_GATE_H_D_ENA		(0x1<<6)				//enable

//Psedueo Stereo & Spatial Effect Block Control(0x68)
#define SPATIAL_CTRL_EN				(0x1<<15)				//enable Spatial effect
#define ALL_PASS_FILTER_EN			(0x1<<14)				//enable all pass filter
#define PSEUDO_STEREO_EN			(0x1<<13)				//enable pseudo stereo block
#define STEREO_EXPENSION_EN			(0x1<<12)				//enable stereo expansion block

#define GAIN1_3D_PARA_MASK			(0x3<<10)				//3D gain1 parameter
#define GAIN1_3D_PARA_1_00			(0x0<<10)				//3D gain1 1.0
#define GAIN1_3D_PARA_1_50			(0x1<<10)				//3D gain1 1.5
#define GAIN1_3D_PARA_2_00			(0x2<<10)				//3D gain1 2.0

#define RATIO1_3D_MASK				(0x3<<8)				//3D ratio1 parameter
#define RATIO1_3D_0_0				(0x0<<8)				//3D ratio1 0.0
#define RATIO1_3D_0_66				(0x1<<8)				//3D ratio1 0.66
#define RATIO1_3D_1_0				(0x2<<8)				//3D ratio1 1.0

#define GAIN2_3D_PARA_MASK			(0x3<<6)				//3D gain2 parameter
#define GAIN2_3D_PARA_1_00			(0x0<<6)				//3D gain2 1.0
#define GAIN2_3D_PARA_1_50			(0x1<<6)				//3D gain2 1.5
#define GAIN2_3D_PARA_2_00			(0x2<<6)				//3D gain2 2.0

#define RATIO2_3D_MASK				(0x3<<4)				//3D ratio2 parameter
#define RATIO2_3D_0_0				(0x0<<4)				//3D ratio2 0.0
#define RATIO2_3D_0_66				(0x1<<4)				//3D ratio2 0.66
#define RATIO2_3D_1_0				(0x2<<4)				//3D ratio2 1.0

#define APF_FUN_SLE_MASK			(0x3<<0)				//select samplerate for all pass filter 
#define APF_FUN_SEL_48K				(0x3<<0)				//select 48k
#define APF_FUN_SEL_44_1K			(0x2<<0)				//select 44.1k
#define APF_FUN_SEL_32K				(0x1<<0)				//select 32k
#define APF_FUN_DIS					(0x0<<0)				//disable


//EQ CONTROL 1(0x6E)

#define HW_EQ_PATH_SEL_MASK			(0x1<<15)		//HW EQ FUN SEL
#define HW_EQ_PATH_SEL_DAC			(0x0<<15)		//HW EQ FOR DAC PATH
#define HW_EQ_PATH_SEL_ADC			(0x1<<15)		//HW EQ FOR ADC PATH

#define HW_EQ_UPDATE_CTRL			(0x1<<13)		//HW EQ Parameter Update CTRL

#define HW_EQ_ZC_UPDATE_PARA		(0x1<<12)		//Zero Detection for HW EQ Update parameter


//EQ CONTROL 2(0x70)

#define EN_HW_EQ_HPF2				(0x1<<7)		//EQ High Pass Filter 2 Control
#define EN_HW_EQ_HPF1				(0x1<<6)		//EQ High Pass Filter 1 Control
#define EN_HW_EQ_BP4				(0x1<<4)		//EQ Band-4 Control
#define EN_HW_EQ_BP3				(0x1<<3)		//EQ Band-3 Control
#define EN_HW_EQ_BP2				(0x1<<2)		//EQ Band-2 Control
#define EN_HW_EQ_BP1				(0x1<<1)		//EQ Band-1 Control
#define EN_HW_EQ_LPF				(0x1<<0)		//EQ Low Pass Filter Control



#define REALTEK_HWDEP 	0
#define ALC5633_SPI_IN_USED	0	

enum {
        ALC5633_AIF1,
        ALC5633_AIF2,
        ALC5633_AIFS,
};

struct alc5633_setup_data {
	unsigned short i2c_address;
	unsigned short i2c_bus;	
};


#endif //__ALC5633_H__
