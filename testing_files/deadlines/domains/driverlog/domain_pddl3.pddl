(define (domain driverlog)
  (:requirements :typing :durative-actions) 
  (:types           location locatable - object
		driver truck obj - locatable)

  (:predicates 
		(at ?obj - locatable ?loc - location)
		(in ?obj1 - obj ?obj - truck)
		(driving ?d - driver ?v - truck)
		(link ?x ?y - location) (path ?x ?y - location)
		(empty ?v - truck)
		(success ?obj1 - obj ?loc - location)
  )

(:durative-action LOAD-TRUCK
  :parameters
   (?obj - obj
    ?truck - truck
    ?loc - location)
  :duration (= ?duration 2)
  :condition
   (and 
   (over all (at ?truck ?loc))
   (at start (at ?obj ?loc)))
  :effect
   (and 
(at start (not (at ?obj ?loc))) 
(at end (in ?obj ?truck)))
)

(:durative-action UNLOAD-TRUCK
  :parameters
   (?obj - obj
    ?truck - truck
    ?loc - location)
  :duration (= ?duration 2)
  :condition
   (and
        (over all (at ?truck ?loc)) 
         (at start (in ?obj ?truck)))
  :effect
   (and 
(at start (not (in ?obj ?truck))) 
(at end (at ?obj ?loc)))
)

(:durative-action DRIVE-TRUCK
  :parameters
   (?truck - truck
    ?loc_from - location
    ?loc_to - location
    ?driver - driver)
  :duration (= ?duration 10)
  :condition
   (and 
(at start (at ?truck ?loc_from))
   (over all (driving ?driver ?truck)) 
(at start (link ?loc_from ?loc_to)))
  :effect
   (and 
(at start (not (at ?truck ?loc_from))) 
	(at end (at ?truck ?loc_to)))
)


(:durative-action task-finish-ontime
 :parameters (?obj - obj 
              ?loc - location)
 :duration (= ?duration 1)
 :condition (and 
     (over all (at ?obj ?loc)) 
 )
 :effect (and  
    (at end (success ?obj ?loc)) 
 )
)


)


