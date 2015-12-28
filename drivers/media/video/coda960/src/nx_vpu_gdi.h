#ifndef __NX_VPU_GDI_H__
#define	__NX_VPU_GDI_H__

int SetTiledMapType(int mapType, int stride, int interleave);
NX_VPU_RET ConfigEncSecAXI(int codecMode, SecAxiInfo *sa, int width, int height);
NX_VPU_RET ConfigDecSecAXI(int codStd, SecAxiInfo *sa, int width, int height );
//NX_VPU_RET ConfigDecSecAXI(int codStd, SecAxiInfo *sa, int width, int height, unsigned int axiAddr, int axiBufSize );
unsigned int MaverickCache2Config(int decoder , int interleave, int bypass, int burst, int merge, int mapType, int wayshape);


#endif	//__NX_VPU_GDI_H__