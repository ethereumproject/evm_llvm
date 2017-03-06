; NOTE: Assertions have been autogenerated by update_test_checks.py
; RUN: opt < %s -instsimplify -S | FileCheck %s

define i32 @select1(i32 %x, i1 %b) {
; CHECK-LABEL: @select1(
; CHECK-NEXT:    ret i32 0
;
  %rhs = select i1 %b, i32 %x, i32 1
  %rem = srem i32 %x, %rhs
  ret i32 %rem
}

define i32 @select2(i32 %x, i1 %b) {
; CHECK-LABEL: @select2(
; CHECK-NEXT:    ret i32 0
;
  %rhs = select i1 %b, i32 %x, i32 1
  %rem = urem i32 %x, %rhs
  ret i32 %rem
}

define i32 @rem1(i32 %x, i32 %n) {
; CHECK-LABEL: @rem1(
; CHECK-NEXT:    [[MOD:%.*]] = srem i32 %x, %n
; CHECK-NEXT:    ret i32 [[MOD]]
;
  %mod = srem i32 %x, %n
  %mod1 = srem i32 %mod, %n
  ret i32 %mod1
}

define i32 @rem2(i32 %x, i32 %n) {
; CHECK-LABEL: @rem2(
; CHECK-NEXT:    [[MOD:%.*]] = urem i32 %x, %n
; CHECK-NEXT:    ret i32 [[MOD]]
;
  %mod = urem i32 %x, %n
  %mod1 = urem i32 %mod, %n
  ret i32 %mod1
}

define i32 @rem3(i32 %x, i32 %n) {
; CHECK-LABEL: @rem3(
; CHECK-NEXT:    [[MOD:%.*]] = srem i32 %x, %n
; CHECK-NEXT:    [[MOD1:%.*]] = urem i32 [[MOD]], %n
; CHECK-NEXT:    ret i32 [[MOD1]]
;
  %mod = srem i32 %x, %n
  %mod1 = urem i32 %mod, %n
  ret i32 %mod1
}

declare i32 @external()

define i32 @rem4() {
; CHECK-LABEL: @rem4(
; CHECK-NEXT:    [[CALL:%.*]] = call i32 @external(), !range !0
; CHECK-NEXT:    ret i32 [[CALL]]
;
  %call = call i32 @external(), !range !0
  %urem = urem i32 %call, 3
  ret i32 %urem
}

!0 = !{i32 0, i32 3}
