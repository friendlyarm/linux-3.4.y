#ifndef __DRV_OSAPI_H__
#define __DRV_OSAPI_H__

#ifdef __cplusplus
extern "C"{
#endif


//////////////////////////////////////////////////////////////////////////////
//
//							Debug Message
//

#define	EN_DEBUG_MSG	
//#define	EN_FUNC_DBG
#ifndef NX_DTAG
#define	NX_DTAG			" "
#endif

#if ( defined(__linux) || defined(__LINUX__) || defined(linux) || defined(ANDROID) )
	#if defined(__KERNEL__)
		#include <linux/kernel.h>
		#include <linux/mutex.h>
		#define	nx_printf		printk
	#endif	//	__KERNEL__
#else	
	void nx_printf( char *fmt,... );
#endif	//	__linux || linux || ANDROID


#ifdef EN_DEBUG_MSG
	#define	NX_DbgMsg( COND, MSG )	do{ if(COND){ nx_printf(NX_DTAG); nx_printf MSG; } }while(0)
#else
	#define	NX_DbgMsg( COND, MSG )	do{}while(0)
#endif

#ifdef EN_FUNC_DBG
	#define	FUNCIN()				nx_printf(NX_DTAG "%s() %d IN.\n", __func__, __LINE__)
	#define	FUNCOUT()				nx_printf(NX_DTAG "%s() %d OUT.\n", __func__, __LINE__)
#else
	#define	FUNCIN()				do{}while(0)
	#define	FUNCOUT()				do{}while(0)
#endif

#define	NX_RelMsg( COND, MSG )	do{ if(COND){ nx_printf(NX_DTAG); nx_printf MSG; } }while(0)
#define NX_ErrMsg( MSG )		do										\
								{										\
									nx_printf("%s%s(%d) : ",			\
										NX_DTAG, __FILE__, __LINE__);	\
									nx_printf MSG;						\
								}while(0)


//
//
//
//////////////////////////////////////////////////////////////////////////////



	
//////////////////////////////////////////////////////////////////////////////
//
//				Memory Allocation ( Normal Memory Allocation )
//

void	*NX_DrvMalloc( int size );
void	NX_DrvFree( void *ptr );
void	NX_DrvMemset( void *ptr, int value, int size );
void	NX_DrvMemcpy( void *dst, void *src, int size );

//
//
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//				Physically Linear Memory Allocation
//
void	NX_LinearAlloc( void *handle, unsigned int *phyAddr, unsigned int *virAddr, int size, int align );
void	NX_LinearFree( void *handle, unsigned int phyAddr, unsigned int virAddr );
//
//
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//				Driver Layer Mutex
//


#if ( defined(__linux) || defined(__LINUX__) || defined(linux) || defined(ANDROID) )
	#if defined(__KERNEL__)
		typedef struct tagLinuxMutex	*NX_MutexHandle;
		typedef struct tagLinuxMutex{
			struct mutex			linux_mutex;
			struct lock_class_key	key;
		}LinuxMutex;
	#endif	//	__KERNEL__
#else	
	typedef	void *	NX_MutexHandle;
	void nx_printf( char *fmt,... );
#endif	//	__linux || linux || ANDROID


void	DrvInitMutex( NX_MutexHandle handle, const char *name );
void	DrvCloseMutex( NX_MutexHandle handle );
void	DrvMutexLock( NX_MutexHandle handle );
void	DrvMutexUnlock( NX_MutexHandle handle );

//
//
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//				Sleep & Delay
//

void DrvMSleep( unsigned int mSeconds );
void DrvUSleep( unsigned int uSeconds );

//
//
//
//////////////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
};
#endif

#endif	//	__DRV_OSAPI_H__

