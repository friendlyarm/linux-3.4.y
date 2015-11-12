//------------------------------------------------------------------------------
//  Copyright (C) 2012 Nexell Co., All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      : SDMMC
//  File        : nx_SDMMC.h
//  Description :
//  Author      : sei (parkjh@nexell.com)
//  History     :
//------------------------------------------------------------------------------
#ifndef __NX_SDMMC_H__
#define __NX_SDMMC_H__

#include "../base/nx_prototype.h"

#ifdef  __cplusplus
extern "C"
{
#endif


//------------------------------------------------------------------------------
/// @defgroup   SDMMC SD Host Controller
//------------------------------------------------------------------------------
//@{

//------------------------------------------------------------------------------
/// @brief  SDMMC Module
//------------------------------------------------------------------------------
//@{

    /// @brief  SDMMC Module's Register List
    struct  NX_SDMMC_RegisterSet
    {
        // Internal Register
        volatile U32    CTRL;                       ///< 0x000 : Control Register                               [  31:   0]
        volatile U32    PWREN;                      ///< 0x004 : Power Enable Register                          [  63:  32]
        volatile U32    CLKDIV;                     ///< 0x008 : Clock Divider Register                         [  95:  64]
        volatile U32    CLKSRC;                     ///< 0x00C : Clock Source Register                          [ 127:  96]
        volatile U32    CLKENA;                     ///< 0x010 : Clock Enable Register                          [ 159: 128]
        volatile U32    TMOUT;                      ///< 0x014 : Time-Out Register                              [ 191: 160]
        volatile U32    CTYPE;                      ///< 0x018 : Card Type Register                             [ 223: 192]
        volatile U32    BLKSIZ;                     ///< 0x01C : Block Size Register                            [ 255: 224]
        volatile U32    BYTCNT;                     ///< 0x020 : Byte Count Register                            [ 287: 256]
        volatile U32    INTMASK;                    ///< 0x024 : Interrupt Mask Register                        [ 319: 288]
        volatile U32    CMDARG;                     ///< 0x028 : Command Argument Register                      [ 351: 320]
        volatile U32    CMD;                        ///< 0x02C : Command Register                               [ 383: 352]
        volatile U32    RESP0;                      ///< 0x030 : Response 0 Register                            [ 415: 384]
        volatile U32    RESP1;                      ///< 0x034 : Response 1 Register                            [ 447: 416]
        volatile U32    RESP2;                      ///< 0x038 : Response 2 Register                            [ 479: 448]
        volatile U32    RESP3;                      ///< 0x03C : Response 3 Register                            [ 511: 480]
        volatile U32    MINTSTS;                    ///< 0x040 : Masked Interrupt Status Register               [ 543: 512]
        volatile U32    RINTSTS;                    ///< 0x044 : Raw Interrupt Status Register                  [ 575: 544]
        volatile U32    STATUS;                     ///< 0x048 : Status Register - Mainly for Debug Purpose     [ 607: 576]
        volatile U32    FIFOTH;                     ///< 0x04C : FIFO Threshold Register                        [ 639: 608]
        volatile U32    CDETECT;                    ///< 0x050 : Card Detect Register                           [ 671: 640]
        volatile U32    WRTPRT;                     ///< 0x054 : Write Protect Register                         [ 703: 672]
        volatile U32    GPIO;                       ///< 0x058 : General Purpose Input/Output Register          [ 735: 704]
        volatile U32    TCBCNT;                     ///< 0x05C : Transferred CIU card byte count                [ 767: 736]
        volatile U32    TBBCNT;                     ///< 0x060 : Transferred Host Byte Count                    [ 799: 768]
        volatile U32    DEBNCE;                     ///< 0x064 : Card Detect Debounce Register                  [ 831: 800]
        volatile U32    USRID;                      ///< 0x068 : User ID Register                               [ 863: 832]
        volatile U32    VERID;                      ///< 0x06C : Version ID Register                            [ 895: 864]
        volatile U32    HCON;                       ///< 0x070 : Hardware Configuration Register                [ 927: 896]
        volatile U32    UHS_REG;                    ///< 0x074 : UHS_REG register
        volatile U32    RSTn;                       ///< 0x078 : Hardware reset register
        volatile U32    _Reserved0;                 ///< 0x07C
        volatile U32    BMOD;                       ///< 0x080 : Bus Mode Register
        volatile U32    PLDMND;                     ///< 0x084 : Poll Demand Register
        volatile U32    DBADDR;                     ///< 0x088 : Descriptor List Base Address Register
        volatile U32    IDSTS;                      ///< 0x08C : Internal DMAC Status Register
        volatile U32    IDINTEN;                    ///< 0x090 : Internal DMAC Interrupt Enable Register
        volatile U32    DSCADDR;                    ///< 0x094 : Current Host Descriptor Address Register
        volatile U32    BUFADDR;                    ///< 0x098 : Current Buffer Descriptor Address Register
        volatile U8     _Reserved1[0x100-0x09C];    ///< 0x09C ~ 0x100 reserved area
        volatile U32    CARDTHRCTL;                 ///< 0x100 : Card Read Threshold Enable
        volatile U32    BACKEND_POWER;              ///< 0x104 : Back-end Power
        volatile U32    UHS_REG_EXT;                ///< 0x108 : eMMC 4.5 1.2V Register
        volatile U32    EMMC_DDR_REG;               ///< 0x10C : eMMC DDR START bit detection control register
        volatile U32    ENABLE_SHIFT;               ///< 0x110 : Phase shift control register
        volatile U32    CLKCTRL;                    ///< 0x114 : External clock phase & delay control register
        volatile U8     _Reserved2[0x200-0x118];    ///< 0x118 ~ 0x200
        volatile U32    DATA;                       ///< 0x200 : Data
    };

    /// @brief  SDMMC BIU
    struct strNxSdmmcBiu
    {
        volatile U32    CTRL;
        volatile U32    BSIZE;
        volatile U32    BADDR;
        struct strNxSdmmcBiu *Next;
    };

    typedef struct strNxSdmmcBiu    NX_SDMMC_BIU;

    /// @brief  SDMMC BIU Config
    typedef struct
    {
        CBOOL   OWN;
        CBOOL   IntDisable;
        U32     BuffAddr;
        U32     DataSize;
    } NX_SDMMC_BIUConfig;

    /// @brief  SDMMC Interrupts for Interrupt Interface
    enum    NX_SDMMC_INT
    {
        NX_SDMMC_INT_SDIO   = 16,   ///< SDIO Interrupt.\r\n
                                    ///     Interrupt from SDIO card.
        NX_SDMMC_INT_EBE    = 15,   ///< End Bit Error for reading, Write no CRC for wrting.\r\n
                                    ///     Error in end-bit during read operations, or no data CRC or negative CRC
                                    ///     received during write operation.\r\n\r\n
                                    ///     \b Note> \r\n
                                    ///     For MMC CMD19, there may be no CRC status returned by the card.
                                    ///     Hence, NX_SDMMC_INT_EBE is set for CMD19. The application should not treat
                                    ///     this as an error.
        NX_SDMMC_INT_ACD    = 14,   ///< Auto Command Done.\r\n
                                    ///     Stop/abort commands automatically sent by card unit and not initiated by
                                    ///     host; similar to Command Done (NX_SDMMC_INT_CD) interrupt.
        NX_SDMMC_INT_SBE    = 13,   ///< Start Bit Error.\r\n
                                    ///     Error in data start bit when data is read from a card. In 4-bit mode, if all
                                    ///     data bits do not have start bit, then this error is set.
        NX_SDMMC_INT_HLE    = 12,   ///< Hardware Locked Write Error.\r\n
                                    ///     This interrupt is genearted when the SDMMC module cannot load a command
                                    ///     issued by the user. When the user sets a command with NX_SDMMC_CMDFLAG_STARTCMD
                                    ///     flag, the SDMMC module tries to load the command. If the command buffer is
                                    ///     already filled with a command, this error is raised.
                                    ///     And also if the user try to modify any of settings while a command loading is in
                                    ///     progress, then the modification is ignored and the SDMMC module generates this
                                    ///     error.
        NX_SDMMC_INT_FRUN   = 11,   ///< FIFO underrun/overrun Error.\r\n
                                    ///     Host tried to push data when FIFO was full, or host tried to read data
                                    ///     when FIFO was empty. Typically this should not happen, except due to
                                    ///     error in software.\r\n
                                    ///     SDMMC module never pushes data into FIFO when FIFO is full, and pop data
                                    ///     when FIFO is empty.
        NX_SDMMC_INT_HTO    = 10,   ///< Data Starvation by Host Timeout.\r\n
                                    ///     To avoid data loss, SDCLK is stopped if FIFO is empty
                                    ///     when writing to card, or FIFO is full when reading from card. Whenever
                                    ///     SDCLK is stopped to avoid data loss, data-starvation timeout counter
                                    ///     is started with data-timeout value. This interrupt is set if host does not fill
                                    ///     data into FIFO during write to card, or does not read from FIFO during
                                    ///     read from card before timeout period.\r\n
                                    ///     Even after timeout, SDCLK stays in stopped state with SDMMC module waiting.
                                    ///     It is responsibility of host to push or pop data into FIFO upon interrupt,
                                    ///     which automatically restarts SDCLK and SDMMC module.\r\n
                                    ///     Even if host wants to send stop/abort command, it still needs to ensure it
                                    ///     has to push or pop FIFO so that clock starts in order for stop/abort
                                    ///     command to send on cmd signal along with data that is sent or received
                                    ///     on data line.
        NX_SDMMC_INT_DRTO   = 9,    ///< Data Read Timeout.\r\n
                                    ///     Data timeout occurred. Data Transfer Over (NX_SDMMC_INT_DTO) also set if data
                                    ///     timeout occurs.
        NX_SDMMC_INT_RTO    = 8,    ///< Response Timeout.\r\n
                                    ///     Response timeout occurred. Command Done (NX_SDMMC_INT_CD) also set if response
                                    ///     timeout occurs. If command involves data transfer and when response
                                    ///     times out, no data transfer is attempted by SDMMC module.
        NX_SDMMC_INT_DCRC   = 7,    ///< Data CRC Error.\r\n
                                    ///     Received Data CRC does not match with locally-generated CRC in SDMMC module.
        NX_SDMMC_INT_RCRC   = 6,    ///< Response CRC Error.\r\n
                                    ///     Response CRC does not match with locally-generated CRC in SDMMC module.
        NX_SDMMC_INT_RXDR   = 5,    ///< Receive FIFO Data Request.\r\n
                                    ///     Interrupt set during read operation from card when FIFO level is greater
                                    ///     than Receive-Threshold level.\r\n\r\n
                                    ///     \b Recommendation> In DMA modes, this interrupt should not be enabled.\r\n\r\n
                                    ///     ISR, in non-DMA mode: \r\n
                                    /// @code
                                    /// pop RX_WMark + 1 data from FIFO
                                    /// @endcode
        NX_SDMMC_INT_TXDR   = 4,    ///< Transmit FIFO Data Request.\r\n
                                    ///     Interrupt set during write operation to card when FIFO level reaches less
                                    ///     than or equal to Transmit-Threshold level.\r\n\r\n
                                    ///     \b Recommendation> In DMA modes, this interrupt should not be enabled.\r\n\r\n
                                    ///     ISR in non-DMA mode: \r\n
                                    /// @code
                                    /// if (pending_bytes > (16 - TX_WMark))
                                    ///     push (16 - TX_WMark) data into FIFO
                                    /// else
                                    ///     push pending_bytes data into FIFO
                                    /// @endcode
        NX_SDMMC_INT_DTO    = 3,    ///< Data Transfer Over.\r\n
                                    ///     Data transfer completed, even if there is Start Bit Error or CRC error.
                                    ///     This bit is also set when "read data-timeout" occurs.\r\n\r\n
                                    ///     \b Recommendation> \r\n
                                    ///     In non-DMA mode, when data is read from card, on seeing interrupt,
                                    ///     host should read any pending data from FIFO.\r\n
                                    ///     In DMA mode, DMA controllers guarantee FIFO is flushed before interrupt.\r\n\r\n
                                    ///     \b Note> \r\n
                                    ///     NX_SDMMC_INT_DTO is set at the end of the last data block, even if
                                    ///     the device asserts MMC busy after the last data block.
        NX_SDMMC_INT_CD     = 2,    ///< Command Done.\r\n
                                    ///     Command sent to card and got response from card, even if Response
                                    ///     Error or CRC error occurs. Also set when response timeout occurs.
        NX_SDMMC_INT_RE     = 1,    ///< Response Error.\r\n
                                    ///     Error in received response set if one of following occurs:
                                    ///     - Transmission bit != 0
                                    ///     - Command index mismatch
                                    ///     - End-bit != 1
        NX_SDMMC_INT_CDET   = 0     ///< Card Detected.
    };

    /// @brief  Command Flags for NX3_SDMMC_SetCommand()
    enum    NX_SDMMC_CMDFLAG
    {
        NX_SDMMC_CMDFLAG_STARTCMD           = 1UL<<31,  ///< Start Command.
                                                        ///     Set a command with this flag to update modified settings to the
                                                        ///     SDMMC module. If this flag is set, you have not to modify any
                                                        ///     any settings until the command is taken by the SDMMC module.
        NX_SDMMC_CMDFLAG_USE_HOLD           = 1UL<<29,  ///< Use Hold Register
                                                        ///     0: CMD and DATA sent to card bypassing HOLD Register
                                                        ///     1: CMD and DATA sent to card through the HOLD Register
        NX_SDMMC_CMDFLAG_VOLT_SWITCH        = 1UL<<28,  ///< Volatage switch bit
        NX_SDMMC_CMDFLAG_BOOT_MODE          = 1UL<<27,  ///< boot mode
        NX_SDMMC_CMDFLAG_DISABLE_BOOT       = 1UL<<26,  ///< disable boot
        NX_SDMMC_CMDFLAG_EXPECT_BOOT_ACK    = 1UL<<25,  ///< expect boot ack
        NX_SDMMC_CMDFLAG_ENABLE_BOOT        = 1UL<<24,  ///< enable boot
        NX_SDMMC_CMDFLAG_CCS_EXPECTED       = 1UL<<23,  ///< ccs expected
        NX_SDMMC_CMDFLAG_READ_CEATA         = 1UL<<22,  ///< Read ceata
        NX_SDMMC_CMDFLAG_UPDATECLKONLY      = 1UL<<21,  ///< Just update clock settings for SDCLK into card clock domain.\r\n
                                                        ///     Changes card clocks (change frequency, truncate off or on, and
                                                        ///     set low-frequency mode); provided in order to change clock
                                                        ///     frequency or stop clock without having to send command to
                                                        ///     cards.\r\n
                                                        ///     Even though this flag is set with NX_SDMMC_CMDFLAG_STARTCMD flag,
                                                        ///     SDMMC module does not issue NX_SDMMC_INT_CD signal upon command completion.
        NX_SDMMC_CMDFLAG_CARD_NUMBER        = 1UL<<16,  ///< Card Number in use.
        NX_SDMMC_CMDFLAG_SENDINIT           = 1UL<<15,  ///< Send initialization sequence before sending this command.\r\n
                                                        ///     After power on, 80 clocks must be sent to card for initialization
                                                        ///     before sending any commands to card. This flag should be set while
                                                        ///     sending first command to card so that controller will initialize
                                                        ///     clocks before sending command to card.
        NX_SDMMC_CMDFLAG_STOPABORT          = 1UL<<14,  ///< Stop or abort command intended to stop current data transfer in progress.\r\n
                                                        ///     When open-ended or predefined data transfer is in progress, and
                                                        ///     host issues stop or abort command to stop data transfer, this flag should
                                                        ///     be set so that command/data state-machines of SDMMC module can return
                                                        ///     correctly to idle state.
        NX_SDMMC_CMDFLAG_WAITPRVDAT         = 1UL<<13,  ///< Wait for previous data transfer completion before sending command.\r\n
                                                        ///     Command without this flag typically used to query
                                                        ///     status of card during data transfer or to stop current data transfer.
        NX_SDMMC_CMDFLAG_SENDAUTOSTOP       = 1UL<<12,  ///< Send stop command at end of data transfer.\r\n
                                                        ///     When set, SDMMC module sends stop command to card at end of data transfer. \r\n
                                                        ///     * when this flag should be set, since some data
                                                        ///     transfers do not need explicit stop commands.\r\n
                                                        ///     * open-ended transfers that software should explicitly send to
                                                        ///     stop command.\r\n
                                                        ///     Additionally, when "resume" is sent to resume . suspended
                                                        ///     memory access of SD-Combo card . this falg should be set correctly if
                                                        ///     suspended data transfer needs send_auto_stop.\r\n
                                                        ///     Don't care if no data expected from card.
        NX_SDMMC_CMDFLAG_BLOCK              = 0UL<<11,  ///< Block data transfer command.\r\n Don't care if no data expected from card.
        NX_SDMMC_CMDFLAG_STREAM             = 1UL<<11,  ///< Stream data transfer command.\r\n Don't care if no data expected from card.
        NX_SDMMC_CMDFLAG_TXDATA             = 3UL<< 9,  ///< Write to card.\r\n Do not set this flag if no data expected from card.
        NX_SDMMC_CMDFLAG_RXDATA             = 1UL<< 9,  ///< Read form card.\r\n Do not set this flag if no data expected from card.
        NX_SDMMC_CMDFLAG_CHKRSPCRC          = 1UL<< 8,  ///< Check response CRC.\r\n
                                                        ///     Some of command responses do not return valid CRC bits.
                                                        ///     Software should disable CRC checks for those commands in
                                                        ///     order to disable CRC checking by SDMMC module.
        NX_SDMMC_CMDFLAG_SHORTRSP           = 1UL<< 6,  ///< Short response expected from card.\r\n Do not set this flag if no response is expected from card.
        NX_SDMMC_CMDFLAG_LONGRSP            = 3UL<< 6   ///< Long response expected from card.\r\n Do not set this flag if no response is expected from card.
    };

    /// @brief  Command FSM state for NX_SDMMC_GetCommandFSM().
    typedef enum
    {
        NX_SDMMC_CMDFSM_IDLE            = 0,    ///< Idle.
        NX_SDMMC_CMDFSM_SENDINIT        = 1,    ///< Send init sequence.
        NX_SDMMC_CMDFSM_TXCMDSTART      = 2,    ///< Tx command start bit
        NX_SDMMC_CMDFSM_TXCMDTX         = 3,    ///< Tx command tx bit.
        NX_SDMMC_CMDFSM_TXCMDINDEXARG   = 4,    ///< Tx command index + arg.
        NX_SDMMC_CMDFSM_TXCMDCRC7       = 5,    ///< Tx command CRC7.
        NX_SDMMC_CMDFSM_TXCMDEND        = 6,    ///< Tx command end bit.
        NX_SDMMC_CMDFSM_RXRSPSTART      = 7,    ///< Rx response start bit.
        NX_SDMMC_CMDFSM_RXRSPIRQ        = 8,    ///< Rx response IRQ response.
        NX_SDMMC_CMDFSM_RXRSPTX         = 9,    ///< Rx response tx bit.
        NX_SDMMC_CMDFSM_TXRSPCMDIDX     = 10,   ///< Rx response command index.
        NX_SDMMC_CMDFSM_RXRSPDATA       = 11,   ///< Rx response data.
        NX_SDMMC_CMDFSM_RXRSPCRC7       = 12,   ///< Rx response CRC7.
        NX_SDMMC_CMDFSM_RXRSPEND        = 13,   ///< Rx response end bit.
        NX_SDMMC_CMDFSM_CMDWAITNCC      = 14,   ///< Command path wait NCC.
        NX_SDMMC_CMDFSM_WAIT            = 15    ///< Wait; Comamnd to Response turnaround.
    } NX_SDMMC_CMDFSM;

    /// @brief  SDMMC clock phase shift
    typedef enum
    {
        NX_SDMMC_CLOCK_SHIFT_0          = 0,
        NX_SDMMC_CLOCK_SHIFT_90         = 1,
        NX_SDMMC_CLOCK_SHIFT_180        = 2,
        NX_SDMMC_CLOCK_SHIFT_270        = 3
    } NX_SDMMC_CLKSHIFT;

	typedef enum
	{
		NX_SDMMC_CLOCK_SOURCE_0			= 0,
		NX_SDMMC_CLOCK_SOURCE_1			= 1,
		NX_SDMMC_CLOCK_SOURCE_2			= 2,
		NX_SDMMC_CLOCK_SOURCE_3			= 3,
	} NX_SDMMC_CLOCK_SOURCE;

//------------------------------------------------------------------------------
/// @brief  SDMMC Module's MACRO
//------------------------------------------------------------------------------
#define NX_SDMMC_MAX_BIU_DATASIZE   (1UL<<12)

//------------------------------------------------------------------------------
/// @brief  To remove following waring on RVDS compiler
//          Warning : #66-D: enumeration value is out of "int" range
//------------------------------------------------------------------------------
//#ifdef __arm
//#pragma diag_remark 66      // disable #66 warining
//#endif

//------------------------------------------------------------------------------
/// @name   Module Interface
//------------------------------------------------------------------------------
//@{
CBOOL   NX_SDMMC_Initialize( void );
U32     NX_SDMMC_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
/// @name   Basic Interface
//------------------------------------------------------------------------------
//@{
U32     NX_SDMMC_GetPhysicalAddress( U32 ModuleIndex );
U32     NX_SDMMC_GetSizeOfRegisterSet( void );

void    NX_SDMMC_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*    NX_SDMMC_GetBaseAddress( U32 ModuleIndex );

CBOOL   NX_SDMMC_OpenModule( U32 ModuleIndex );
CBOOL   NX_SDMMC_CloseModule( U32 ModuleIndex );

CBOOL   NX_SDMMC_CheckBusy( U32 ModuleIndex );
CBOOL   NX_SDMMC_CanPowerDown( U32 ModuelIndex );
//@}

//------------------------------------------------------------------------------
/// @name   Interrupt Interface
//------------------------------------------------------------------------------
//@{
S32     NX_SDMMC_GetInterruptNumber( U32 ModuleIndex );

void    NX_SDMMC_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable );
CBOOL   NX_SDMMC_GetInterruptEnable( U32 ModuleIndex, S32 IntNum );
CBOOL   NX_SDMMC_GetInterruptPending( U32 ModuleIndex, S32 IntNum );
void    NX_SDMMC_ClearInterruptPending( U32 ModuleIndex, S32 IntNum );

void    NX_SDMMC_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_SDMMC_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL   NX_SDMMC_GetInterruptPendingAll( U32 ModuleIndex );
void    NX_SDMMC_ClearInterruptPendingAll( U32 ModuleIndex );
S32     NX_SDMMC_GetInterruptPendingNumber( U32 ModuleIndex );

void    NX_SDMMC_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag );
U32     NX_SDMMC_GetInterruptEnable32( U32 ModuleIndex );
U32     NX_SDMMC_GetInterruptPending32( U32 ModuleIndex );
void    NX_SDMMC_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag );
//@}

//------------------------------------------------------------------------------
/// @name   Clock Control Interface
//------------------------------------------------------------------------------
//@{
U32     NX_SDMMC_GetClockNumber( U32 ModuleIndex );
U32     NX_SDMMC_GetResetNumber( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
/// @name   Module customized operations
//------------------------------------------------------------------------------
//@{
void    NX_SDMMC_SetPowerEnable( U32 ModuleIndex, U32 PowerIndex, CBOOL Enable );
CBOOL   NX_SDMMC_GetPowerEnable( U32 ModuleIndex, U32 PowerIndex );

void    NX_SDMMC_AbortReadData( U32 ModuleIndex );
void    NX_SDMMC_SendIRQResponse( U32 ModuleIndex );
void    NX_SDMMC_SetReadWait( U32 ModuleIndex, CBOOL bAssert );

//void    NX_SDMMC_ResetDMA( U32 ModuleIndex );
//CBOOL   NX_SDMMC_IsResetDMA( U32 ModuleIndex );
void    NX_SDMMC_ResetDMAC( U32 ModuleIndex );
void    NX_SDMMC_SetDMAMode( U32 ModuleIndex, CBOOL Enable );
void    NX_SDMMC_SetUseInternalDMAC( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_SDMMC_IsDMAReq( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsDMAAck( U32 ModuleIndex );

void    NX_SDMMC_ResetFIFO( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsResetFIFO( U32 ModuleIndex );
void    NX_SDMMC_ResetController( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsResetController( U32 ModuleIndex );

void    NX_SDMMC_SetClockSource( U32 ModuleIndex, NX_SDMMC_CLOCK_SOURCE ClkSrc );
NX_SDMMC_CLOCK_SOURCE    NX_SDMMC_GetClockSource( U32 ModuleIndex );
void    NX_SDMMC_SetOutputClockDivider( U32 ModuleIndex, U32 divider );
U32     NX_SDMMC_GetOutputClockDivider( U32 ModuleIndex );
void    NX_SDMMC_SetLowPowerClockMode( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_SDMMC_GetLowPowerClockMode( U32 ModuleIndex );
void    NX_SDMMC_SetOutputClockEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_SDMMC_GetOutputClockEnable( U32 ModuleIndex );

void    NX_SDMMC_SetDriveClockShiftPhase( U32 ModuleIndex, NX_SDMMC_CLKSHIFT ClockShift );
NX_SDMMC_CLKSHIFT   NX_SDMMC_GetDriveClockShiftPhase( U32 ModuleIndex );
void    NX_SDMMC_SetSampleClockShiftPhase( U32 ModuleIndex, NX_SDMMC_CLKSHIFT ClockShift );
NX_SDMMC_CLKSHIFT   NX_SDMMC_GetSampleClockShiftPhase( U32 ModuleIndex );
void    NX_SDMMC_SetDriveClockDelay( U32 ModuleIndex, U32 Delay );
U32     NX_SDMMC_GetDriveClockDelay( U32 ModuleIndex );
void    NX_SDMMC_SetSampleClockDelay( U32 ModuleIndex, U32 Delay );
U32     NX_SDMMC_GetSampleClockDelay( U32 ModuleIndex );

void    NX_SDMMC_SetDataTimeOut( U32 ModuleIndex, U32 dwTimeOut );
U32     NX_SDMMC_GetDataTimeOut( U32 ModuleIndex );
void    NX_SDMMC_SetResponseTimeOut( U32 ModuleIndex, U32 dwTimeOut );
U32     NX_SDMMC_GetResponseTimeOut( U32 ModuleIndex );

void    NX_SDMMC_SetDataBusWidth( U32 ModuleIndex, U32 width );
U32     NX_SDMMC_GetDataBusWidth( U32 ModuleIndex );

void    NX_SDMMC_SetBlockSize( U32 ModuleIndex, U32 SizeInByte );
U32     NX_SDMMC_GetBlockSize( U32 ModuleIndex );
void    NX_SDMMC_SetByteCount( U32 ModuleIndex, U32 SizeInByte );
U32     NX_SDMMC_GetByteCount( U32 ModuleIndex );

void    NX_SDMMC_SetCommandArgument( U32 ModuleIndex, U32 argument );
void    NX_SDMMC_SetCommand( U32 ModuleIndex, U32 Cmd, U32 flag );
void    NX_SDMMC_StartCommand( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsCommandBusy( U32 ModuleIndex );

U32     NX_SDMMC_GetShortResponse( U32 ModuleIndex );
void    NX_SDMMC_GetLongResponse( U32 ModuleIndex, U32 *pLongResponse );
U32     NX_SDMMC_GetAutoStopResponse( U32 ModuleIndex );
U32     NX_SDMMC_GetResponseIndex( U32 ModuleIndex );

void    NX_SDMMC_SetFIFORxThreshold( U32 ModuleIndex, U32 Threshold );
U32     NX_SDMMC_GetFIFORxThreshold( U32 ModuleIndex );
void    NX_SDMMC_SetFIFOTxThreshold( U32 ModuleIndex, U32 Threshold );
U32     NX_SDMMC_GetFIFOTxThreshold( U32 ModuleIndex );


U32     NX_SDMMC_GetFIFOCount( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsFIFOFull( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsFIFOEmpty( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsFIFOTxThreshold( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsFIFORxThreshold( U32 ModuleIndex );

U32     NX_SDMMC_GetDataTransferSize( U32 ModuleIndex );
U32     NX_SDMMC_GetFIFOTransferSize( U32 ModuleIndex );

void    NX_SDMMC_SetData( U32 ModuleIndex, U32 dwData );
U32     NX_SDMMC_GetData( U32 ModuleIndex );
void    NX_SDMMC_SetData32( U32 ModuleIndex, const U32 *pdwData );
void    NX_SDMMC_GetData32( U32 ModuleIndex, U32 *pdwData );
volatile U32*   NX_SDMMC_GetDataPointer( U32 ModuleIndex );

CBOOL   NX_SDMMC_MakeBIU( NX_SDMMC_BIUConfig *pBIUC, NX_SDMMC_BIU *pBIU );
void    NX_SDMMC_GetDataBIU( U32 ModuleIndex, NX_SDMMC_BIU *pBIU );
void    NX_SDMMC_SetDataBIU( U32 ModuleIndex, NX_SDMMC_BIU *pBIU );
void    NX_SDMMC_PollDemand( U32 ModuleIndex );

U32     NX_SDMMC_GetDMACStatus( U32 ModuleIndex );
void    NX_SDMMC_SetDMACStatus( U32 ModuleIndex, U32 SetData );
void    NX_SDMMC_SetDMACIntEnable( U32 ModuleIndex, U32 IntFlag );
U32     NX_SDMMC_GetDMACIntEnable( U32 ModuleIndex );
CBOOL   NX_SDMMC_GetDMACResetStatus( U32 ModuleIndex );
void    NX_SDMMC_SetDMACBurstLength( U32 ModuleIndex, U32 BurstLength );
void    NX_SDMMC_SetIDMACEnable( U32 ModuleIndex, CBOOL bEnable );
CBOOL   NX_SDMMC_GetIDMACEnable( U32 ModuleIndex );

void    NX_SDMMC_SetDescSkipLen( U32 ModuleIndex, U32 uLength );
U32     NX_SDMMC_GetDescSkipLen( U32 ModuleIndex );
void    NX_SDMMC_ResetIDMAC( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsResetIDMAC( U32 ModuleIndex );
void    NX_SDMMC_SetDebounce( U32 ModuleIndex, U32 uDebounce );
U32     NX_SDMMC_GetDebounce( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsIDMACSupported( U32 ModuleIndex );

void    NX_SDMMC_SetCardVoltage( U32 ModuleIndex, U32 VolBase, U32 VolOffset);
U32     NX_SDMMC_GetCardVoltage( U32 ModuleIndex, U32 VolBase);

CBOOL   NX_SDMMC_IsDataTransferBusy( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsCardDataBusy( U32 ModuleIndex );
CBOOL   NX_SDMMC_IsCardPresent( U32 ModuleIndex );

void    NX_SDMMC_SetDDRMode( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_SDMMC_GetDDRMode( U32 ModuleIndex );
void    NX_SDMMC_SetVoltageMode( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_SDMMC_GetVoltageMode( U32 ModuleIndex );

NX_SDMMC_CMDFSM NX_SDMMC_GetCommandFSM( U32 ModuleIndex );
//@}


#ifdef  __cplusplus
}
#endif

#endif  // __NX_SDMMC_H__

