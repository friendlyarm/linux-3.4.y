#include <linux/linkage.h>
#include <asm/assembler.h>


ENTRY(__nfc_read_burst)
			STP		X4, X5, [SP, #-16]!


			mov		w3, w2
			b		R_COMPAREx4
R_LOOPx4:
			LDP		X4, X5, [X0]
			STP		X4, X5, [X1], #16

			sub		w3, w3, #16
R_COMPAREx4:
			cmp		w3, 16
			bgt		R_LOOPx4
			b		R_COMPAREx1	
R_LOOPx1:
			LDR		w4, [x0]
			STR		w4, [x1]
			ADD		x1, x1, #4

			sub		w3, w3, #4
R_COMPAREx1:
			cmp		w3, 4
			bgt		R_LOOPx1
			b		R_COMPAREx0
R_LOOPx0:
			LDRB	w4, [x0]
			STRB	w4, [x1]
			ADD		x1, x1, #1

			sub		w3, w3, #1
R_COMPAREx0:
			cmp		w3, wzr
			bgt		R_LOOPx0


			LDP		X4, X5, [SP], #16
			RET
ENDPROC(__nfc_read_burst)




ENTRY(__nfc_write_burst)
			STP		X4, X5, [SP, #-16]!


			mov		w3, w2
			b		W_COMPAREx4
W_LOOPx4:
			LDP		X4, X5, [X1], #16
			STP		X4, X5, [X0]

			sub		w3, w3, #16
W_COMPAREx4:
			cmp		w3, 16
			bgt		W_LOOPx4
			b		W_COMPAREx1	
W_LOOPx1:
			LDR		w4, [x1]
			STR		w4, [x0]
			ADD		x1, x1, #4

			sub		w3, w3, #4
W_COMPAREx1:
			cmp		w3, 4
			bgt		W_LOOPx1
			b		W_COMPAREx0
W_LOOPx0:
			LDRB	w4, [x1]
			STRB	w4, [x0]
			ADD		x1, x1, #1

			sub		w3, w3, #1
W_COMPAREx0:
			cmp		w3, wzr
			bgt		W_LOOPx0


			LDP		X4, X5, [SP], #16
			RET
ENDPROC(__nfc_write_burst)
