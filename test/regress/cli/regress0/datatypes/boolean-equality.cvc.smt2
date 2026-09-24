; EXPECT: sat
(set-logic ALL)
(set-option :incremental false)
(declare-datatypes ((__ava6_record_b_Bool 0)) (((__ava6_record_b_Bool_ctor (b Bool)))))
(declare-fun x () __ava6_record_b_Bool)
(assert (= x x))
(check-sat)
