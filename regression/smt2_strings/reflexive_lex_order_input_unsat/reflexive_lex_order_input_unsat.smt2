(declare-const in1 String)
(declare-const in2 String)
(assert (= in1 "def"))
(assert (= in2 "abc"))
(assert (str.<= in1 in2))
(check-sat)
