; EXPECT: unsat
;; define-const is not supported in Alethe
(set-logic UF)
(declare-const x Bool)
(define-const p Bool x)
(assert false)
(check-sat)
