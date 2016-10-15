/* 
**  File:   event.h
**  Author: Maurizio Munafo'
**  Description: Definitions, data structures and propotypes for the management of the event list 
*/

#include "simtime.h"

typedef struct event_unit Event;

/*
  The Event List is a bi-linked circular list, referenced by a pointer to the last element 
*/
struct event_unit
      {
	Event *next;	/* Next Event */
	Event *prev;    /* Previous Event */
	Time  time;     /* Scheduled time for the event */
	int   type;     /* Type of the event */
	/* Optional fields, depending of the events to be scheduled */
	int   param1;   /* Parameter 1, it is an integer */
	int   param2;   /* Parameter 2, it is an integer */
	double param3;  /* Parameter 2, it is a double */ 
      };

/* Prototypes */
void insert_event(Event **, Event *);	/* Insert an event in the FES */
Event *get_event(Event **);		/* Extract the first event from the FES */
Event *new_event(void );		/* Return a new event */
void release_event(Event *);		/* Release an event */
