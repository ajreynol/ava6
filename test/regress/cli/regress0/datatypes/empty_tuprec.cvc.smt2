; COMMAND-LINE:
; EXPECT: unsat
; EXPECT: unsat
; EXPECT: unsat
; EXPECT: unsat
(set-logic ALL)
(set-option :incremental true)
(declare-fun a1 () UnitTuple)
(declare-fun a2 () UnitTuple)
(declare-datatypes ((__ava6_record 0)) (((__ava6_record_ctor))))
(declare-fun b1 () __ava6_record)
(declare-fun b2 () __ava6_record)
(declare-fun c1 () (Tuple UnitTuple))
(declare-fun c2 () (Tuple UnitTuple))
(declare-datatypes ((__ava6_record_z_Tuple 0)) (((__ava6_record_z_Tuple_ctor (z UnitTuple)))))
(declare-datatypes ((|__ava6_record_x_(Tuple __ava6_record)_y___ava6_record_z_Tuple| 0)) (((|__ava6_record_x_(Tuple __ava6_record)_y___ava6_record_z_Tuple_ctor| (x (Tuple __ava6_record)) (y __ava6_record_z_Tuple)))))
(declare-fun d1 () |__ava6_record_x_(Tuple __ava6_record)_y___ava6_record_z_Tuple|)
(declare-fun d2 () |__ava6_record_x_(Tuple __ava6_record)_y___ava6_record_z_Tuple|)
(check-sat-assuming ( (not (= a1 a2)) ))
(check-sat-assuming ( (not (= b1 b2)) ))
(check-sat-assuming ( (not (= c1 c2)) ))
(check-sat-assuming ( (not (= d1 d2)) ))
