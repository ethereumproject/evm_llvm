; RUN: llc -march=r600 -mcpu=SI -verify-machineinstrs < %s | FileCheck -check-prefix=SI -check-prefix=FUNC %s

declare i32 @llvm.r600.read.tidig.x() #1
declare float @llvm.fabs.f32(float) #1

; FUNC-LABEL: @commute_add_imm_fabs_f32
; SI: buffer_load_dword [[X:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64{{$}}
; SI: v_add_f32_e64 [[REG:v[0-9]+]], 2.0, |[[X]]|
; SI-NEXT: buffer_store_dword [[REG]]
define void @commute_add_imm_fabs_f32(float addrspace(1)* %out, float addrspace(1)* %in) #0 {
  %tid = call i32 @llvm.r600.read.tidig.x() #1
  %gep.0 = getelementptr float addrspace(1)* %in, i32 %tid
  %x = load float addrspace(1)* %gep.0
  %x.fabs = call float @llvm.fabs.f32(float %x) #1
  %z = fadd float 2.0, %x.fabs
  store float %z, float addrspace(1)* %out
  ret void
}

; FUNC-LABEL: @commute_mul_imm_fneg_fabs_f32
; SI: buffer_load_dword [[X:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64{{$}}
; SI: v_mul_f32_e64 [[REG:v[0-9]+]], -4.0, |[[X]]|
; SI-NEXT: buffer_store_dword [[REG]]
define void @commute_mul_imm_fneg_fabs_f32(float addrspace(1)* %out, float addrspace(1)* %in) #0 {
  %tid = call i32 @llvm.r600.read.tidig.x() #1
  %gep.0 = getelementptr float addrspace(1)* %in, i32 %tid
  %x = load float addrspace(1)* %gep.0
  %x.fabs = call float @llvm.fabs.f32(float %x) #1
  %x.fneg.fabs = fsub float -0.000000e+00, %x.fabs
  %z = fmul float 4.0, %x.fneg.fabs
  store float %z, float addrspace(1)* %out
  ret void
}

; FUNC-LABEL: @commute_mul_imm_fneg_f32
; SI: buffer_load_dword [[X:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64{{$}}
; SI: v_mul_f32_e32 [[REG:v[0-9]+]], -4.0, [[X]]
; SI-NEXT: buffer_store_dword [[REG]]
define void @commute_mul_imm_fneg_f32(float addrspace(1)* %out, float addrspace(1)* %in) #0 {
  %tid = call i32 @llvm.r600.read.tidig.x() #1
  %gep.0 = getelementptr float addrspace(1)* %in, i32 %tid
  %x = load float addrspace(1)* %gep.0
  %x.fneg = fsub float -0.000000e+00, %x
  %z = fmul float 4.0, %x.fneg
  store float %z, float addrspace(1)* %out
  ret void
}

; FIXME: Should use SGPR for literal.
; FUNC-LABEL: @commute_add_lit_fabs_f32
; SI: buffer_load_dword [[X:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64{{$}}
; SI: v_mov_b32_e32 [[K:v[0-9]+]], 0x44800000
; SI: v_add_f32_e64 [[REG:v[0-9]+]], |[[X]]|, [[K]]
; SI-NEXT: buffer_store_dword [[REG]]
define void @commute_add_lit_fabs_f32(float addrspace(1)* %out, float addrspace(1)* %in) #0 {
  %tid = call i32 @llvm.r600.read.tidig.x() #1
  %gep.0 = getelementptr float addrspace(1)* %in, i32 %tid
  %x = load float addrspace(1)* %gep.0
  %x.fabs = call float @llvm.fabs.f32(float %x) #1
  %z = fadd float 1024.0, %x.fabs
  store float %z, float addrspace(1)* %out
  ret void
}

; FUNC-LABEL: @commute_add_fabs_f32
; SI-DAG: buffer_load_dword [[X:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64{{$}}
; SI-DAG: buffer_load_dword [[Y:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64 offset:0x4
; SI: v_add_f32_e64 [[REG:v[0-9]+]], [[X]], |[[Y]]|
; SI-NEXT: buffer_store_dword [[REG]]
define void @commute_add_fabs_f32(float addrspace(1)* %out, float addrspace(1)* %in) #0 {
  %tid = call i32 @llvm.r600.read.tidig.x() #1
  %gep.0 = getelementptr float addrspace(1)* %in, i32 %tid
  %gep.1 = getelementptr float addrspace(1)* %gep.0, i32 1
  %x = load float addrspace(1)* %gep.0
  %y = load float addrspace(1)* %gep.1
  %y.fabs = call float @llvm.fabs.f32(float %y) #1
  %z = fadd float %x, %y.fabs
  store float %z, float addrspace(1)* %out
  ret void
}

; FUNC-LABEL: @commute_mul_fneg_f32
; SI-DAG: buffer_load_dword [[X:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64{{$}}
; SI-DAG: buffer_load_dword [[Y:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64 offset:0x4
; SI: v_mul_f32_e64 [[REG:v[0-9]+]], [[X]], -[[Y]]
; SI-NEXT: buffer_store_dword [[REG]]
define void @commute_mul_fneg_f32(float addrspace(1)* %out, float addrspace(1)* %in) #0 {
  %tid = call i32 @llvm.r600.read.tidig.x() #1
  %gep.0 = getelementptr float addrspace(1)* %in, i32 %tid
  %gep.1 = getelementptr float addrspace(1)* %gep.0, i32 1
  %x = load float addrspace(1)* %gep.0
  %y = load float addrspace(1)* %gep.1
  %y.fneg = fsub float -0.000000e+00, %y
  %z = fmul float %x, %y.fneg
  store float %z, float addrspace(1)* %out
  ret void
}

; FUNC-LABEL: @commute_mul_fabs_fneg_f32
; SI-DAG: buffer_load_dword [[X:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64{{$}}
; SI-DAG: buffer_load_dword [[Y:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64 offset:0x4
; SI: v_mul_f32_e64 [[REG:v[0-9]+]], [[X]], -|[[Y]]|
; SI-NEXT: buffer_store_dword [[REG]]
define void @commute_mul_fabs_fneg_f32(float addrspace(1)* %out, float addrspace(1)* %in) #0 {
  %tid = call i32 @llvm.r600.read.tidig.x() #1
  %gep.0 = getelementptr float addrspace(1)* %in, i32 %tid
  %gep.1 = getelementptr float addrspace(1)* %gep.0, i32 1
  %x = load float addrspace(1)* %gep.0
  %y = load float addrspace(1)* %gep.1
  %y.fabs = call float @llvm.fabs.f32(float %y) #1
  %y.fabs.fneg = fsub float -0.000000e+00, %y.fabs
  %z = fmul float %x, %y.fabs.fneg
  store float %z, float addrspace(1)* %out
  ret void
}

; There's no reason to commute this.
; FUNC-LABEL: @commute_mul_fabs_x_fabs_y_f32
; SI-DAG: buffer_load_dword [[X:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64{{$}}
; SI-DAG: buffer_load_dword [[Y:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64 offset:0x4
; SI: v_mul_f32_e64 [[REG:v[0-9]+]], |[[X]]|, |[[Y]]|
; SI-NEXT: buffer_store_dword [[REG]]
define void @commute_mul_fabs_x_fabs_y_f32(float addrspace(1)* %out, float addrspace(1)* %in) #0 {
  %tid = call i32 @llvm.r600.read.tidig.x() #1
  %gep.0 = getelementptr float addrspace(1)* %in, i32 %tid
  %gep.1 = getelementptr float addrspace(1)* %gep.0, i32 1
  %x = load float addrspace(1)* %gep.0
  %y = load float addrspace(1)* %gep.1
  %x.fabs = call float @llvm.fabs.f32(float %x) #1
  %y.fabs = call float @llvm.fabs.f32(float %y) #1
  %z = fmul float %x.fabs, %y.fabs
  store float %z, float addrspace(1)* %out
  ret void
}

; FUNC-LABEL: @commute_mul_fabs_x_fneg_fabs_y_f32
; SI-DAG: buffer_load_dword [[X:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64{{$}}
; SI-DAG: buffer_load_dword [[Y:v[0-9]+]], {{v\[[0-9]+:[0-9]+\]}}, {{s\[[0-9]+:[0-9]+\]}}, 0 addr64 offset:0x4
; SI: v_mul_f32_e64 [[REG:v[0-9]+]], |[[X]]|, -|[[Y]]|
; SI-NEXT: buffer_store_dword [[REG]]
define void @commute_mul_fabs_x_fneg_fabs_y_f32(float addrspace(1)* %out, float addrspace(1)* %in) #0 {
  %tid = call i32 @llvm.r600.read.tidig.x() #1
  %gep.0 = getelementptr float addrspace(1)* %in, i32 %tid
  %gep.1 = getelementptr float addrspace(1)* %gep.0, i32 1
  %x = load float addrspace(1)* %gep.0
  %y = load float addrspace(1)* %gep.1
  %x.fabs = call float @llvm.fabs.f32(float %x) #1
  %y.fabs = call float @llvm.fabs.f32(float %y) #1
  %y.fabs.fneg = fsub float -0.000000e+00, %y.fabs
  %z = fmul float %x.fabs, %y.fabs.fneg
  store float %z, float addrspace(1)* %out
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
