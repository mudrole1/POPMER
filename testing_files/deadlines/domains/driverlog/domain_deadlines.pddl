(define (domain driverlog)
  (:requirements :typing :durative-actions :timed-initial-literals) 
  (:types          task location locatable truck - object
		driver obj - locatable)

  (:predicates 
		(at ?obj - locatable ?loc - location ?t - task)
                (atr ?tr - truck ?loc - location)
		(in ?obj1 - obj ?obj - truck)
		(driving ?d - driver ?v - truck)
		(link ?x ?y - location) (path ?x ?y - location)
		(empty ?v - truck)
                (success ?t - task ?obj1 - obj ?loc - location)
                (valid ?t - task)
)

(:durative-action LOAD-TRUCK
  :parameters
   (?obj - obj
    ?truck - truck
    ?loc - location
    ?t - task)
  :duration (= ?duration 2)
  :condition
   (and 
   (over all (atr ?truck ?loc)) 
   (at start (at ?obj ?loc ?t))
   (over all (valid ?t))
   )
  :effect
   (and (at start (not (at ?obj ?loc ?t))) (at end (in ?obj ?truck))))

(:durative-action UNLOAD-TRUCK
  :parameters
   (?obj - obj
    ?truck - truck
    ?loc - location
    ?task - task )
  :duration (= ?duration 2)
  :condition
   (and
        (over all (atr ?truck ?loc))
        (at start (in ?obj ?truck))
        (over all (valid ?task))
   )
  :effect
   (and (at start (not (in ?obj ?truck))) (at end (at ?obj ?loc ?task))))

(:durative-action DRIVE-TRUCK
  :parameters
   (?truck - truck
    ?loc-from - location
    ?loc-to - location
    ?driver - driver
    ?task - task)
  :duration (= ?duration 10)
  :condition
   (and (at start (atr ?truck ?loc-from))
   (over all (driving ?driver ?truck)) 
   (at start (link ?loc-from ?loc-to))
   (over all (valid ?task))
   )
  :effect
   (and (at start (not (atr ?truck ?loc-from))) 
	(at end (atr ?truck ?loc-to))))

 
(:durative-action task-finish-ontime
 :parameters (?obj - obj 
              ?loc - location
              ?task - task)
 :duration (= ?duration 1)
 :condition (and 
     (over all (at ?obj ?loc ?task)) 
     (over all (valid ?task))
 )
 :effect (and  
    (at end (success ?task ?obj ?loc)) 
 )
)

)
