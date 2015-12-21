#ifndef __NX_ALLOC_MEM_H__
#define __NX_ALLOC_MEM_H__

#ifdef	__cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
//			Enum & Define
enum {
	NX_MEM_MAP_LINEAR = 0,		//	Linear Memory Type
	NX_MEM_MAP_TILED  = 1,		//	Tiled Memory Type
};


//
//	Nexell Private Memory Type
//
typedef struct
{
	void			*privateDesc;
	int				align;
	int				size;
	unsigned int	virAddr;
	unsigned int	phyAddr;
} NX_MEMORY_INFO, *NX_MEMORY_HANDLE;



//
//	Nexell Private Video Memory Type
//
typedef struct
{
	void			*privateDesc[3];//	Private Descriptor( for allocator's handle or descriptor )
	int				align;			//	Start Address Align( L/Cb/Cr )
	int				memoryMap;		//	Memory Map Type( Linear or Tiled,. etc, N/A )
	unsigned int	fourCC;			//	Four Charect Code
	int				imgWidth;		//	Video Image's Width
	int				imgHeight;		//	Video Image's Height

	unsigned int	luPhyAddr;
	unsigned int	luVirAddr;
	unsigned int	luStride;

	unsigned int	cbPhyAddr;
	unsigned int	cbVirAddr;
	unsigned int	cbStride;

	unsigned int	crPhyAddr;
	unsigned int	crVirAddr;
	unsigned int	crStride;
} NX_VID_MEMORY_INFO, *NX_VID_MEMORY_HANDLE;



//	Nexell Private Memory Allocator
NX_MEMORY_HANDLE NX_AllocateMemory( int size, int align );
void NX_FreeMemory( NX_MEMORY_HANDLE handle );


//	Video Specific Allocator Wrapper
NX_VID_MEMORY_HANDLE NX_VideoAllocateMemory( int align, int width, int height, int memMap, int fourCC );
void NX_FreeVideoMemory( NX_VID_MEMORY_HANDLE handle );


#ifdef	__cplusplus
};
#endif

#endif	//	__NX_ALLOC_MEM_H__
