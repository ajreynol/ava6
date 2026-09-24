; EXPECT: unsat
(set-logic ALL)
(set-option :incremental false)
(declare-fun c () Bool)
(declare-datatypes ((__ava6_record__a_Int__b_Int 0)) (((__ava6_record__a_Int__b_Int_ctor (_a Int) (_b Int)))))
(define-fun a17 () Bool (= (__ava6_record__a_Int__b_Int_ctor 2 2) ((_ update _a) (ite c (__ava6_record__a_Int__b_Int_ctor 3 2) (__ava6_record__a_Int__b_Int_ctor 1 2)) 2)))
(check-sat-assuming ( (not (= (__ava6_record__a_Int__b_Int_ctor 2 2) ((_ update _a) (ite c (__ava6_record__a_Int__b_Int_ctor 3 2) (__ava6_record__a_Int__b_Int_ctor 1 2)) 2))) ))
