(define gutter? (lis) (eq 0 (car lis)))
(define strike? (lis) (eq 10 (car lis)))
(define spare? (lis) (eq 10 (+ (car lis) (cadr lis))))


(define sum2 (lis) (+ (car lis) (cadr lis)))
(define sum3 (lis) (+ (car lis) (+ (cadr lis) (caddr lis)))

(define drop1 (lis) (cdr lis) )
(define drop2 (lis) (cddr lis) )
(define drop (lis) (cond
	(strike? lis) (drop1 lis)
	't (drop2 lis)
))
 
(define bowlingscore (lis) 
	bowling(lis 1)
)

(define +1 (x) (+ 1 x))

(define bowling (lis frame#) (cond
	(eq frame# 11) 0
	't (+ thisframe(lis) 
		(bowling(drop lis) (+1 frame#))
)))

(define thisframe (lis)(cond
	(or (spare? lis) (strike? lis) (sum3 lis)
	't (sum2 lis)
)))

(map f lis)
(define map (f lis) (cond
	(nil? lis) ()
	't (cons (f (car lis)) (map f (cdr lis)))
))

(define join (f lis inti)(cond
	(nil? lis) init
	't (join f (cdr lis (f (car lis) init))))
)

f => min
f => sum

map
join/fold/foldr/foldl
apply

(list a b c) -> (a b c)
(list2 a b) -> (a b)
(list3 a b c) -> (a b c)
(list a) => (cons a ())

(cons a b) => (a.b)

