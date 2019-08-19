# NOTE: Assertions have been autogenerated by utils/update_mca_test_checks.py
# RUN: llvm-mca -mtriple=x86_64-unknown-unknown -mcpu=znver1 -instruction-tables < %s | FileCheck %s

cmpxchg8b  (%rax)
cmpxchg16b (%rax)
lock cmpxchg8b  (%rax)
lock cmpxchg16b (%rax)

cmpxchgb  %bl, %cl
cmpxchgw  %bx, %cx
cmpxchgl  %ebx, %ecx
cmpxchgq  %rbx, %rcx

cmpxchgb  %bl, (%rsi)
cmpxchgw  %bx, (%rsi)
cmpxchgl  %ebx, (%rsi)
cmpxchgq  %rbx, (%rsi)

lock cmpxchgb  %bl, (%rsi)
lock cmpxchgw  %bx, (%rsi)
lock cmpxchgl  %ebx, (%rsi)
lock cmpxchgq  %rbx, (%rsi)

# CHECK:      Instruction Info:
# CHECK-NEXT: [1]: #uOps
# CHECK-NEXT: [2]: Latency
# CHECK-NEXT: [3]: RThroughput
# CHECK-NEXT: [4]: MayLoad
# CHECK-NEXT: [5]: MayStore
# CHECK-NEXT: [6]: HasSideEffects (U)

# CHECK:      [1]    [2]    [3]    [4]    [5]    [6]    Instructions:
# CHECK-NEXT:  18     1     0.50    *      *            cmpxchg8b	(%rax)
# CHECK-NEXT:  1      100   0.25    *      *            cmpxchg16b	(%rax)
# CHECK-NEXT:  18     1     0.50    *      *            lock		cmpxchg8b	(%rax)
# CHECK-NEXT:  1      100   0.25    *      *            lock		cmpxchg16b	(%rax)
# CHECK-NEXT:  1      1     0.25                        cmpxchgb	%bl, %cl
# CHECK-NEXT:  1      1     0.25                        cmpxchgw	%bx, %cx
# CHECK-NEXT:  1      1     0.25                        cmpxchgl	%ebx, %ecx
# CHECK-NEXT:  1      1     0.25                        cmpxchgq	%rbx, %rcx
# CHECK-NEXT:  5      8     0.50    *      *            cmpxchgb	%bl, (%rsi)
# CHECK-NEXT:  5      8     0.50    *      *            cmpxchgw	%bx, (%rsi)
# CHECK-NEXT:  5      8     0.50    *      *            cmpxchgl	%ebx, (%rsi)
# CHECK-NEXT:  5      8     0.50    *      *            cmpxchgq	%rbx, (%rsi)
# CHECK-NEXT:  5      8     0.50    *      *            lock		cmpxchgb	%bl, (%rsi)
# CHECK-NEXT:  5      8     0.50    *      *            lock		cmpxchgw	%bx, (%rsi)
# CHECK-NEXT:  5      8     0.50    *      *            lock		cmpxchgl	%ebx, (%rsi)
# CHECK-NEXT:  5      8     0.50    *      *            lock		cmpxchgq	%rbx, (%rsi)

# CHECK:      Resources:
# CHECK-NEXT: [0]   - ZnAGU0
# CHECK-NEXT: [1]   - ZnAGU1
# CHECK-NEXT: [2]   - ZnALU0
# CHECK-NEXT: [3]   - ZnALU1
# CHECK-NEXT: [4]   - ZnALU2
# CHECK-NEXT: [5]   - ZnALU3
# CHECK-NEXT: [6]   - ZnDivider
# CHECK-NEXT: [7]   - ZnFPU0
# CHECK-NEXT: [8]   - ZnFPU1
# CHECK-NEXT: [9]   - ZnFPU2
# CHECK-NEXT: [10]  - ZnFPU3
# CHECK-NEXT: [11]  - ZnMultiplier

# CHECK:      Resource pressure per iteration:
# CHECK-NEXT: [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    [9]    [10]   [11]
# CHECK-NEXT: 5.00   5.00   3.50   3.50   3.50   3.50    -      -      -      -      -      -

# CHECK:      Resource pressure by instruction:
# CHECK-NEXT: [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    [9]    [10]   [11]   Instructions:
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     cmpxchg8b	(%rax)
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -      -      -      -     cmpxchg16b	(%rax)
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     lock		cmpxchg8b	(%rax)
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -      -      -      -     lock		cmpxchg16b	(%rax)
# CHECK-NEXT:  -      -     0.25   0.25   0.25   0.25    -      -      -      -      -      -     cmpxchgb	%bl, %cl
# CHECK-NEXT:  -      -     0.25   0.25   0.25   0.25    -      -      -      -      -      -     cmpxchgw	%bx, %cx
# CHECK-NEXT:  -      -     0.25   0.25   0.25   0.25    -      -      -      -      -      -     cmpxchgl	%ebx, %ecx
# CHECK-NEXT:  -      -     0.25   0.25   0.25   0.25    -      -      -      -      -      -     cmpxchgq	%rbx, %rcx
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     cmpxchgb	%bl, (%rsi)
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     cmpxchgw	%bx, (%rsi)
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     cmpxchgl	%ebx, (%rsi)
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     cmpxchgq	%rbx, (%rsi)
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     lock		cmpxchgb	%bl, (%rsi)
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     lock		cmpxchgw	%bx, (%rsi)
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     lock		cmpxchgl	%ebx, (%rsi)
# CHECK-NEXT: 0.50   0.50   0.25   0.25   0.25   0.25    -      -      -      -      -      -     lock		cmpxchgq	%rbx, (%rsi)
