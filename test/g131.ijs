1:@:(dbr bind Debug)@:(9!:19)2^_44[(echo^:ECHOFILENAME) './g131.ijs'
NB. %. ------------------------------------------------------------------

NB. mdiv failed on small ct
ct   =: 9!:18''
9!:19[2^_40

X    =: +/ . *
en   =: 1&{@(,&1 1)@$
norm =: (%:@X +)@,

NB. given n, find m, the recursion split value
splitm =: 3 : 0
rawm =. <. -: y
xferamt =. (7 (17 b.) y) { 0 0 1 2 2 2 1 0
if. y>rawm+xferamt do. rawm =. rawm+xferamt end.
rawm
)

qr =: 3 : 0
 n  =. en y
 if. 1>:n do.
  (((* %) ;&,. ,~@en@[ $ ]) norm) y
 else.
  m  =. splitm n
  a0 =. m{."1 y
  a1 =. m}."1 y
  t0 =. qr a0
  q0 =. >@{. t0
  r0 =. >@{: t0
  c  =. (+|:q0) X a1
  t1 =. qr a1 - q0 X c
  q1 =. >@{. t1
  r1 =. >@{: t1
  (q0,.q1);(r0,.c),(-n){."1 r1
 end.
)

em   =:splitm@#                    NB.  m  =. >.-:#y.
mm   =: ,~@em                      NB.  mm =: m,m
ai   =: rinv@(mm {. ])             NB.  ai =: rinv mm{.y.
di   =: rinv@(mm }. ])             NB.  di =: rinv mm}.y.
bee  =: (em , em - #) {. ]         NB.  b  =: (m,m-n){.y.
bx   =: -@(ai X bee X di)          NB.  bx =: - ai X b X di
r4   =: (ai,.bx) , (-@# {."1 di)   NB.  (ai,.bx),(-n){."1 di
rinv =: r4`% @. (1&>:@#)

minv =: (|.@$ ($,) (rinv@] X +@|:@[)&>/@qr) " 2
mdiv =: (%.@] +/ . * [) " _ 2

id=: =&i.&#

4 19 -: $%.?19 4$2
6 10 -: $%.?10 6$100
7 11 -: $%.?11 7$100
5 7  -: $%.j./?2 7 5$100

1       -: %.1
(%4)    -: %.4
(%3.5)  -: %.3.5
(%4j_5) -: %.4j_5

(%. -: minv) 1
(%. -: minv) 1+?12369        NB. minv failed on 0
(%. -: minv) (+ 0&=)o.?1721
(%. -: minv) j./1+?100 2001

(b-:minv a) *. 1e_8>>./|,(id a)-a X b=:%.a=:_50000+?10 10$10000
(b-:minv a) *. 1e_8>>./|,(id a)-a X b=:%.a=:0.01*_4000+?7 7$10000
(b-:minv a) *. 1e_8>>./|,(id a)-a X b=:%.a=:j./?2 8 8$1300

(b-:minv a) *. (1=+/a*b) *. (+/a*+a)-:%+/b*+b =:%.a=:1=?36$2
(b-:minv a) *. (1=+/a*b) *. (+/a*+a)-:%+/b*+b =:%.a=:_10+?17$20
(b-:minv a) *. (1=+/a*b) *. (+/a*+a)-:%+/b*+b =:%.a=:0.1*_10+?13$20
(b-:minv a) *. (1=+/a*b) *. (+/a*+a)-:%+/b*+b =:%.a=:r.?23$20

x -: %. x=:=i.1
x -: %. x=:=i.2
x -: %. x=:=i.4
x -: %. x=:=i.9

f =: (%@[ * ]) -: %.@:*

4        f =i.1
3        f =i.2
_912     f =i.15

_6.9     f =i.1
5.79     f =i.2
_13.9    f =i.9

0j1      f =i.1
3j4      f =i.2
123j_7.9 f =i.16

(i.0 0 ) -: %.   i.0 0
(i.0 10) -: %.   i.10 0
(i.0)    -: %.   i.0

(i.0 0 ) -: minv i.0 0
(i.0 10) -: minv i.10 0
(i.0)    -: minv i.0

(i.0 0 ) -: (i.0 0)     %.   i.0 0
(i.0 10) -: (i.0 10)    %.   i.0 0
(i.0)    -: (?10$100)   %.   i.10 0
(i.0 3)  -: (?10 3$100) %.   i.10 0

(i.0 0 ) -: (i.0 0)     mdiv i.0 0
(i.0 10) -: (i.0 10)    mdiv i.0 0
(i.0)    -: (?10$100)   mdiv i.10 0
(i.0 3)  -: (?10 3$100) mdiv i.10 0

(    ?10$100)   (%. -: mdiv) _50000+?10 10$112300
(0.1*?10$100)   (%. -: mdiv) 0.0231*_4000+?10 10$12200
(j./?2 10$100)  (%. -: mdiv) j./0.0231*_4000+?2 10 10$12200

(?10 10$100)    (%. -: mdiv) ?10 10$100
(0.1*?7 7$100)  (%. -: mdiv) 0.231*_40+?7 7$100
(j./?2 9 9$100) (%. -: mdiv) r./0.231*_40+?2 9 9$100

'domain error' -:        %. etx 4 4$'abc'
'domain error' -:        %. etx 4 4$u:'abc'
'domain error' -:        %. etx 4 4$10&u:'abc'
'domain error' -:        %. etx 4 4$s:@<"0 'abc'
'domain error' -:        %. etx 4 4$<123

'domain error' -: 'abcd' %. etx ?4 4$100
'domain error' -: (u:'abcd') %. etx ?4 4$100
'domain error' -: (10&u:'abcd') %. etx ?4 4$100
'domain error' -: (s:@<"0 'abcd') %. etx ?4 4$100

'length error' -:        %. etx ?3 5$123
'length error' -: 3 4 5  %. etx ?7 4$100

9!:19 ct

4!:55 ;:'X a a0 a1 ai b bee bx c ct di '
4!:55 ;:'em en f id m mdiv minv mm n norm '
4!:55 ;:'q0 q1 q2 qr r0 r1 r4 rinv splitm x '


