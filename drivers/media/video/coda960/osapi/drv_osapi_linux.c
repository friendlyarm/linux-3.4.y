#include <linux/string.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/cma.h>
#include <linux/mm.h>

#include "../include/drv_osapi.h"

//
//		Configurations
//

//	Memory Allocator Debugging
#define	DEBUG_DRV_MEM		0




//////////////////////////////////////////////////////////////////////////////
//
//					Memory Allocator Part
//


#if DEBUG_DRV_MEM
static int gst_AllocCounter = 0;
#endif

void	*NX_DrvMalloc( int size )
{
#if DEBUG_DRV_MEM
	void *ptr = vmalloc(size);
	if( !ptr )
	{
		printk("[VPU_MALLOC] : Allocation Failed.(size=%d)\n", size);
		return NULL;
	}
	gst_AllocCounter ++;
	return ptr;
#else
	return vmalloc(size);
#endif
}

void	NX_DrvFree( void *ptr )
{
#if DEBUG_DRV_MEM
	gst_AllocCounter --;
	if( ptr )
	{
		vfree( ptr );
	}
	else
	{
		printk("[VPU_MALLOC] : Invalid Pointer.\n");
	}
#else
	vfree( ptr );
#endif
}

void	NX_DrvMemset( void *ptr, int value, int size )
{
	memset( ptr, value, size );
}

void	NX_DrvMemcpy( void *dst, void *src, int size )
{
	memcpy( dst, src, size );
}

//
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//				Physically Linear Memory Allocation
//
void	NX_LinearAlloc( void *handle, unsigned int *phyAddr, unsigned int *virAddr, int size, int align )
{
	*phyAddr= cma_alloc( handle, "nxp-ion", size, align );
	if( *phyAddr == (-EINVAL) )
	{
		*phyAddr = 0;
		NX_ErrMsg( ("Failed to allocation firmware memory\n") );
		return;
	}
	*virAddr = (unsigned int)cma_get_virt( *phyAddr, size, 1 );		//	Get non cachable memory

	if( 0 == *virAddr )
	{
		cma_free( *phyAddr );
		NX_ErrMsg(("Failed to cma_get_virt(0x%08x)\n", (int)*phyAddr));
		return;
	}
}

void	NX_LinearFree( void *handle, unsigned int phyAddr, unsigned int virAddr )
{
	cma_free(phyAddr);
}
//
//
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//				Driver Layer Mutex
//


void DrvInitMutex( NX_MutexHandle handle, const char *name )
{
	__mutex_init( &handle->linux_mutex, name, &handle->key );
}

void DrvCloseMutex( NX_MutexHandle handle )
{
	LinuxMutex *hMutex = (LinuxMutex *)handle;
	mutex_destroy( &hMutex->linux_mutex );
}

void DrvMutexLock( NX_MutexHandle handle )
{
	LinuxMutex *hMutex = (LinuxMutex *)handle;
	mutex_lock( &hMutex->linux_mutex );
}

void DrvMutexUnlock( NX_MutexHandle handle )
{
	LinuxMutex *hMutex = (LinuxMutex *)handle;
	mutex_unlock( &hMutex->linux_mutex );
}

//
//
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//				Timer
//
void DrvMSleep( unsigned int mSeconds )
{
	msleep( mSeconds );
}

void DrvUSleep( unsigned int uSeconds )
{
	udelay( uSeconds );
}
	
//
//
//
//////////////////////////////////////////////////////////////////////////////

