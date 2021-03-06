(define (domain tank-domain)
(:requirements :fluents :durative-actions :duration-inequalities :negative-preconditions)
(:predicates (draining ?t) (filling ?b))
(:functions (volume ?t) (drain-time ?t) (flow-constant ?t) (capacity ?b) (sqrtvolinit ?t) (sqrtvol ?t))

(:durative-action fill-bucket
:parameters (?b ?t)
:duration (<= ?duration (/ (sqrtvolinit ?t) (flow-constant ?t)))
:condition (and (over all (<= (volume ?b) (capacity ?b)))
	(at start (not (draining ?t)))
        (at start (not (filling ?b))))
:effect (and (at start (assign (drain-time ?t) 0))
		(at start (assign (sqrtvol ?t) (sqrtvolinit ?t)))
		(at start (draining ?t))
		(at start (filling ?b))
		(increase (drain-time ?t) (* #t 1))
		(decrease (volume ?t) (* #t (* (* 2 (flow-constant ?t))
						(- (sqrtvolinit ?t) (* (flow-constant ?t) (drain-time ?t))))))
		(decrease (sqrtvol ?t) (* #t (flow-constant ?t)))
		(increase (volume ?b) (* #t (* (* 2 (flow-constant ?t))
						(- (sqrtvolinit ?t) (* (flow-constant ?t) (drain-time ?t))))))
		(at end (assign (sqrtvolinit ?t) (sqrtvol ?t)))
		(at end (not (draining ?t)))
		(at end (not (filling ?b))))))
