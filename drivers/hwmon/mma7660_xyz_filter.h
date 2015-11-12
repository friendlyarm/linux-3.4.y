#include <linux/kernel.h>

typedef signed char		VINT8;
typedef unsigned char	VUINT8;
typedef signed short	VINT16;
typedef unsigned short	VUINT16;
typedef unsigned long	VUINT32;
typedef signed long		VINT32;

#define _MMA7660_LOW_PASS_FILTER_
#define LowPassFactor 3
#define RawDataLength 4

#define MMA7660_Sin30pos 10
#define MMA7660_Sin30neg -10

VINT8 Xraw[RawDataLength];
VINT8 Yraw[RawDataLength];
VINT8 Zraw[RawDataLength];

VINT8 RawDataPointer = 0;

VINT8 Xnew8, Ynew8, Znew8;
VINT8 Xavg8, Yavg8, Zavg8;
#ifdef _MMA7660_LOW_PASS_FILTER_
VINT8 Xflt8, Yflt8, Zflt8;
VINT8 Xrsdl, Yrsdl, Zrsdl;
#endif

void XYZ_Filter(VINT8 *X, VINT8 *Y, VINT8 *Z);

void XYZ_Filter(VINT8 *X, VINT8 *Y, VINT8 *Z)
{
	VUINT8 i;
	VINT8 temp8_1, temp8_2;
	VINT16 temp16;

	Xnew8 = *X;
	Ynew8 = *Y;
	Znew8 = *Z;

	//printk(KERN_ERR "filting...\n");

#ifdef _MMA7660_LOW_PASS_FILTER_

	if((++RawDataPointer)>=RawDataLength) RawDataPointer = 0;

	Xraw[RawDataPointer] = Xnew8;
	Yraw[RawDataPointer] = Ynew8;
	Zraw[RawDataPointer] = Znew8;

	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Xraw[i];
	}
	Xflt8 = (VINT8)(temp16/RawDataLength);
	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Yraw[i];
	}
	Yflt8 = (VINT8)(temp16/RawDataLength);
	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Zraw[i];
	}
	Zflt8 = (VINT8)(temp16/RawDataLength);

	temp8_1 = Xflt8 - Xavg8;
	temp8_2 = temp8_1 / (1 + LowPassFactor);
	Xavg8 += temp8_2;
	temp8_1 -= temp8_2 * (1 + LowPassFactor);   //Current Residual
	Xrsdl += temp8_1;                           //Overall Residual
	temp8_2 = Xrsdl / (1 + LowPassFactor);
	Xavg8 += temp8_2;
	Xrsdl -= temp8_2 * (1 + LowPassFactor);

	temp8_1 = Yflt8 - Yavg8;
	temp8_2 = temp8_1 / (1 + LowPassFactor);
	Yavg8 += temp8_2;
	temp8_1 -= temp8_2 * (1 + LowPassFactor);
	Yrsdl += temp8_1;
	temp8_2 = Yrsdl / (1 + LowPassFactor);
	Yavg8 += temp8_2;
	Yrsdl -= temp8_2 * (1 + LowPassFactor);

	temp8_1 = Zflt8 - Zavg8;
	temp8_2 = temp8_1 / (1 + LowPassFactor);
	Zavg8 += temp8_2;
	temp8_1 -= temp8_2 * (1 + LowPassFactor);
	Zrsdl += temp8_1;
	temp8_2 = Zrsdl / (1 + LowPassFactor);
	Zavg8 += temp8_2;
	Zrsdl -= temp8_2 * (1 + LowPassFactor);

	*X = Xavg8;
	*Y = Yavg8;
	*Z = Zavg8;

#else
	if((++RawDataPointer)>=RawDataLength) RawDataPointer = 0;

	Xraw[RawDataPointer] = Xnew8;
	Yraw[RawDataPointer] = Ynew8;
	Zraw[RawDataPointer] = Znew8;

	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Xraw[i];
	}
	Xavg8 = (VINT8)(temp16/RawDataLength);
	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Yraw[i];
	}
	Yavg8 = (VINT8)(temp16/RawDataLength);
	for(i=0, temp16=0;i<RawDataLength;i++)
	{
		temp16 += Zraw[i];
	}
	Zavg8 = (VINT8)(temp16/RawDataLength);

	*X = Xavg8;
	*Y = Yavg8;
	*Z = Zavg8;
#endif
}




