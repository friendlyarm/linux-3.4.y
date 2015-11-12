/*
 * ADC definitions
 */
#define	ADC_MAX_SAMPLE_RATE		1*1000*1000	// with 6bit
#define	ADC_MAX_SAMPLE_BITS		6
#define	ADC_MAX_PRESCALE		256			// 8bit
#define	ADC_MIN_PRESCALE		20			// 8bit
//#define	ADC_WAIT_DELAY			1000000		// usec



#ifdef CONFIG_ARCH_S5P4418
/*
 * ADC register
 */
struct adc_register {
	volatile U32 ADCCON;
	volatile U32 ADCDAT;
	volatile U32 ADCINTENB;
	volatile U32 ADCINTCLR;
};

#define	APEN_BITP	(14)
#define	APSV_BITP	(6)
#define	ASEL_BITP	(3)
#define	ADCON_STBY	(2)
#define	ADEN_BITP	(0)
#define	AIEN_BITP	(0)
#define	AICL_BITP	(0)

#else	/* CONFIG_ARCH_S5P6818 */

/*
 * ADC register
 */
struct adc_register {
	volatile U32 ADCCON;
	volatile U32 ADCDAT;
	volatile U32 ADCINTENB;
	volatile U32 ADCINTCLR;
	volatile U32 ADCPRESCON;
};

/* PRESCALERCON */
#define	APEN_BITP		(15)	/* 15  */
#define	PRES_BITP		(0)		/* 9:0 */

/* ADCCON */
#define DATA_SEL_VAL	(0)		/* 0:5clk, 1:4clk, 2:3clk, 3:2clk, 4:1clk: 5:not delayed, else: 4clk */
#define CLK_CNT_VAL		(6)		/* 28nm ADC */

#define DATA_SEL_BITP	(10)	/* 13:10 */
#define CLK_CNT_BITP	(6)		/* 9:6 */
#define	ASEL_BITP		(3)
#define	ADCON_STBY		(2)
#define	ADEN_BITP		(0)

/* ADCINTENB */
#define	AIEN_BITP		(0)

/* ADCINTCLR */
#define	AICL_BITP		(0)

#endif /* CONFIG_ARCH_S5P6818 */

#define	ADC_BASE	((struct adc_register *)IO_ADDRESS(PHY_BASEADDR_ADC))

