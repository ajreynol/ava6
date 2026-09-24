; EXPECT: unsat
(set-logic ALL)
(set-option :incremental false)
(declare-fun a () Bool)
(declare-datatypes ((__ava6_record__a_Int 0)) (((__ava6_record__a_Int_ctor (_a Int)))))
(define-fun a49 () Bool (= (ite a (__ava6_record__a_Int_ctor 1) (__ava6_record__a_Int_ctor 2)) (__ava6_record__a_Int_ctor (ite a 1 2))))
(check-sat-assuming ( (not (= (ite a (__ava6_record__a_Int_ctor 1) (__ava6_record__a_Int_ctor 2)) (__ava6_record__a_Int_ctor (ite a 1 2)))) ))
