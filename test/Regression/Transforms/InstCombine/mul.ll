; This test makes sure that mul instructions are properly eliminated.
;

; RUN: if as < %s | opt -instcombine | dis | grep mul
; RUN: then exit 1
; RUN: else exit 0
; RUN: fi

implementation

int %test1(int %A) {
	%B = mul int %A, 1
	ret int %B
}

int %test2(int %A) {
	%B = mul int %A, 2   ; Should convert to an add instruction
	ret int %B
}

int %test3(int %A) {
	%B = mul int %A, 0   ; This should disappear entirely
	ret int %B
}

double %test4(double %A) {
	%B = mul double 1.0, %A   ; This is safe for FP
	ret double %B
}

int %test5(int %A) {
	%B = mul int %A, 8
	ret int %B
}
