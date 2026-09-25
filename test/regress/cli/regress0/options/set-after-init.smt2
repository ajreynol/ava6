; EXPECT: 0
; EXPECT: 1
; EXPECT: eager
; EXPECT: lazy
; EXPECT: sat
; EXPECT: 0
; EXPECT: (error "invalid call to 'setOption' for option 'preregister-mode', solver is already fully initialized")
; EXIT: 1

(get-option :verbosity)
(set-option :verbosity 1)
(get-option :verbosity)
(set-option :verbosity 0)
(get-option :preregister-mode)
(set-option :preregister-mode lazy)
(get-option :preregister-mode)

(set-logic QF_UF)
(declare-fun x () Bool)
(assert (or x (not x)))
(check-sat)

(set-option :verbosity 0)
(get-option :verbosity)
(set-option :preregister-mode eager)
