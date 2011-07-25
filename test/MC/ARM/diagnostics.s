@ RUN: not llvm-mc -triple=armv7-apple-darwin < %s 2> %t
@ RUN: FileCheck --check-prefix=CHECK-ERRORS < %t %s

@ Check for various assembly diagnostic messages on invalid input.

@ 's' bit on an instruction that can't accept it.
        mlss r1, r2, r3, r4
@ CHECK-ERRORS: error: instruction 'mls' can not set flags,
@ CHECK-ERRORS: but 's' suffix specified


        @ Out of range shift immediate values.
        adc r1, r2, r3, lsl #invalid
        adc r4, r5, r6, lsl #-1
        adc r4, r5, r6, lsl #32
        adc r4, r5, r6, lsr #-1
        adc r4, r5, r6, lsr #33
        adc r4, r5, r6, asr #-1
        adc r4, r5, r6, asr #33
        adc r4, r5, r6, ror #-1
        adc r4, r5, r6, ror #32

@ CHECK-ERRORS: error: invalid immediate shift value
@ CHECK-ERRORS:         adc r1, r2, r3, lsl #invalid
@ CHECK-ERRORS:                              ^
@ CHECK-ERRORS: error: immediate shift value out of range
@ CHECK-ERRORS:         adc r4, r5, r6, lsl #-1
@ CHECK-ERRORS:                              ^
@ CHECK-ERRORS: error: immediate shift value out of range
@ CHECK-ERRORS:         adc r4, r5, r6, lsl #32
@ CHECK-ERRORS:                              ^
@ CHECK-ERRORS: error: immediate shift value out of range
@ CHECK-ERRORS:         adc r4, r5, r6, lsr #-1
@ CHECK-ERRORS:                              ^
@ CHECK-ERRORS: error: immediate shift value out of range
@ CHECK-ERRORS:         adc r4, r5, r6, lsr #33
@ CHECK-ERRORS:                              ^
@ CHECK-ERRORS: error: immediate shift value out of range
@ CHECK-ERRORS:         adc r4, r5, r6, asr #-1
@ CHECK-ERRORS:                              ^
@ CHECK-ERRORS: error: immediate shift value out of range
@ CHECK-ERRORS:         adc r4, r5, r6, asr #33
@ CHECK-ERRORS:                              ^
@ CHECK-ERRORS: error: immediate shift value out of range
@ CHECK-ERRORS:         adc r4, r5, r6, ror #-1
@ CHECK-ERRORS:                              ^
@ CHECK-ERRORS: error: immediate shift value out of range
@ CHECK-ERRORS:         adc r4, r5, r6, ror #32


        @ Out of range 16-bit immediate on BKPT
        bkpt #65536

@ CHECK-ERRORS: error: invalid operand for instruction

        @ Out of range 4 and 3 bit immediates on CDP[2]

        @ Out of range immediates for CDP/CDP2
        cdp  p7, #2, c1, c1, c1, #8
        cdp  p7, #1, c1, c1, c1, #8
        cdp2  p7, #2, c1, c1, c1, #8
        cdp2  p7, #1, c1, c1, c1, #8

@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction

        @ Out of range immediates for DBG
        dbg #-1
        dbg #16

@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@  Double-check that we're synced up with the right diagnostics.
@ CHECK-ERRORS: dbg #16

        @ Out of range immediate for MCR/MCR2/MCRR/MCRR2
        mcr  p7, #8, r5, c1, c1, #4
        mcr  p7, #2, r5, c1, c1, #8
        mcr2  p7, #8, r5, c1, c1, #4
        mcr2  p7, #1, r5, c1, c1, #8
        mcrr  p7, #16, r5, r4, c1
        mcrr2  p7, #16, r5, r4, c1
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction


        @ Out of range immediate for MOV
        movw r9, 0x10000
@ CHECK-ERRORS: error: invalid operand for instruction

        @ Invalid 's' bit usage for MOVW
        movs r6, #0xffff
        movwseq r9, #0xffff
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: instruction 'movw' can not set flags, but 's' suffix specified

        @ Out of range immediate for MOVT
        movt r9, 0x10000
@ CHECK-ERRORS: error: invalid operand for instruction

        @ Out of range immediates for MRC/MRC2/MRRC/MRRC2
        mrc  p14, #8, r1, c1, c2, #4
        mrc  p14, #1, r1, c1, c2, #8
        mrc2  p14, #8, r1, c1, c2, #4
        mrc2  p14, #0, r1, c1, c2, #9
        mrrc  p7, #16, r5, r4, c1
        mrrc2  p7, #17, r5, r4, c1
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction
@ CHECK-ERRORS: error: invalid operand for instruction

        @ Shifter operand validation for PKH instructions.
        pkhbt r2, r2, r3, lsl #-1
        pkhbt r2, r2, r3, lsl #32
        pkhtb r2, r2, r3, asr #0
        pkhtb r2, r2, r3, asr #33
        pkhbt r2, r2, r3, asr #3
        pkhtb r2, r2, r3, lsl #3

@ CHECK: error: immediate value out of range
@ CHECK:         pkhbt r2, r2, r3, lsl #-1
@ CHECK:                                ^
@ CHECK: error: immediate value out of range
@ CHECK:         pkhbt r2, r2, r3, lsl #32
@ CHECK:                                ^
@ CHECK: error: immediate value out of range
@ CHECK:         pkhtb r2, r2, r3, asr #0
@ CHECK:                                ^
@ CHECK: error: immediate value out of range
@ CHECK:         pkhtb r2, r2, r3, asr #33
@ CHECK:                                ^
@ CHECK: error: lsl operand expected.
@ CHECK:         pkhbt r2, r2, r3, asr #3
@ CHECK:                           ^
@ CHECK: error: asr operand expected.
@ CHECK:         pkhtb r2, r2, r3, lsl #3
@ CHECK:                           ^


        @ bad values for SETEND
        setendne be
        setend me
        setend 1

@ CHECK: error: instruction 'setend' is not predicable, but condition code specified
@ CHECK:         setendne be
@ CHECK:         ^
@ CHECK: error: 'be' or 'le' operand expected
@ CHECK:         setend me
@ CHECK:                  ^
@ CHECK: error: 'be' or 'le' operand expected
@ CHECK:         setend 1
@ CHECK:                ^


        @ Out of range immediates and bad shift types for SSAT
	ssat	r8, #0, r10, lsl #8
	ssat	r8, #33, r10, lsl #8
	ssat	r8, #1, r10, lsl #-1
	ssat	r8, #1, r10, lsl #32
	ssat	r8, #1, r10, asr #0
	ssat	r8, #1, r10, asr #33
        ssat    r8, #1, r10, lsr #5
        ssat    r8, #1, r10, lsl fred
        ssat    r8, #1, r10, lsl #fred

@ CHECK: error: invalid operand for instruction
@ CHECK: 	ssat	r8, #0, r10, lsl #8
@ CHECK: 	    	    ^
@ CHECK: error: invalid operand for instruction
@ CHECK: 	ssat	r8, #33, r10, lsl #8
@ CHECK: 	    	    ^
@ CHECK: error: 'lsr' shift amount must be in range [0,31]
@ CHECK: 	ssat	r8, #1, r10, lsl #-1
@ CHECK: 	    	                  ^
@ CHECK: error: 'lsr' shift amount must be in range [0,31]
@ CHECK: 	ssat	r8, #1, r10, lsl #32
@ CHECK: 	    	                  ^
@ CHECK: error: 'asr' shift amount must be in range [1,32]
@ CHECK: 	ssat	r8, #1, r10, asr #0
@ CHECK: 	    	                  ^
@ CHECK: error: 'asr' shift amount must be in range [1,32]
@ CHECK: 	ssat	r8, #1, r10, asr #33
@ CHECK: 	    	                  ^
@ CHECK: error: shift operator 'asr' or 'lsl' expected
@ CHECK:         ssat    r8, #1, r10, lsr #5
@ CHECK:                              ^
@ CHECK: error: '#' expected
@ CHECK:         ssat    r8, #1, r10, lsl fred
@ CHECK:                                  ^
@ CHECK: error: shift amount must be an immediate
@ CHECK:         ssat    r8, #1, r10, lsl #fred
@ CHECK:                                   ^
