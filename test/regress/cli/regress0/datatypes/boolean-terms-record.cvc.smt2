; EXPECT: sat
(set-logic ALL)
(set-option :incremental false)
(declare-datatypes ((__ava6_record_i_Int_b_Bool 0)) (((__ava6_record_i_Int_b_Bool_ctor (i Int) (b Bool)))))
(declare-fun x () __ava6_record_i_Int_b_Bool)
(assert (not (= x (__ava6_record_i_Int_b_Bool_ctor 0 false))))
(check-sat)
