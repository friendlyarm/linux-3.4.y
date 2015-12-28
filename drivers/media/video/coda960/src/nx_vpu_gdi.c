//
//	Module : Nexell VPU Device Driver for CNM Coda960
//	Description : Nexell VPU Set Internal SRAM Setting & Display Memory Setting API
//	Author : SeongO-Park ( ray@nexell.co.kr )
//

#include "vpu_hw_interface.h"			//	Register Access
#include "nx_vpu_gdi.h"

#define	NX_DTAG		"[DRV|VDI_API]"
#include "../include/drv_osapi.h"


#define XY2CONFIG(A,B,C,D,E,F,G,H,I) ((A)<<20 | (B)<<19 | (C)<<18 | (D)<<17 | (E)<<16 | (F)<<12 | (G)<<8 | (H)<<4 | (I))
#define XY2(A,B,C,D)                 ((A)<<12 | (B)<<8 | (C)<<4 | (D))
#define XY2BANK(A,B,C,D,E,F)         ((A)<<13 | (B)<<12 | (C)<<8 | (D)<<5 | (E)<<4 | (F))
#define RBC(A,B,C,D)                 ((A)<<10 | (B)<< 6 | (C)<<4 | (D))
#define RBC_SAME(A,B)                ((A)<<10 | (B)<< 6 | (A)<<4 | (B))

#define	NUM_MB_720	( (1280/16) * ( 720/16) )
#define	NUM_MB_1080	( (1920/16) * (1088/16) )
#define NUM_MB_SD	( ( 720/16) * ( 576/16) )


typedef struct {
    int  xy2ca_map[16];
    int  xy2ba_map[16];
    int  xy2ra_map[16];
    int  rbc2axi_map[32];
    int  MapType;

    int  xy2rbc_config;
    int  tb_separate_map;
    int  top_bot_split;
    int  tiledMap;
    int  ca_inc_hor;
    int  val;
} GdiTiledMap;

enum {
    CA_SEL  = 0,
	BA_SEL  = 1,
	RA_SEL  = 2,
	Z_SEL   = 3,
};

enum {
	X_SEL   = 0,
	Y_SEL   = 1,
};

#define RBC(A,B,C,D)	((A)<<10 | (B)<< 6 | (C)<<4 | (D))

#if 0
int SetTiledMapType(int tiledMapType, int width, int interleave)
{
	int luma_map;
	int chro_map;
	int i;

	sTiledInfo.MapType = tiledMapType;
	//         inv = 1'b0, zero = 1'b1 , tbxor = 1'b0, xy = 1'b0, bit = 4'd0
	luma_map = 64;
	chro_map = 64;

	for (i=0; i<16 ; i=i+1) {
		sTiledInfo.xy2ca_map[i] = luma_map << 8 | chro_map;
	}

	for (i=0; i<4;  i=i+1) {
		sTiledInfo.xy2ba_map[i] = luma_map << 8 | chro_map;
	}

	for (i=0; i<16; i=i+1) {
		sTiledInfo.xy2ra_map[i] = luma_map << 8 | chro_map;
	}

	if (tiledMapType == VPU_LINEAR_FRAME_MAP)
	{
		sTiledInfo.xy2rbc_config = 0;
	}
	else if (tiledMapType == VPU_TILED_FRAME_V_MAP)
	{
		//cas
		sTiledInfo.xy2ca_map[0] = XY2(Y_SEL, 0, Y_SEL, 0);
		sTiledInfo.xy2ca_map[1] = XY2(Y_SEL, 1, Y_SEL, 1);
		sTiledInfo.xy2ca_map[2] = XY2(Y_SEL, 2, Y_SEL, 2);
		sTiledInfo.xy2ca_map[3] = XY2(Y_SEL, 3, Y_SEL, 3);
		sTiledInfo.xy2ca_map[4] = XY2(X_SEL, 3, X_SEL, 3);
		sTiledInfo.xy2ca_map[5] = XY2(X_SEL, 4, X_SEL, 4);
		sTiledInfo.xy2ca_map[6] = XY2(X_SEL, 5, X_SEL, 5);
		sTiledInfo.xy2ca_map[7] = XY2(X_SEL, 6, X_SEL, 6);
		sTiledInfo.xy2ca_map[8] = XY2(Y_SEL, 4, Y_SEL, 5);

		//bank
		sTiledInfo.xy2ba_map[0] = XY2BANK(0,X_SEL, 7, 4, X_SEL, 7);
		sTiledInfo.xy2ba_map[1] = XY2BANK(0,Y_SEL, 5, 4, Y_SEL, 4);

		//ras
		sTiledInfo.xy2ra_map[ 0] = XY2(X_SEL, 8, X_SEL, 8);
		sTiledInfo.xy2ra_map[ 1] = XY2(X_SEL, 9, X_SEL, 9);
		sTiledInfo.xy2ra_map[ 2] = XY2(X_SEL,10, X_SEL,10);
		sTiledInfo.xy2ra_map[ 3] = XY2(Y_SEL, 6, Y_SEL, 6);
		sTiledInfo.xy2ra_map[ 4] = XY2(Y_SEL, 7, Y_SEL, 7);
		sTiledInfo.xy2ra_map[ 5] = XY2(Y_SEL, 8, Y_SEL, 8);
		sTiledInfo.xy2ra_map[ 6] = XY2(Y_SEL, 9, Y_SEL, 9);
		sTiledInfo.xy2ra_map[ 7] = XY2(Y_SEL,10, Y_SEL,10);
		sTiledInfo.xy2ra_map[ 8] = XY2(Y_SEL,11, Y_SEL,11);
		sTiledInfo.xy2ra_map[ 9] = XY2(Y_SEL,12, Y_SEL,12);
		sTiledInfo.xy2ra_map[10] = XY2(Y_SEL,13, Y_SEL,13);
		sTiledInfo.xy2ra_map[11] = XY2(Y_SEL,14, Y_SEL,14);
		sTiledInfo.xy2ra_map[12] = XY2(Y_SEL,15, Y_SEL,15);

		//xy2rbc_config
		sTiledInfo.xy2rbc_config = XY2CONFIG(0,0,0,1,1,15,0,15,0);

	}
	else if (tiledMapType == VPU_TILED_FRAME_H_MAP)
	{
		//cas
		sTiledInfo.xy2ca_map[0] = XY2(X_SEL, 3, X_SEL, 3);
		sTiledInfo.xy2ca_map[1] = XY2(X_SEL, 4, X_SEL, 4);
		sTiledInfo.xy2ca_map[2] = XY2(X_SEL, 5, X_SEL, 5);
		sTiledInfo.xy2ca_map[3] = XY2(X_SEL, 6, X_SEL, 6);
		sTiledInfo.xy2ca_map[4] = XY2(Y_SEL, 0, Y_SEL, 0);
		sTiledInfo.xy2ca_map[5] = XY2(Y_SEL, 1, Y_SEL, 1);
		sTiledInfo.xy2ca_map[6] = XY2(Y_SEL, 2, Y_SEL, 2);
		sTiledInfo.xy2ca_map[7] = XY2(Y_SEL, 3, Y_SEL, 3);
		sTiledInfo.xy2ca_map[8] = XY2(Y_SEL, 4, Y_SEL, 5);

		//bank
		sTiledInfo.xy2ba_map[0] = XY2BANK(0,X_SEL, 7, 4,X_SEL, 7);
		sTiledInfo.xy2ba_map[1] = XY2BANK(0,Y_SEL, 5, 4,Y_SEL, 4);

		//ras
		sTiledInfo.xy2ra_map[ 0] = XY2(X_SEL, 8, X_SEL, 8);
		sTiledInfo.xy2ra_map[ 1] = XY2(X_SEL, 9, X_SEL, 9);
		sTiledInfo.xy2ra_map[ 2] = XY2(X_SEL,10, X_SEL,10);
		sTiledInfo.xy2ra_map[ 3] = XY2(Y_SEL, 6, Y_SEL, 6);
		sTiledInfo.xy2ra_map[ 4] = XY2(Y_SEL, 7, Y_SEL, 7);
		sTiledInfo.xy2ra_map[ 5] = XY2(Y_SEL, 8, Y_SEL, 8);
		sTiledInfo.xy2ra_map[ 6] = XY2(Y_SEL, 9, Y_SEL, 9);
		sTiledInfo.xy2ra_map[ 7] = XY2(Y_SEL,10, Y_SEL,10);
		sTiledInfo.xy2ra_map[ 8] = XY2(Y_SEL,11, Y_SEL,11);
		sTiledInfo.xy2ra_map[ 9] = XY2(Y_SEL,12, Y_SEL,12);
		sTiledInfo.xy2ra_map[10] = XY2(Y_SEL,13, Y_SEL,13);
		sTiledInfo.xy2ra_map[11] = XY2(Y_SEL,14, Y_SEL,14);
		sTiledInfo.xy2ra_map[12] = XY2(Y_SEL,15, Y_SEL,15);
		//xy2rbc_config
		sTiledInfo.xy2rbc_config = XY2CONFIG(0,0,0,1,0,15,15,15,15);

	}
	else if (tiledMapType == VPU_TILED_FIELD_V_MAP)
	{
		//cas
		sTiledInfo.xy2ca_map[0] = XY2(Y_SEL, 0, Y_SEL, 0);
		sTiledInfo.xy2ca_map[1] = XY2(Y_SEL, 1, Y_SEL, 1);
		sTiledInfo.xy2ca_map[2] = XY2(Y_SEL, 2, Y_SEL, 2);
		sTiledInfo.xy2ca_map[3] = XY2(Y_SEL, 3, Y_SEL, 3);
		sTiledInfo.xy2ca_map[4] = XY2(X_SEL, 3, X_SEL, 3);
		sTiledInfo.xy2ca_map[5] = XY2(X_SEL, 4, X_SEL, 4);
		sTiledInfo.xy2ca_map[6] = XY2(X_SEL, 5, X_SEL, 5);
		sTiledInfo.xy2ca_map[7] = XY2(X_SEL, 6, X_SEL, 6);
		sTiledInfo.xy2ca_map[8] = XY2(Y_SEL, 4, Y_SEL, 5);

		//bank
		sTiledInfo.xy2ba_map[0] = XY2BANK(0,X_SEL, 7, 4,X_SEL, 7);
		sTiledInfo.xy2ba_map[1] = XY2BANK(1,Y_SEL, 5, 5,Y_SEL, 4);

		//ras
		sTiledInfo.xy2ra_map[ 0] = XY2(X_SEL, 8, X_SEL, 8);
		sTiledInfo.xy2ra_map[ 1] = XY2(X_SEL, 9, X_SEL, 9);
		sTiledInfo.xy2ra_map[ 2] = XY2(X_SEL,10, X_SEL,10);
		sTiledInfo.xy2ra_map[ 3] = XY2(Y_SEL, 6, Y_SEL, 6);
		sTiledInfo.xy2ra_map[ 4] = XY2(Y_SEL, 7, Y_SEL, 7);
		sTiledInfo.xy2ra_map[ 5] = XY2(Y_SEL, 8, Y_SEL, 8);
		sTiledInfo.xy2ra_map[ 6] = XY2(Y_SEL, 9, Y_SEL, 9);
		sTiledInfo.xy2ra_map[ 7] = XY2(Y_SEL,10, Y_SEL,10);
		sTiledInfo.xy2ra_map[ 8] = XY2(Y_SEL,11, Y_SEL,11);
		sTiledInfo.xy2ra_map[ 9] = XY2(Y_SEL,12, Y_SEL,12);
		sTiledInfo.xy2ra_map[10] = XY2(Y_SEL,13, Y_SEL,13);
		sTiledInfo.xy2ra_map[11] = XY2(Y_SEL,14, Y_SEL,14);
		sTiledInfo.xy2ra_map[12] = XY2(Y_SEL,15, Y_SEL,15);

		//xy2rbc_config
		sTiledInfo.xy2rbc_config = XY2CONFIG(0,1,1,1,1,15,15,15,15);
	}
	else if (tiledMapType == VPU_TILED_MIXED_V_MAP)
	{
		//cas
		sTiledInfo.xy2ca_map[0] = XY2(Y_SEL, 1, Y_SEL, 1);
		sTiledInfo.xy2ca_map[1] = XY2(Y_SEL, 2, Y_SEL, 2);
		sTiledInfo.xy2ca_map[2] = XY2(Y_SEL, 3, Y_SEL, 3);
		sTiledInfo.xy2ca_map[3] = XY2(Y_SEL, 0, Y_SEL, 0);
		sTiledInfo.xy2ca_map[4] = XY2(X_SEL, 3, X_SEL, 3);
		sTiledInfo.xy2ca_map[5] = XY2(X_SEL, 4, X_SEL, 4);
		sTiledInfo.xy2ca_map[6] = XY2(X_SEL, 5, X_SEL, 5);
		sTiledInfo.xy2ca_map[7] = XY2(X_SEL, 6, X_SEL, 6);
		sTiledInfo.xy2ca_map[8] = XY2(Y_SEL, 4, Y_SEL, 5);

		//bank
		sTiledInfo.xy2ba_map[0] = XY2BANK(0,X_SEL, 7, 4,X_SEL, 7);
		sTiledInfo.xy2ba_map[1] = XY2BANK(0,Y_SEL, 5, 4,Y_SEL, 4);

		//ras
		sTiledInfo.xy2ra_map[ 0] = XY2(X_SEL, 8, X_SEL, 8);
		sTiledInfo.xy2ra_map[ 1] = XY2(X_SEL, 9, X_SEL, 9);
		sTiledInfo.xy2ra_map[ 2] = XY2(X_SEL,10, X_SEL,10);
		sTiledInfo.xy2ra_map[ 3] = XY2(Y_SEL, 6, Y_SEL, 6);
		sTiledInfo.xy2ra_map[ 4] = XY2(Y_SEL, 7, Y_SEL, 7);
		sTiledInfo.xy2ra_map[ 5] = XY2(Y_SEL, 8, Y_SEL, 8);
		sTiledInfo.xy2ra_map[ 6] = XY2(Y_SEL, 9, Y_SEL, 9);
		sTiledInfo.xy2ra_map[ 7] = XY2(Y_SEL,10, Y_SEL,10);
		sTiledInfo.xy2ra_map[ 8] = XY2(Y_SEL,11, Y_SEL,11);
		sTiledInfo.xy2ra_map[ 9] = XY2(Y_SEL,12, Y_SEL,12);
		sTiledInfo.xy2ra_map[10] = XY2(Y_SEL,13, Y_SEL,13);
		sTiledInfo.xy2ra_map[11] = XY2(Y_SEL,14, Y_SEL,14);
		sTiledInfo.xy2ra_map[12] = XY2(Y_SEL,15, Y_SEL,15);
		//xy2rbc_config
		sTiledInfo.xy2rbc_config = XY2CONFIG(0,0,1,1,1,7,7,7,7);
	}
	else if (tiledMapType == VPU_TILED_FRAME_MB_RASTER_MAP) {
		//cas
		sTiledInfo.xy2ca_map[0] = XY2(Y_SEL, 0, Y_SEL, 0);
		sTiledInfo.xy2ca_map[1] = XY2(Y_SEL, 1, Y_SEL, 1);
		sTiledInfo.xy2ca_map[2] = XY2(Y_SEL, 2, Y_SEL, 2);
		sTiledInfo.xy2ca_map[3] = XY2(Y_SEL, 3, X_SEL, 3);
		sTiledInfo.xy2ca_map[4] = XY2(X_SEL, 3, 4    , 0);

		//xy2rbc_config
		sTiledInfo.xy2rbc_config = XY2CONFIG(0,0,0,1,1,15,0,7,0);

	} else if (tiledMapType == VPU_TILED_FIELD_MB_RASTER_MAP) {
		//cas
		sTiledInfo.xy2ca_map[0] = XY2(Y_SEL, 0, Y_SEL, 0);
		sTiledInfo.xy2ca_map[1] = XY2(Y_SEL, 1, Y_SEL, 1);
		sTiledInfo.xy2ca_map[2] = XY2(Y_SEL, 2, X_SEL, 3);
		sTiledInfo.xy2ca_map[3] = XY2(X_SEL, 3, 4    , 0);

		//xy2rbc_config
		sTiledInfo.xy2rbc_config = XY2CONFIG(0,1,1,1,1,7,7,3,3);
	}
	else
	{
		NX_ErrMsg(("tiledMapType is %d >-- Error\n",tiledMapType));
		return 0;
	}

	if (tiledMapType == VPU_TILED_FRAME_MB_RASTER_MAP)
	{
		sTiledInfo.rbc2axi_map[ 0] = RBC( Z_SEL, 0,  Z_SEL, 0);
		sTiledInfo.rbc2axi_map[ 1] = RBC( Z_SEL, 0,  Z_SEL, 0);
		sTiledInfo.rbc2axi_map[ 2] = RBC( Z_SEL, 0,  Z_SEL, 0);
		sTiledInfo.rbc2axi_map[ 3] = RBC(CA_SEL, 0, CA_SEL, 0);
		sTiledInfo.rbc2axi_map[ 4] = RBC(CA_SEL, 1, CA_SEL, 1);
		sTiledInfo.rbc2axi_map[ 5] = RBC(CA_SEL, 2, CA_SEL, 2);
		sTiledInfo.rbc2axi_map[ 6] = RBC(CA_SEL, 3, CA_SEL, 3);
		sTiledInfo.rbc2axi_map[ 7] = RBC(CA_SEL, 4, CA_SEL, 8);
		sTiledInfo.rbc2axi_map[ 8] = RBC(CA_SEL, 8, CA_SEL, 9);
		sTiledInfo.rbc2axi_map[ 9] = RBC(CA_SEL, 9, CA_SEL,10);
		sTiledInfo.rbc2axi_map[10] = RBC(CA_SEL,10, CA_SEL,11);
		sTiledInfo.rbc2axi_map[11] = RBC(CA_SEL,11, CA_SEL,12);
		sTiledInfo.rbc2axi_map[12] = RBC(CA_SEL,12, CA_SEL,13);
		sTiledInfo.rbc2axi_map[13] = RBC(CA_SEL,13, CA_SEL,14);
		sTiledInfo.rbc2axi_map[14] = RBC(CA_SEL,14, CA_SEL,15);
		sTiledInfo.rbc2axi_map[15] = RBC(CA_SEL,15, RA_SEL, 0);
		sTiledInfo.rbc2axi_map[16] = RBC(RA_SEL, 0, RA_SEL, 1);
		sTiledInfo.rbc2axi_map[17] = RBC(RA_SEL, 1, RA_SEL, 2);
		sTiledInfo.rbc2axi_map[18] = RBC(RA_SEL, 2, RA_SEL, 3);
		sTiledInfo.rbc2axi_map[19] = RBC(RA_SEL, 3, RA_SEL, 4);
		sTiledInfo.rbc2axi_map[20] = RBC(RA_SEL, 4, RA_SEL, 5);
		sTiledInfo.rbc2axi_map[21] = RBC(RA_SEL, 5, RA_SEL, 6);
		sTiledInfo.rbc2axi_map[22] = RBC(RA_SEL, 6, RA_SEL, 7);
		sTiledInfo.rbc2axi_map[23] = RBC(RA_SEL, 7, RA_SEL, 8);
		sTiledInfo.rbc2axi_map[24] = RBC(RA_SEL, 8, RA_SEL, 9);
		sTiledInfo.rbc2axi_map[25] = RBC(RA_SEL, 9, RA_SEL,10);
		sTiledInfo.rbc2axi_map[26] = RBC(RA_SEL,10, RA_SEL,11);
		sTiledInfo.rbc2axi_map[27] = RBC(RA_SEL,11, RA_SEL,12);
		sTiledInfo.rbc2axi_map[28] = RBC(RA_SEL,12, RA_SEL,13);
		sTiledInfo.rbc2axi_map[29] = RBC(RA_SEL,13, RA_SEL,14);
		sTiledInfo.rbc2axi_map[30] = RBC(RA_SEL,14, RA_SEL,15);
		sTiledInfo.rbc2axi_map[31] = RBC(RA_SEL,15,  Z_SEL, 0);
	}
	else if (tiledMapType == VPU_TILED_FIELD_MB_RASTER_MAP)
	{
		sTiledInfo.rbc2axi_map[ 0] = RBC(Z_SEL ,0  ,Z_SEL , 0);
		sTiledInfo.rbc2axi_map[ 1] = RBC(Z_SEL ,0  ,Z_SEL , 0);
		sTiledInfo.rbc2axi_map[ 2] = RBC(Z_SEL ,0  ,Z_SEL , 0);
		sTiledInfo.rbc2axi_map[ 3] = RBC(CA_SEL,0  ,CA_SEL, 0);
		sTiledInfo.rbc2axi_map[ 4] = RBC(CA_SEL,1  ,CA_SEL, 1);
		sTiledInfo.rbc2axi_map[ 5] = RBC(CA_SEL,2  ,CA_SEL, 2);
		sTiledInfo.rbc2axi_map[ 6] = RBC(CA_SEL,3  ,CA_SEL, 8);
		sTiledInfo.rbc2axi_map[ 7] = RBC(CA_SEL,8,  CA_SEL, 9);
		sTiledInfo.rbc2axi_map[ 8] = RBC(CA_SEL,9,  CA_SEL,10);
		sTiledInfo.rbc2axi_map[ 9] = RBC(CA_SEL,10 ,CA_SEL,11);
		sTiledInfo.rbc2axi_map[10] = RBC(CA_SEL,11 ,CA_SEL,12);
		sTiledInfo.rbc2axi_map[11] = RBC(CA_SEL,12 ,CA_SEL,13);
		sTiledInfo.rbc2axi_map[12] = RBC(CA_SEL,13 ,CA_SEL,14);
		sTiledInfo.rbc2axi_map[13] = RBC(CA_SEL,14 ,CA_SEL,15);
		sTiledInfo.rbc2axi_map[14] = RBC(CA_SEL,15 ,RA_SEL, 0);

		sTiledInfo.rbc2axi_map[15] = RBC(RA_SEL,0  ,RA_SEL, 1);
		sTiledInfo.rbc2axi_map[16] = RBC(RA_SEL,1  ,RA_SEL, 2);
		sTiledInfo.rbc2axi_map[17] = RBC(RA_SEL,2  ,RA_SEL, 3);
		sTiledInfo.rbc2axi_map[18] = RBC(RA_SEL,3  ,RA_SEL, 4);
		sTiledInfo.rbc2axi_map[19] = RBC(RA_SEL,4  ,RA_SEL, 5);
		sTiledInfo.rbc2axi_map[20] = RBC(RA_SEL,5  ,RA_SEL, 6);
		sTiledInfo.rbc2axi_map[21] = RBC(RA_SEL,6  ,RA_SEL, 7);
		sTiledInfo.rbc2axi_map[22] = RBC(RA_SEL,7  ,RA_SEL, 8);
		sTiledInfo.rbc2axi_map[23] = RBC(RA_SEL,8  ,RA_SEL, 9);
		sTiledInfo.rbc2axi_map[24] = RBC(RA_SEL,9  ,RA_SEL,10);
		sTiledInfo.rbc2axi_map[25] = RBC(RA_SEL,10 ,RA_SEL,11);
		sTiledInfo.rbc2axi_map[26] = RBC(RA_SEL,11 ,RA_SEL,12);
		sTiledInfo.rbc2axi_map[27] = RBC(RA_SEL,12 ,RA_SEL,13);
		sTiledInfo.rbc2axi_map[28] = RBC(RA_SEL,13 ,RA_SEL,14);
		sTiledInfo.rbc2axi_map[29] = RBC(RA_SEL,14 ,RA_SEL,15);
		sTiledInfo.rbc2axi_map[30] = RBC(RA_SEL,15 , Z_SEL, 0);
		sTiledInfo.rbc2axi_map[31] = RBC(Z_SEL , 0 , Z_SEL, 0);
	}
	else
	{
		sTiledInfo.rbc2axi_map[ 0] = RBC(Z_SEL,0, Z_SEL,0);
		sTiledInfo.rbc2axi_map[ 1] = RBC(Z_SEL,0, Z_SEL,0);
		sTiledInfo.rbc2axi_map[ 2] = RBC(Z_SEL,0, Z_SEL,0);
		sTiledInfo.rbc2axi_map[ 3] = RBC(CA_SEL,0,CA_SEL,0);
		sTiledInfo.rbc2axi_map[ 4] = RBC(CA_SEL,1,CA_SEL,1);
		sTiledInfo.rbc2axi_map[ 5] = RBC(CA_SEL,2,CA_SEL,2);
		sTiledInfo.rbc2axi_map[ 6] = RBC(CA_SEL,3,CA_SEL,3);
		sTiledInfo.rbc2axi_map[ 7] = RBC(CA_SEL,4,CA_SEL,4);
		sTiledInfo.rbc2axi_map[ 8] = RBC(CA_SEL,5,CA_SEL,5);
		sTiledInfo.rbc2axi_map[ 9] = RBC(CA_SEL,6,CA_SEL,6);
		sTiledInfo.rbc2axi_map[10] = RBC(CA_SEL,7,CA_SEL,7);
		sTiledInfo.rbc2axi_map[11] = RBC(CA_SEL,8,CA_SEL,8);

		sTiledInfo.rbc2axi_map[12] = RBC(BA_SEL,0, BA_SEL,0);
		sTiledInfo.rbc2axi_map[13] = RBC(BA_SEL,1, BA_SEL,1);

		sTiledInfo.rbc2axi_map[14] = RBC(RA_SEL,0, RA_SEL, 0);
		sTiledInfo.rbc2axi_map[15] = RBC(RA_SEL,1, RA_SEL, 1);
		sTiledInfo.rbc2axi_map[16] = RBC(RA_SEL,2 ,RA_SEL, 2);
		sTiledInfo.rbc2axi_map[17] = RBC(RA_SEL,3 ,RA_SEL, 3);
		sTiledInfo.rbc2axi_map[18] = RBC(RA_SEL,4 ,RA_SEL, 4);
		sTiledInfo.rbc2axi_map[19] = RBC(RA_SEL,5 ,RA_SEL, 5);
		sTiledInfo.rbc2axi_map[20] = RBC(RA_SEL,6 ,RA_SEL, 6);
		sTiledInfo.rbc2axi_map[21] = RBC(RA_SEL,7 ,RA_SEL, 7);
		sTiledInfo.rbc2axi_map[22] = RBC(RA_SEL,8 ,RA_SEL, 8);
		sTiledInfo.rbc2axi_map[23] = RBC(RA_SEL,9 ,RA_SEL, 9);
		sTiledInfo.rbc2axi_map[24] = RBC(RA_SEL,10,RA_SEL,10);
		sTiledInfo.rbc2axi_map[25] = RBC(RA_SEL,11,RA_SEL,11);
		sTiledInfo.rbc2axi_map[26] = RBC(RA_SEL,12,RA_SEL,12);
		sTiledInfo.rbc2axi_map[27] = RBC(RA_SEL,13,RA_SEL,13);
		sTiledInfo.rbc2axi_map[28] = RBC(RA_SEL,14,RA_SEL,14);
		sTiledInfo.rbc2axi_map[29] = RBC(RA_SEL,15,RA_SEL,15);

		sTiledInfo.rbc2axi_map[30] = RBC(Z_SEL , 0, Z_SEL, 0);
		sTiledInfo.rbc2axi_map[31] = RBC(Z_SEL , 0, Z_SEL, 0);
	}

	for (i=0; i<16; i++) { //xy2ca_map
		VpuWriteReg(GDI_XY2_CAS_0 + 4*i, sTiledInfo.xy2ca_map[i]);
	}

	for (i=0; i<4; i++) { //xy2ba_map
		VpuWriteReg(GDI_XY2_BA_0  + 4*i, sTiledInfo.xy2ba_map[i]);
	}

	for (i=0; i<16; i++) { //xy2ra_map
		VpuWriteReg(GDI_XY2_RAS_0 + 4*i, sTiledInfo.xy2ra_map[i]);
	}

	//xy2rbc_config
	VpuWriteReg(GDI_XY2_RBC_CONFIG,sTiledInfo.xy2rbc_config);

	//// fast access for reading
	sTiledInfo.tb_separate_map  = (sTiledInfo.xy2rbc_config >> 19) & 0x1;
	sTiledInfo.top_bot_split    = (sTiledInfo.xy2rbc_config >> 18) & 0x1;
	sTiledInfo.tiledMap			= (sTiledInfo.xy2rbc_config >> 17) & 0x1;
	sTiledInfo.ca_inc_hor       = (sTiledInfo.xy2rbc_config >> 16) & 0x1;

	// RAS, BA, CAS -> Axi Addr
	for (i=0; i<32; i++)
	{
		VpuWriteReg(GDI_RBC2_AXI_0 + 4*i ,sTiledInfo.rbc2axi_map[i]);
	}
	return 1;
}
#else

typedef struct {
	int  xy2caMap[16];
	int  xy2baMap[16];
	int  xy2raMap[16];
	int  rbc2axiMap[32];
	int  mapType;
	int  xy2rbcConfig;

	int  tiledBaseAddr;
	int  tbSeparateMap;
	int  topBotSplit;
	int  tiledMap;
	int  caIncHor;
	int  convLinear;
} TiledMapConfig;


typedef struct {
	int  rasBit;
	int  casBit;
	int  bankBit;
	int  busBit;
} DRAMConfig;


// DRAM configuration for TileMap access
#define EM_RAS 13
#define EM_BANK  3
#define EM_CAS   10
#define EM_WIDTH 2


int SetTiledMapType(int mapType, int stride, int interleave)
{
	int ret;
	int luma_map;
	int chro_map;
	int i;
	TiledMapConfig mapCfg;
	DRAMConfig dramCfg;

	dramCfg.rasBit = EM_RAS;
	dramCfg.casBit = EM_CAS;
	dramCfg.bankBit = EM_BANK;
	dramCfg.busBit = EM_WIDTH;

	NX_DrvMemset( &mapCfg, 0, sizeof(mapCfg) );

	mapCfg.mapType = mapType;
	mapCfg.xy2rbcConfig = 0;

	//         inv = 1'b0, zero = 1'b1 , tbxor = 1'b0, xy = 1'b0, bit = 4'd0
	luma_map = 64;
	chro_map = 64;

	for (i=0; i<16 ; i=i+1) {
		mapCfg.xy2caMap[i] = luma_map << 8 | chro_map;
	}

	for (i=0; i<4;  i=i+1) {
		mapCfg.xy2baMap[i] = luma_map << 8 | chro_map;
	}

	for (i=0; i<16; i=i+1) {
		mapCfg.xy2raMap[i] = luma_map << 8 | chro_map;
	}

	ret = stride; // this will be removed after map size optimizing.
	ret = 0;

	if (dramCfg.casBit == 9 && dramCfg.bankBit == 2 && dramCfg.rasBit == 13)
	{
		mapCfg.rbc2axiMap[ 0] = RBC(Z_SEL,0, Z_SEL,0);
		mapCfg.rbc2axiMap[ 1] = RBC(Z_SEL,0, Z_SEL,0);
		mapCfg.rbc2axiMap[ 2] = RBC(Z_SEL,0, Z_SEL,0);
		mapCfg.rbc2axiMap[ 3] = RBC(CA_SEL,0,CA_SEL,0);
		mapCfg.rbc2axiMap[ 4] = RBC(CA_SEL,1,CA_SEL,1);
		mapCfg.rbc2axiMap[ 5] = RBC(CA_SEL,2,CA_SEL,2);
		mapCfg.rbc2axiMap[ 6] = RBC(CA_SEL,3,CA_SEL,3);
		mapCfg.rbc2axiMap[ 7] = RBC(CA_SEL,4,CA_SEL,4);
		mapCfg.rbc2axiMap[ 8] = RBC(CA_SEL,5,CA_SEL,5);
		mapCfg.rbc2axiMap[ 9] = RBC(CA_SEL,6,CA_SEL,6);
		mapCfg.rbc2axiMap[10] = RBC(CA_SEL,7,CA_SEL,7);
		mapCfg.rbc2axiMap[11] = RBC(CA_SEL,8,CA_SEL,8);

		mapCfg.rbc2axiMap[12] = RBC(BA_SEL,0, BA_SEL,0);
		mapCfg.rbc2axiMap[13] = RBC(BA_SEL,1, BA_SEL,1);

		mapCfg.rbc2axiMap[14] = RBC(RA_SEL,0, RA_SEL, 0);
		mapCfg.rbc2axiMap[15] = RBC(RA_SEL,1, RA_SEL, 1);
		mapCfg.rbc2axiMap[16] = RBC(RA_SEL,2 ,RA_SEL, 2);
		mapCfg.rbc2axiMap[17] = RBC(RA_SEL,3 ,RA_SEL, 3);
		mapCfg.rbc2axiMap[18] = RBC(RA_SEL,4 ,RA_SEL, 4);
		mapCfg.rbc2axiMap[19] = RBC(RA_SEL,5 ,RA_SEL, 5);
		mapCfg.rbc2axiMap[20] = RBC(RA_SEL,6 ,RA_SEL, 6);
		mapCfg.rbc2axiMap[21] = RBC(RA_SEL,7 ,RA_SEL, 7);
		mapCfg.rbc2axiMap[22] = RBC(RA_SEL,8 ,RA_SEL, 8);
		mapCfg.rbc2axiMap[23] = RBC(RA_SEL,9 ,RA_SEL, 9);
		mapCfg.rbc2axiMap[24] = RBC(RA_SEL,10,RA_SEL,10);
		mapCfg.rbc2axiMap[25] = RBC(RA_SEL,11,RA_SEL,11);
		mapCfg.rbc2axiMap[26] = RBC(RA_SEL,12,RA_SEL,12);
		mapCfg.rbc2axiMap[27] = RBC(RA_SEL,13,RA_SEL,13);
		mapCfg.rbc2axiMap[28] = RBC(RA_SEL,14,RA_SEL,14);
		mapCfg.rbc2axiMap[29] = RBC(RA_SEL,15,RA_SEL,15);
		mapCfg.rbc2axiMap[30] = RBC(Z_SEL , 0, Z_SEL, 0);
		mapCfg.rbc2axiMap[31] = RBC(Z_SEL , 0, Z_SEL, 0);
		ret = 1;
	}
	else if(dramCfg.casBit == 10 && dramCfg.bankBit == 3 && dramCfg.rasBit == 13)
	{
		mapCfg.rbc2axiMap[ 0] = RBC(Z_SEL, 0, Z_SEL,0);
		mapCfg.rbc2axiMap[ 1] = RBC(Z_SEL, 0, Z_SEL,0);
		mapCfg.rbc2axiMap[ 2] = RBC(CA_SEL,0,CA_SEL,0);
		mapCfg.rbc2axiMap[ 3] = RBC(CA_SEL,1,CA_SEL,1);
		mapCfg.rbc2axiMap[ 4] = RBC(CA_SEL,2,CA_SEL,2);
		mapCfg.rbc2axiMap[ 5] = RBC(CA_SEL,3,CA_SEL,3);
		mapCfg.rbc2axiMap[ 6] = RBC(CA_SEL,4,CA_SEL,4);
		mapCfg.rbc2axiMap[ 7] = RBC(CA_SEL,5,CA_SEL,5);
		mapCfg.rbc2axiMap[ 8] = RBC(CA_SEL,6,CA_SEL,6);
		mapCfg.rbc2axiMap[ 9] = RBC(CA_SEL,7,CA_SEL,7);
		mapCfg.rbc2axiMap[10] = RBC(CA_SEL,8,CA_SEL,8);
		mapCfg.rbc2axiMap[11] = RBC(CA_SEL,9,CA_SEL,9);

		mapCfg.rbc2axiMap[12] = RBC(BA_SEL,0, BA_SEL,0);
		mapCfg.rbc2axiMap[13] = RBC(BA_SEL,1, BA_SEL,1);
		mapCfg.rbc2axiMap[14] = RBC(BA_SEL,2, BA_SEL,2);

		mapCfg.rbc2axiMap[15] = RBC(RA_SEL, 0, RA_SEL, 0);
		mapCfg.rbc2axiMap[16] = RBC(RA_SEL, 1 ,RA_SEL, 1);
		mapCfg.rbc2axiMap[17] = RBC(RA_SEL, 2 ,RA_SEL, 2);
		mapCfg.rbc2axiMap[18] = RBC(RA_SEL, 3 ,RA_SEL, 3);
		mapCfg.rbc2axiMap[19] = RBC(RA_SEL, 4 ,RA_SEL, 4);
		mapCfg.rbc2axiMap[20] = RBC(RA_SEL, 5 ,RA_SEL, 5);
		mapCfg.rbc2axiMap[21] = RBC(RA_SEL, 6 ,RA_SEL, 6);
		mapCfg.rbc2axiMap[22] = RBC(RA_SEL, 7 ,RA_SEL, 7);
		mapCfg.rbc2axiMap[23] = RBC(RA_SEL, 8 ,RA_SEL, 8);
		mapCfg.rbc2axiMap[24] = RBC(RA_SEL, 9, RA_SEL, 9);
		mapCfg.rbc2axiMap[25] = RBC(RA_SEL,10, RA_SEL,10);
		mapCfg.rbc2axiMap[26] = RBC(RA_SEL,11, RA_SEL,11);
		mapCfg.rbc2axiMap[27] = RBC(RA_SEL,12, RA_SEL,12);
		mapCfg.rbc2axiMap[28] = RBC(Z_SEL , 0, Z_SEL , 0);
		mapCfg.rbc2axiMap[29] = RBC(Z_SEL , 0, Z_SEL , 0);
		mapCfg.rbc2axiMap[30] = RBC(Z_SEL , 0, Z_SEL , 0);
		mapCfg.rbc2axiMap[31] = RBC(Z_SEL , 0, Z_SEL , 0);
		ret = 1;
	}

	for (i=0; i<16; i++) { //xy2ca_map
		VpuWriteReg(GDI_XY2_CAS_0 + 4*i, mapCfg.xy2caMap[i]);
	}

	for (i=0; i<4; i++) { //xy2baMap
		VpuWriteReg(GDI_XY2_BA_0  + 4*i, mapCfg.xy2baMap[i]);
	}

	for (i=0; i<16; i++) { //xy2raMap
		VpuWriteReg(GDI_XY2_RAS_0 + 4*i, mapCfg.xy2raMap[i]);
	}

	//xy2rbcConfig
	VpuWriteReg(GDI_XY2_RBC_CONFIG,mapCfg.xy2rbcConfig);
	//// fast access for reading
	mapCfg.tbSeparateMap= (mapCfg.xy2rbcConfig >> 19) & 0x1;
	mapCfg.topBotSplit	= (mapCfg.xy2rbcConfig >> 18) & 0x1;
	mapCfg.tiledMap		= (mapCfg.xy2rbcConfig >> 17) & 0x1;
	mapCfg.caIncHor		= (mapCfg.xy2rbcConfig >> 16) & 0x1;

	// RAS, BA, CAS -> Axi Addr
	for (i=0; i<32; i++) {
		VpuWriteReg(GDI_RBC2_AXI_0 + 4*i ,mapCfg.rbc2axiMap[i]);
	}

	return ret;
}

#endif

//
//	Internal SRAM Memory Map ( for Encding )
//
//	|                  |
//	|------------------| SRAM High Address
//	|                  |
//	|  Deblock Chroma  | <-- H.263 : if( width>720 ) use external ram
//	|                  |
//	|------------------|
//	|                  |
//	|  Deblock Luma    | <-- H.263 : if( width>720 ) use external ram
//	|                  |
//	|------------------|
//	|                  |
//	|  Prediction Buf  |
//	|     (AC/DC)      |
//	|                  |
//	|------------------|
//	|                  |
//	| Intra Prediction | <-- H.264 : if( width>720 ) use external ram
//	|                  |
//	|------------------|
//	|                  |
//	|  BIT Processor   |
//	|                  |
//	|------------------| SRAM Low Address
//	|                  |
//

NX_VPU_RET ConfigEncSecAXI(int codStd, SecAxiInfo *sa, int width, int height )
{
	int offset;
	int MbNumX = ((width & 0xFFFF) + 15) / 16;
	int MbNumY = ((height & 0xFFFF) + 15) / 16;
	int totalMB = MbNumX * MbNumY;

	int sramSize = VPU_SRAM_SIZE;
	int sramPhyAddr = VPU_SRAM_PHYSICAL_BASE;

	switch( codStd )
	{
	case CODEC_STD_AVC:
		sa->useIpEnable = 1;				//	Intra Prediction
		sa->useBitEnable = 1;			//	BitAxiSecEn (USE Bit Processor)
		sa->useDbkYEnable = 0;			//	Deblocking Luminance
		sa->useDbkCEnable = 0;			//	Deblocking Chrominance
		sa->useOvlEnable = 0;			//	Enabled Overlap Filter(VC-1 Only)
		sa->useBtpEnable = 0;			//	Enable BTP(Bit-Plane)(VC-1 Only)
		break;
	case CODEC_STD_MPEG4:
		sa->useBitEnable = 1;			//	BitAxiSecEn (USE Bit Processor)
		sa->useIpEnable = 1;				//	Intra Prediction
		sa->useDbkYEnable = 1;			//	Deblocking Luminance
		sa->useDbkCEnable = 1;			//	Deblocking Chrominance
		sa->useOvlEnable = 0;			//	Enabled Overlap Filter(VC-1 Only)
		sa->useBtpEnable = 0;			//	Enable BTP(Bit-Plane)(VC-1 Only)
		break;
	case CODEC_STD_H263:
		if( totalMB > NUM_MB_SD )
		{
			sa->useDbkYEnable = 0;			//	Deblocking Luminance
			sa->useDbkCEnable = 0;			//	Deblocking Chrominance
		}
		else
		{
			sa->useDbkYEnable = 1;			//	Deblocking Luminance
			sa->useDbkCEnable = 1;			//	Deblocking Chrominance
		}
		sa->useBitEnable = 1;			//	BitAxiSecEn (USE Bit Processor)
		sa->useIpEnable = 1;				//	Intra Prediction
		sa->useOvlEnable = 0;			//	Enabled Overlap Filter(VC-1 Only)
		sa->useBtpEnable = 0;			//	Enable BTP(Bit-Plane)(VC-1 Only)
		break;
	}

	offset = 0;

	//	BIT Processor
	if( sa->useBitEnable )
	{
		sa->bufBitUse = sramPhyAddr + offset;
		if( CODEC_STD_AVC == codStd )
		{
			offset = offset + MbNumX * 144;
		}
		else
		{
			offset = offset + MbNumX *  16;
		}
	}

	//	Intra Prediction,( H.264 Only )
	if (sa->useIpEnable /*&& CODEC_STD_AVC == codStd*/)
	{
		sa->bufIpAcDcUse = sramPhyAddr + offset;
		offset = offset+ (MbNumX * 64);
	}

	//	Deblock Luma
	if (sa->useDbkYEnable)
	{
		sa->bufDbkYUse = sramPhyAddr + offset;
		if( CODEC_STD_AVC == codStd )
		{
			offset = offset+ (MbNumX * 64);
		}
		else if( CODEC_STD_H263 == codStd )
		{
			offset = offset + MbNumX * 128;
		}
	}

	//	Deblock Chroma
	if (sa->useDbkCEnable)
	{
		sa->bufDbkCUse = sramPhyAddr + offset;
		if( CODEC_STD_AVC == codStd )
		{
			offset = offset+ (MbNumX * 64);
		}
		else if( CODEC_STD_H263 == codStd )
		{
			offset = offset + MbNumX * 128;
		}
	}
	sa->bufSize = offset;

	if( sa->bufSize > sramSize )
	{
		NX_ErrMsg(("ConfigEncSecAXI() Failed!!!(bufSize=%d, sramSize=%d)\n", sa->bufSize, sramSize));
		return VPU_RET_ERR_SRAM;
	}

	return VPU_RET_OK;
}



int ConfigDecSecAXI(int codStd, SecAxiInfo *sa, int width, int height)
{
	int offset;
	int MbNumX = ((width & 0xFFFF) + 15) / 16;
	int MbNumY = ((height & 0xFFFF) + 15) / 16;
	int totalMB = MbNumX * MbNumY;

	int sramSize = VPU_SRAM_SIZE;
	int sramPhyAddr = VPU_SRAM_PHYSICAL_BASE;

	switch( codStd )
	{
	case CODEC_STD_AVC:
		if( (totalMB > NUM_MB_SD) && (totalMB <= NUM_MB_720) )
		{
			sa->useIpEnable = 1;				//	Intra Prediction
			sa->useDbkYEnable = 0;			//	Deblocking Luminance
			sa->useDbkCEnable = 0;			//	Deblocking Chrominance
		}
		if( totalMB > NUM_MB_720 )
		{
			sa->useIpEnable = 1;				//	Intra Prediction
			sa->useDbkYEnable = 0;			//	Deblocking Luminance
			sa->useDbkCEnable = 0;			//	Deblocking Chrominance
		}
		else
		{
			sa->useIpEnable = 1;				//	Intra Prediction
			sa->useDbkYEnable = 0;			//	Deblocking Luminance
			sa->useDbkCEnable = 0;			//	Deblocking Chrominance
		}
		sa->useBitEnable = 1;			//	BitAxiSecEn (USE Bit Processor)
		sa->useOvlEnable = 1;			//	Enabled Overlap Filter(VC-1 Only)
		sa->useBtpEnable = 1;			//	Enable BTP(Bit-Plane)(VC-1 Only)
		break;
	case CODEC_STD_VC1:
		sa->useBitEnable = 1;			//	BitAxiSecEn (USE Bit Processor)
		sa->useIpEnable = 0;			//	Intra Prediction
		sa->useDbkYEnable = 0;			//	Deblocking Luminance
		sa->useDbkCEnable = 0;			//	Deblocking Chrominance
		sa->useOvlEnable = 1;			//	Enabled Overlap Filter(VC-1 Only)
		sa->useBtpEnable = 1;			//	Enable BTP(Bit-Plane)(VC-1 Only)
		break;
	case CODEC_STD_MPEG4:
	case CODEC_STD_H263:
	case CODEC_STD_MPEG2:
		sa->useBitEnable = 1;			//	BitAxiSecEn (USE Bit Processor)
		sa->useIpEnable = 1;				//	Intra Prediction
		sa->useDbkYEnable = 0;			//	Deblocking Luminance
		sa->useDbkCEnable = 0;			//	Deblocking Chrominance
		sa->useOvlEnable = 1;			//	Enabled Overlap Filter(VC-1 Only)
		sa->useBtpEnable = 1;			//	Enable BTP(Bit-Plane)(VC-1 Only)
		break;
	case CODEC_STD_RV:
		sa->useBitEnable = 1;			//	BitAxiSecEn (USE Bit Processor)
		sa->useIpEnable = 1;				//	Intra Prediction
		sa->useDbkYEnable = 0;			//	Deblocking Luminance
		sa->useDbkCEnable = 0;			//	Deblocking Chrominance
		sa->useOvlEnable = 1;			//	Enabled Overlap Filter(VC-1 Only)
		sa->useBtpEnable = 1;			//	Enable BTP(Bit-Plane)(VC-1 Only)
		break;
	}


	offset = 0;
	//BIT
	if (sa->useBitEnable)
	{
		sa->useBitEnable = 1;
		sa->bufBitUse = sramPhyAddr + offset;

		switch (codStd)
		{
		case CODEC_STD_AVC:
			offset = offset + MbNumX * 144;
			break; // AVC
		case CODEC_STD_RV:
			offset = offset + MbNumX * 128;
			break;
		case CODEC_STD_VC1:
			offset = offset + MbNumX *  64;
			break;
		case CODEC_STD_AVS:
			offset = offset + (MbNumX + (MbNumX%4)) *  32;
			break;
		case CODEC_STD_MPEG2:
			offset = offset + MbNumX * 0;
			break;
		case CODEC_STD_VP8:
			offset = offset + MbNumX * 128;
			break;
		default:
			offset = offset + MbNumX *  16;
			break; // MPEG-4, Divx3
		}

		if (offset > sramSize)
		{
			sa->bufSize = 0;
			return 0;
		}

	}

	//Intra Prediction, ACDC
	if (sa->useIpEnable)
	{
		sa->bufIpAcDcUse = sramPhyAddr + offset;
		sa->useIpEnable = 1;

		switch (codStd)
		{
		case CODEC_STD_AVC:
			offset = offset + MbNumX * 64;
			break; // AVC
		case CODEC_STD_RV:
			offset = offset + MbNumX * 64;
			break;
		case CODEC_STD_VC1:
			offset = offset + MbNumX * 128;
			break;
		case CODEC_STD_AVS:
			offset = offset + MbNumX * 64;
			break;
		case CODEC_STD_MPEG2:
			offset = offset + MbNumX * 0;
			break;
		case CODEC_STD_VP8:
			offset = offset + MbNumX * 64;
			break;
		default:
			offset = offset + MbNumX * 128;
			break; // MPEG-4, Divx3
		}

		if (offset > sramSize)
		{
			sa->bufSize = 0;
			return 0;
		}
	}


	//Deblock Chroma
	if (sa->useDbkCEnable)
	{
		sa->bufDbkCUse = sramPhyAddr + offset;
		sa->useDbkCEnable = 1;
		switch (codStd)
		{
		case CODEC_STD_AVC:
			offset = offset + (MbNumX * 128);
			break; // AVC
		case CODEC_STD_RV:
			offset = offset + MbNumX * 128;
			break;
		case CODEC_STD_VC1:
			offset = offset + MbNumX * 256;
			break;
		case CODEC_STD_AVS:
			offset = offset + MbNumX * 128;
			break;
		case CODEC_STD_MPEG2:
			offset = offset + MbNumX * 64;
			break;
		case CODEC_STD_VP8:
			offset = offset + MbNumX * 128;
			break;
		default:
			offset = offset + MbNumX * 64;
			break;
		}

		if (offset > sramSize)
		{
			sa->bufSize = 0;
			return 0;
		}
	}

	//Deblock Luma
	if (sa->useDbkYEnable)
	{
		sa->bufDbkYUse = sramPhyAddr + offset;
		sa->useDbkYEnable = 1;

		switch (codStd)
		{
		case CODEC_STD_AVC:
			offset = offset + (MbNumX * 128);
			break; // AVC
		case CODEC_STD_RV:
			offset = offset + MbNumX * 128;
			break;
		case CODEC_STD_VC1:
			offset = offset + MbNumX * 256;
			break;
		case CODEC_STD_AVS:
			offset = offset + MbNumX * 128;
			break;
		case CODEC_STD_MPEG2:
			offset = offset + MbNumX * 128;
			break;
		case CODEC_STD_VP8:
			offset = offset + MbNumX * 128;
			break;
		default:
			offset = offset + MbNumX * 128;
			break;
		}

		if (offset > sramSize)
		{
			sa->bufSize = 0;
			return 0;
		}
	}

	//VC1 Bit-plane
	if (sa->useBtpEnable)
	{
		if (codStd != CODEC_STD_VC1)
		{
			sa->useBtpEnable = 0;
		}
		else
		{
			int oneBTP;

			offset = ((offset+255)&~255);
			sa->bufBtpUse = sramPhyAddr + offset;
			sa->useBtpEnable = 1;

			oneBTP  = (((MbNumX+15)/16) * MbNumY + 1) * 2;
			oneBTP  = (oneBTP%256) ? ((oneBTP/256)+1)*256 : oneBTP;

			offset = offset + oneBTP * 3;
			//offset = ((offset+255)&~255);
		}
	}

	//VC1 Overlap
	if (sa->useOvlEnable)
	{
		if (codStd != CODEC_STD_VC1)
		{
			sa->useOvlEnable = 0;
		}
		else
		{
			sa->bufOvlUse = sramPhyAddr + offset;
			sa->useOvlEnable = 1;

			offset = offset + MbNumX *  80;
		}
	}

	sa->bufSize = offset;

	if( sa->bufSize > sramSize )
	{
		NX_ErrMsg(("ConfigDecSecAXI() Failed!!!(bufSize=%d, sramSize=%d)\n", sa->bufSize, sramSize));
		return VPU_RET_ERR_SRAM;
	}

	return 1;
}







// Maverick Cache II
unsigned int MaverickCache2Config(int decoder , int interleave, int bypass, int burst, int merge, int mapType, int wayshape)
{
	unsigned int cacheConfig = 0;

	if(decoder) // decoder
	{
		if (mapType == 0)// LINEAR_FRAME_MAP
		{
			//VC1 opposite field padding is not allowable in UV separated, burst 8 and linear map
			if(!interleave)
				burst = 0;

			wayshape = 15;

			if (merge == 1)
				merge = 3;

			//GDI constraint. Width should not be over 64
			if (( merge== 1) && (burst))
				burst = 0;
		}
		else
		{
			//horizontal merge constraint in tiled map
			if (merge == 1)
				merge = 3;
		}
	}
	else // encoder
	{
		if (mapType == 0)// LINEAR_FRAME_MAP
		{
			wayshape = 15;
			//GDI constraint. Width should not be over 64
			if ((merge == 1) && (burst))
				burst= 0;
		}
		else
		{
			//horizontal merge constraint in tiled map
			if (merge == 1)
				merge = 3;
		}

	}

	cacheConfig = (merge & 0x3) << 9;
	cacheConfig = cacheConfig | ((wayshape & 0xf) << 5);
	cacheConfig = cacheConfig | ((burst & 0x1) << 3);
	cacheConfig = cacheConfig | (bypass & 0x3);

	if(mapType != 0)//LINEAR_FRAME_MAP
		cacheConfig = cacheConfig | 0x00000004;

	///{16'b0, 5'b0, merge[1:0], wayshape[3:0], 1'b0, burst[0], map[0], bypass[1:0]};
	return cacheConfig;
}


#if 0
static int GetXY2RBCLogic(int map_val,int xpos,int ypos, int tb)
{
   	int invert;
	int assign_zero;
	int tbxor;
	int xysel;
	int bitsel;

	int xypos,xybit,xybit_st1,xybit_st2,xybit_st3;

	invert		= map_val >> 7;
	assign_zero = (map_val & 0x78) >> 6;
	tbxor		= (map_val & 0x3C) >> 5;
	xysel		= (map_val & 0x1E) >> 4;
	bitsel		= map_val & 0x0f;

	xypos     = (xysel) ? ypos : xpos;
	xybit     = (xypos >> bitsel) & 0x01;
	xybit_st1 = (tbxor)       ? xybit^tb : xybit;
	xybit_st2 = (assign_zero) ? 0 : xybit_st1;
	xybit_st3 = (invert)      ? !xybit_st2 : xybit_st2;

	return xybit_st3;
}

static int rbc2axi_logic(int map_val , int ra_in, int ba_in, int ca_in)
{
   	int rbc;
	int rst_bit ;
	int rbc_sel = map_val >> 4;
	int bit_sel = map_val & 0x0f;


	if (rbc_sel == 0)
		rbc = ca_in;
	else if (rbc_sel == 1)
		rbc = ba_in;
	else if (rbc_sel == 2)
		rbc = ra_in;
	else
		rbc = 0;

	rst_bit = ((rbc >> bit_sel) & 1);

	return rst_bit;
}
#endif

#if 0

int GetXY2AXI_ADDR(int ycbcr, int intlv, int posY, int posX, int stride, FrameBuffer *fb)
{
   	int ypos_mod;
	int temp;
	int temp_bit;
	int i;
	int tb;
	int ra_base;
	int ras_base;
	int ra_conv,ba_conv,ca_conv;

	int pix_addr;

	int lum_top_base,chr_top_base;
	int lum_bot_base,chr_bot_base;

	int mbx,mby,mb_addr;
	int temp_val12bit, temp_val6bit;
	int Addr;
	int mb_raster_base;


	pix_addr       = 0;
	mb_raster_base = 0;
	ra_conv        = 0;
	ba_conv        = 0;
	ca_conv        = 0;

	tb = posY & 0x1;

	ypos_mod =  sTiledInfo.tb_separate_map ? posY >> 1 : posY;

	Addr = ycbcr == 0 ? fb->bufY  :
		   ycbcr == 2 ? fb->bufCb : fb->bufCr;

	if (sTiledInfo.MapType == LINEAR_FRAME_MAP)
		return ((posY * stride) + posX) + Addr;

	// 20bit = AddrY [31:12]
	lum_top_base =   fb->bufY >> 12;

	// 20bit = AddrY [11: 0], AddrCb[31:24]
	chr_top_base = ((fb->bufY  & 0xfff) << 8) | ((fb->bufCb >> 24) & 0xff);  //12bit +  (32-24) bit

	// 20bit = AddrCb[23: 4]
	lum_bot_base =  (fb->bufCb >> 4) & 0xfffff;

	// 20bit = AddrCb[ 3: 0], AddrCr[31:16]
	chr_bot_base =  ((fb->bufCb & 0xf) << 16) | ((fb->bufCr >> 16) & 0xffff);


 	if (sTiledInfo.MapType == TILED_FRAME_MB_RASTER_MAP || sTiledInfo.MapType == TILED_FIELD_MB_RASTER_MAP)
	{
		if (ycbcr == 0)
		{
			mbx = posX/16;
			mby = posY/16;
		}
		else //always interleave
		{
			mbx = posX/16;
			mby = posY/8;
		}

		mb_addr = (stride/16) * mby + mbx;

		// ca[7:0]
		for (i=0 ; i<8; i++)
		{
			if (ycbcr==2 || ycbcr == 3)
				temp = sTiledInfo.xy2ca_map[i] & 0xff;
			else
				temp = sTiledInfo.xy2ca_map[i] >> 8;
			temp_bit = GetXY2RBCLogic(temp,posX,ypos_mod,tb);
			ca_conv  = ca_conv + (temp_bit << i);
		}

		// ca[15:8]
		ca_conv      = ca_conv + ((mb_addr & 0xff) << 8);

		// ra[15:0]
		ra_conv      = mb_addr >> 8;


		// ra,ba,ca -> axi
		for (i=0; i<32; i++) {

			temp_val12bit = sTiledInfo.rbc2axi_map[i];
			temp_val6bit  = (ycbcr == 0 ) ? (temp_val12bit >> 6) : (temp_val12bit & 0x3f);

			temp_bit = rbc2axi_logic(temp_val6bit,ra_conv,ba_conv,ca_conv);

			pix_addr =  pix_addr + (temp_bit<<i);
		}

		if (sTiledInfo.tb_separate_map ==1 && tb ==1)
			mb_raster_base = ycbcr == 0 ? lum_bot_base : chr_bot_base;
		else
			mb_raster_base = ycbcr == 0 ? lum_top_base : chr_top_base;

		pix_addr = pix_addr + (mb_raster_base << 12);
	}
	else
	{
		// ca
		for (i=0 ; i<16; i++)
		{
			if (ycbcr==0 || ycbcr==1)
				temp = sTiledInfo.xy2ca_map[i] >> 8;
			else
				temp = sTiledInfo.xy2ca_map[i] & 0xff;

			temp_bit = GetXY2RBCLogic(temp,posX,ypos_mod,tb);
			ca_conv  = ca_conv + (temp_bit << i);
		}

		// ba
		for (i=0 ; i<4; i++)
		{
			if (ycbcr==2 || ycbcr == 3)
				temp = sTiledInfo.xy2ba_map[i] & 0xff;
			else
				temp = sTiledInfo.xy2ba_map[i] >> 8;

			temp_bit = GetXY2RBCLogic(temp,posX,ypos_mod,tb);
			ba_conv  = ba_conv + (temp_bit << i);
		}

		// ras
		for (i=0 ; i<16; i++)
		{
			if (ycbcr==2 || ycbcr == 3)
				temp = sTiledInfo.xy2ra_map[i] & 0xff;
			else
				temp = sTiledInfo.xy2ra_map[i] >> 8;

			temp_bit = GetXY2RBCLogic(temp,posX,ypos_mod,tb);
			ra_conv  = ra_conv + (temp_bit << i);
		}

		if (sTiledInfo.tb_separate_map == 1 && tb == 1)
			ras_base = Addr >> 16;
		else
			ras_base = Addr & 0xffff;

		ra_base  = ra_conv + ras_base;
		pix_addr = 0;

		// ra,ba,ca -> axi
		for (i=0; i<32; i++) {

			temp_val12bit = sTiledInfo.rbc2axi_map[i];
			temp_val6bit  = (ycbcr == 0 ) ? (temp_val12bit >> 6) : (temp_val12bit & 0x3f);

			temp_bit = rbc2axi_logic(temp_val6bit,ra_base,ba_conv,ca_conv);

			pix_addr = pix_addr + (temp_bit<<i);
		}
		pix_addr += VpuReadReg(GDI_TILEDBUF_BASE);	// this register is assigned in StartOneFrame
	}

	//printf("ycbcr[%d], intlv[%d], ypos[%d], xpos[%d], stride[%d], base_addr[%x], pix_addr[%x], ca[%x], ba[%x], ra[%x], ra_base[%x]",
	//	ycbcr, 0, posY , posX ,stride ,Addr ,pix_addr, ca_conv, ba_conv, ra_conv, ra_base);
	return pix_addr;
}

#endif
