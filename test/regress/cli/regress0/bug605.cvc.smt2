; EXPECT: sat
(set-logic ALL)
(set-option :incremental true)
(set-option :produce-models true)
(declare-datatypes ((__ava6_record_longitude_Int_latitude_Int 0)) (((__ava6_record_longitude_Int_latitude_Int_ctor (longitude Int) (latitude Int)))))

(declare-datatypes ((|__ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)| 0)) (((|__ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)_ctor| (geoLoc (Set __ava6_record_longitude_Int_latitude_Int))))))

(declare-datatypes ((|__ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)_| 0)) (((|__ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)__ctor| (base |__ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)|)))))

(declare-datatypes ((|__ava6_record_s_f____ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)__| 0)) (((|__ava6_record_s_f____ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)___ctor| (s_f |__ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)_|)))))


(declare-fun a () (Array Int |__ava6_record_s_f____ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)__|))
(define-fun p1 () __ava6_record_longitude_Int_latitude_Int (__ava6_record_longitude_Int_latitude_Int_ctor 0 0))
(define-fun s1 () |__ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)| (|__ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)_ctor| (set.singleton (__ava6_record_longitude_Int_latitude_Int_ctor 0 0))))
(define-fun f0 () |__ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)_| (|__ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)__ctor| (|__ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)_ctor| (set.singleton (__ava6_record_longitude_Int_latitude_Int_ctor 0 0)))))
(define-fun init ((v (Array Int |__ava6_record_s_f____ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)__|)) (i Int) (f |__ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)_|)) Bool (= (s_f (select v 0)) f))
(assert (init a 2 (|__ava6_record_base____ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)__ctor| (|__ava6_record_geoLoc_(Set __ava6_record_longitude_Int_latitude_Int)_ctor| (set.singleton (__ava6_record_longitude_Int_latitude_Int_ctor 0 0))))))
(push 1)

(assert true)

(check-sat)

(pop 1)

