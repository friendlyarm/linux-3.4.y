#ifndef __NX_FOURCC_H__
#define __NX_FOURCC_H__

/*------------------------------------------------------------------------------
 * Macro: FOURCC for video
 */
#ifndef MAKEFOURCC
#define MAKEFOURCC(a,b,c,d) (((unsigned int)a) | (((unsigned int)b)<< 8) | (((unsigned int)c)<<16) | (((unsigned int)d)<<24) )
#endif

//	Specific Pixel Format FourCC for NX series
#define FOURCC_MVS0		MAKEFOURCC('M', 'V', 'S', '0')	//	YCbCr 420 : Plarnar Format
#define FOURCC_MVS2		MAKEFOURCC('M', 'V', 'S', '2')	//	YCbCr 422 : Plarnar Format
#define FOURCC_MVS4		MAKEFOURCC('M', 'V', 'S', '4')	//	YCbCr 444 : Plaranr Format
#define FOURCC_MVN2		MAKEFOURCC('M', 'V', 'N', '2')	//	YUV 4:2:2 packed foramt(same as FOURCC_YUYV)

//	YUV Packed Format
#define FOURCC_YUY2		MAKEFOURCC('Y', 'U', 'Y', '2')	//	MPEG1 media format (1D nonseperated 422 format - Y,Cb,Y,Cr)
#define FOURCC_YUYV		MAKEFOURCC('Y', 'U', 'Y', 'V')	//	Duplicated of YUY2(1D nonseperated 422 format - Y,Cb,Y,Cr) */
#define FOURCC_UYVY		MAKEFOURCC('U', 'Y', 'V', 'Y')	//	YUV 4:2:2 (Y sample at every pixel, U and V sampled at every second pixel horizontally on each line).
														//	A macropixel contains 2 pixels in 1 u_int32.
//	YUV Seperate Format ( Planar Format )
#define FOURCC_IYUV		MAKEFOURCC('I', 'Y', 'U', 'V')	//	8 bit Y plane followed by 8 bit 2x2 subsampled U and V planes.
#define FOURCC_YV12		MAKEFOURCC('Y', 'V', '1', '2')	//	8 bit Y plane followed by 8 bit 2x2 subsampled V and U planes.
#define	FOURCC_NV12		MAKEFOURCC('N', 'V', '1', '2')	//	8-bit Y plane followed by an interleaved U/V plane with 2x2 subsampling.(CbCr Order)
#define	FOURCC_NV21		MAKEFOURCC('N', 'V', '2', '1')	//	As NV12 with U(Cb) and V(Cr) reversed in the interleaved plane.(CrCb Order)


#endif	//	__NX_FOURCC_H__
