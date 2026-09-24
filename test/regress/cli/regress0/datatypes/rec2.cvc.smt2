; EXPECT: unsat
(set-logic ALL)
(set-option :incremental false)
(declare-fun c () Bool)
(declare-datatypes ((__ava6_record__a_Int__b_Int 0)) (((__ava6_record__a_Int__b_Int_ctor (_a Int) (_b Int)))))
(define-fun a16 () __ava6_record__a_Int__b_Int ((_ update _a) (ite c (__ava6_record__a_Int__b_Int_ctor 3 2) (__ava6_record__a_Int__b_Int_ctor 1 5)) 2))
(define-fun a21 () Bool (let ((_let_1 ((_ update _a) (ite c (__ava6_record__a_Int__b_Int_ctor 3 2) (__ava6_record__a_Int__b_Int_ctor 1 5)) 2))) (ite c (= (__ava6_record__a_Int__b_Int_ctor 2 2) _let_1) (= (__ava6_record__a_Int__b_Int_ctor 2 5) _let_1))))
(check-sat-assuming ( (not (let ((_let_1 ((_ update _a) (ite c (__ava6_record__a_Int__b_Int_ctor 3 2) (__ava6_record__a_Int__b_Int_ctor 1 5)) 2))) (ite c (= (__ava6_record__a_Int__b_Int_ctor 2 2) _let_1) (= (__ava6_record__a_Int__b_Int_ctor 2 5) _let_1)))) ))
