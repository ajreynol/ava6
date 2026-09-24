; EXPECT: sat
(set-logic ALL)
(set-option :incremental false)
(declare-datatypes ((__ava6_record_i_Int_b_Int 0)) (((__ava6_record_i_Int_b_Int_ctor (i Int) (b Int)))))
(declare-fun f (Int) __ava6_record_i_Int_b_Int)
(declare-fun a () Int)
(assert (not (= (f a) (__ava6_record_i_Int_b_Int_ctor 0 0))))
(check-sat)
