; EXPECT: sat
(set-logic ALL)
(set-option :incremental false)
(declare-datatypes ((|__ava6_record_b_(_ BitVec 1)| 0)) (((|__ava6_record_b_(_ BitVec 1)_ctor| (b (_ BitVec 1))))))
(declare-fun x () |__ava6_record_b_(_ BitVec 1)|)
(assert (= false (= (= x (|__ava6_record_b_(_ BitVec 1)_ctor| #b0)) (= x (|__ava6_record_b_(_ BitVec 1)_ctor| #b1)))))
(check-sat)
