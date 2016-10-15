/* 
**  File:   record.h
**  Author: Maurizio Munafo'
**  Description: Definitions, data structures, prototypes for the management of FIFO queues of records
*/

#include "simtime.h"

typedef struct record_unit Record;

struct record_unit
  { 
    Record *next;
    Time  arrival;
  };

/* Prototypes */
void in_list(Record **, Record *);	/* Insert a record in the queue */
Record *out_list(Record **);		/* Extract a record from the queue */ 
Record *new_record(void );		/* Return a new (empty) record */
void release_record(Record *);		/* Release a record */

