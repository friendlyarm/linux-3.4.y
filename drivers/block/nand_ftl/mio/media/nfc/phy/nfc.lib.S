#include <linux/linkage.h>
#include <asm/assembler.h>

  .globl burst_read
  .align 4,0x90


burst_read:								
            PUSH	{r4-r7}
            MOVS    r3, r2, LSR #2      

            BEQ     copywords           
quadcopy:
            LDMIA   r1,  {r4-r7}        
            STMIA   r0!, {r4-r7}        
            SUBS    r3, r3, #1          
            BNE     quadcopy            
copywords: 
            ANDS    r2, r2, #3          
            BEQ     stop                
wordcopy:
            LDR     r3, [r1], #4        
            STR     r3, [r0], #4        
            SUBS    r2, r2, #1          
            BNE     wordcopy            
stop:
            POP		{r4-r7}
			MOV		pc, lr

.type burst_read, #function




ENTRY(__nfc_readsl)
		teq	r2, #0		@ do we have to check for the zero len?
		moveq	pc, lr
		ands	ip, r1, #3
		bne	3f

		subs	r2, r2, #4
		bmi	2f
		stmfd	sp!, {r4, lr}
1:		ldmia	r0, {r3, r4, ip, lr}
		subs	r2, r2, #4
		stmia	r1!, {r3, r4, ip, lr}
		bpl	1b
		ldmfd	sp!, {r4, lr}
2:		movs	r2, r2, lsl #31
		ldrcs	r3, [r0, #0]
		ldrcs	ip, [r0, #0]
		stmcsia	r1!, {r3, ip}
		ldrne	r3, [r0, #0]
		strne	r3, [r1, #0]
		mov	pc, lr

3:		ldr	r3, [r0]
		cmp	ip, #2
		mov	ip, r3, get_byte_0
		strb	ip, [r1], #1
		bgt	6f
		mov	ip, r3, get_byte_1
		strb	ip, [r1], #1
		beq	5f
		mov	ip, r3, get_byte_2
		strb	ip, [r1], #1

4:		subs	r2, r2, #1
		mov	ip, r3, pull #24
		ldrne	r3, [r0]
		orrne	ip, ip, r3, push #8
		strne	ip, [r1], #4
		bne	4b
		b	8f

5:		subs	r2, r2, #1
		mov	ip, r3, pull #16
		ldrne	r3, [r0]
		orrne	ip, ip, r3, push #16
		strne	ip, [r1], #4
		bne	5b
		b	7f

6:		subs	r2, r2, #1
		mov	ip, r3, pull #8
		ldrne	r3, [r0]
		orrne	ip, ip, r3, push #24
		strne	ip, [r1], #4
		bne	6b

		mov	r3, ip, get_byte_2
		strb	r3, [r1, #2]
7:		mov	r3, ip, get_byte_1
		strb	r3, [r1, #1]
8:		mov	r3, ip, get_byte_0
		strb	r3, [r1, #0]
		mov	pc, lr
ENDPROC(__nfc_readsl)



ENTRY(__nfc_writesl)
		teq	r2, #0		@ do we have to check for the zero len?
		moveq	pc, lr
		ands	ip, r1, #3
		bne	3f

		subs	r2, r2, #4
		bmi	2f
		stmfd	sp!, {r4, lr}
1:		ldmia	r1!, {r3, r4, ip, lr}
		subs	r2, r2, #4
		stmia	r0, {r3, r4, ip, lr}
		bpl	1b
		ldmfd	sp!, {r4, lr}
2:		movs	r2, r2, lsl #31
		ldmcsia	r1!, {r3, ip}
		strcs	r3, [r0, #0]
		ldrne	r3, [r1, #0]
		strcs	ip, [r0, #0]
		strne	r3, [r0, #0]
		mov	pc, lr

3:		bic	r1, r1, #3
		ldr	r3, [r1], #4
		cmp	ip, #2
		blt	5f
		bgt	6f

4:		mov	ip, r3, pull #16
		ldr	r3, [r1], #4
		subs	r2, r2, #1
		orr	ip, ip, r3, push #16
		str	ip, [r0]
		bne	4b
		mov	pc, lr

5:		mov	ip, r3, pull #8
		ldr	r3, [r1], #4
		subs	r2, r2, #1
		orr	ip, ip, r3, push #24
		str	ip, [r0]
		bne	5b
		mov	pc, lr

6:		mov	ip, r3, pull #24
		ldr	r3, [r1], #4
		subs	r2, r2, #1
		orr	ip, ip, r3, push #8
		str	ip, [r0]
		bne	6b
		mov	pc, lr
ENDPROC(__nfc_writesl)






.Linsb_align:	rsb	ip, ip, #4
		cmp	ip, r2
		movgt	ip, r2
		cmp	ip, #2
		ldrb	r3, [r0]
		strb	r3, [r1], #1
		ldrgeb	r3, [r0]
		strgeb	r3, [r1], #1
		ldrgtb	r3, [r0]
		strgtb	r3, [r1], #1
		subs	r2, r2, ip
		bne	.Linsb_aligned

ENTRY(__nfc_readsb)
		teq	r2, #0		@ do we have to check for the zero len?
		moveq	pc, lr
		ands	ip, r1, #3
		bne	.Linsb_align

.Linsb_aligned:	stmfd	sp!, {r4 - r6, lr}

		subs	r2, r2, #16
		bmi	.Linsb_no_16

.Linsb_16_lp:	ldrb	r3, [r0]
		ldrb	r4, [r0]
		ldrb	r5, [r0]
		mov	r3, r3,     put_byte_0
		ldrb	r6, [r0]
		orr	r3, r3, r4, put_byte_1
		ldrb	r4, [r0]
		orr	r3, r3, r5, put_byte_2
		ldrb	r5, [r0]
		orr	r3, r3, r6, put_byte_3
		ldrb	r6, [r0]
		mov	r4, r4,     put_byte_0
		ldrb	ip, [r0]
		orr	r4, r4, r5, put_byte_1
		ldrb	r5, [r0]
		orr	r4, r4, r6, put_byte_2
		ldrb	r6, [r0]
		orr	r4, r4, ip, put_byte_3
		ldrb	ip, [r0]
		mov	r5, r5,     put_byte_0
		ldrb	lr, [r0]
		orr	r5, r5, r6, put_byte_1
		ldrb	r6, [r0]
		orr	r5, r5, ip, put_byte_2
		ldrb	ip, [r0]
		orr	r5, r5, lr, put_byte_3
		ldrb	lr, [r0]
		mov	r6, r6,     put_byte_0
		orr	r6, r6, ip, put_byte_1
		ldrb	ip, [r0]
		orr	r6, r6, lr, put_byte_2
		orr	r6, r6, ip, put_byte_3
		stmia	r1!, {r3 - r6}

		subs	r2, r2, #16
		bpl	.Linsb_16_lp

		tst	r2, #15
		ldmeqfd	sp!, {r4 - r6, pc}

.Linsb_no_16:	tst	r2, #8
		beq	.Linsb_no_8

		ldrb	r3, [r0]
		ldrb	r4, [r0]
		ldrb	r5, [r0]
		mov	r3, r3,     put_byte_0
		ldrb	r6, [r0]
		orr	r3, r3, r4, put_byte_1
		ldrb	r4, [r0]
		orr	r3, r3, r5, put_byte_2
		ldrb	r5, [r0]
		orr	r3, r3, r6, put_byte_3
		ldrb	r6, [r0]
		mov	r4, r4,     put_byte_0
		ldrb	ip, [r0]
		orr	r4, r4, r5, put_byte_1
		orr	r4, r4, r6, put_byte_2
		orr	r4, r4, ip, put_byte_3
		stmia	r1!, {r3, r4}

.Linsb_no_8:	tst	r2, #4
		beq	.Linsb_no_4

		ldrb	r3, [r0]
		ldrb	r4, [r0]
		ldrb	r5, [r0]
		ldrb	r6, [r0]
		mov	r3, r3,     put_byte_0
		orr	r3, r3, r4, put_byte_1
		orr	r3, r3, r5, put_byte_2
		orr	r3, r3, r6, put_byte_3
		str	r3, [r1], #4

.Linsb_no_4:	ands	r2, r2, #3
		ldmeqfd	sp!, {r4 - r6, pc}

		cmp	r2, #2
		ldrb	r3, [r0]
		strb	r3, [r1], #1
		ldrgeb	r3, [r0]
		strgeb	r3, [r1], #1
		ldrgtb	r3, [r0]
		strgtb	r3, [r1]

		ldmfd	sp!, {r4 - r6, pc}
ENDPROC(__nfc_readsb)



		.macro	outword, rd
#ifndef __ARMEB__
		strb	\rd, [r0]
		mov	\rd, \rd, lsr #8
		strb	\rd, [r0]
		mov	\rd, \rd, lsr #8
		strb	\rd, [r0]
		mov	\rd, \rd, lsr #8
		strb	\rd, [r0]
#else
		mov	lr, \rd, lsr #24
		strb	lr, [r0]
		mov	lr, \rd, lsr #16
		strb	lr, [r0]
		mov	lr, \rd, lsr #8
		strb	lr, [r0]
		strb	\rd, [r0]
#endif
		.endm

.Loutsb_align:	rsb	ip, ip, #4
		cmp	ip, r2
		movgt	ip, r2
		cmp	ip, #2
		ldrb	r3, [r1], #1
		strb	r3, [r0]
		ldrgeb	r3, [r1], #1
		strgeb	r3, [r0]
		ldrgtb	r3, [r1], #1
		strgtb	r3, [r0]
		subs	r2, r2, ip
		bne	.Loutsb_aligned

ENTRY(__nfc_writesb)
		teq	r2, #0		@ do we have to check for the zero len?
		moveq	pc, lr
		ands	ip, r1, #3
		bne	.Loutsb_align

.Loutsb_aligned:
		stmfd	sp!, {r4, r5, lr}

		subs	r2, r2, #16
		bmi	.Loutsb_no_16

.Loutsb_16_lp:	ldmia	r1!, {r3, r4, r5, ip}
		outword	r3
		outword	r4
		outword	r5
		outword	ip
		subs	r2, r2, #16
		bpl	.Loutsb_16_lp

		tst	r2, #15
		ldmeqfd	sp!, {r4, r5, pc}

.Loutsb_no_16:	tst	r2, #8
		beq	.Loutsb_no_8

		ldmia	r1!, {r3, r4}
		outword	r3
		outword	r4

.Loutsb_no_8:	tst	r2, #4
		beq	.Loutsb_no_4

		ldr	r3, [r1], #4
		outword	r3

.Loutsb_no_4:	ands	r2, r2, #3
		ldmeqfd	sp!, {r4, r5, pc}

		cmp	r2, #2
		ldrb	r3, [r1], #1
		strb	r3, [r0]
		ldrgeb	r3, [r1], #1
		strgeb	r3, [r0]
		ldrgtb	r3, [r1]
		strgtb	r3, [r0]

		ldmfd	sp!, {r4, r5, pc}
ENDPROC(__nfc_writesb)
