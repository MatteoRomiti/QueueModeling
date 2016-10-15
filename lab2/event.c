/* 
**  File:   Event.c
**  Author: Maurizio Munafo'
**  Description: Functions for the management of the Future Event Set 
*/

#include <stdio.h>
#include <stdlib.h>
#include "event.h"

Event *free_events = NULL;  /* Pointer to the Free List of events */

/* 
**  Function:  void insert_event(Event **last, Event *elem)
**  Parameters: Event **last  - Pointer to the list (last element) 
**             Event  *elem  - Record of type Event to be inserted 
**  Retrun:   None 
**  Description: Insert 'elem' in the Event List pointed by **last
*/
void insert_event(Event **last, Event *elem)
{
  Event *p;
  if (elem==NULL) return;       /* Nothing to insert */

  /* The Event List is empty: elem is the Event List */
  if ((*last)==NULL)
    {
       (*last) = elem;
       (*last)->next = elem;
       (*last)->prev = elem;
       return;
    }

  /* 
  ** elem is scheduled after the last event in the list: 
  ** it is insertged at the end and it becomes the new event 'last' 
  */
  if (elem->time >= (*last)->time)
    {
       elem->prev = (*last);
       elem->next = (*last)->next;
       ((*last)->next)->prev = elem;
       (*last)->next = elem;
       (*last) = elem;
    }
  else
  /* 
  ** look for the position of elem, starting from the end of the list
  ** and following the pointer prev (i.e., previous)  
  */
    {
       p = (*last);
       while (elem->time < p->time && p->prev!=(*last))
         p = p->prev;
	   if (elem->time < p->time)
         { 
		   /* p is the first element that is scheduled after elem: 
		      insert elem just before p */
           elem->prev = p->prev;
           elem->next = p;
           (p->prev)->next = elem;
           p->prev = elem;
         }
       else
         {
		   /* elem must be the first event in the list: 
		      insert elem after p (that is *last) */
           elem->prev = p;
           elem->next = p->next;
           (p->next)->prev = elem;
           p->next = elem;
         }
    }
  return;
}

/* 
**  Function:  Event *get_event(Event **last)
**  Parameters: Event **last  - Pointer to the list (last element)
**  Return:   Pointer to the extracted event 
**  Description: Return the next event to be scheduled, extracting it from the top of the list. 
** 		 Return NULL if the Event List is empty
*/
Event *get_event(Event **last)
{
  Event *p;
  if ((*last)==NULL) return NULL;   /* The Event List is empty */
  if ((*last)->next==(*last))
    {
	  /* Extract the only element in the Event List */
      p = (*last);
      (*last) = NULL;
    }
  else
    {
	  /* Extract the first element (successor of last)  */
      p = (*last)->next;
      (p->next)->prev = (*last);
      (*last)->next = p->next;
    }
  return p;
}

/* 
**  Function:  Event *new_event(void)
**  Parameters: None 
**  Return:   Pointer to a new event 
**  Description: Return a new event 
**  NOTICE: It inserts or removes elements from the Free List 
*/
Event *new_event(void)
{
 extern Event *free_events;
 Event *p;
 int i;
 if (free_events==NULL)
  {
    if ((p=(Event *)malloc(sizeof(Event)))==NULL)
      { printf("Error  : Memory allocation error\n");
        exit(1);
      }
    p->time = (Time)0;
    insert_event(&free_events,p);
  }
 return(get_event(&free_events));  /* Return the first element of the Free List */
}

/* 
**  Function:  void release_event(Event *elem)
**  Parameters: Event to release 
**  Return:   None 
**  Description: Release the data structure of the event, inserting it into the Free List 
*/
void release_event(Event *elem)
{
 extern Event *free_events;
 elem->time = (Time)MAX_TIME;    /* The record goes at the end of the Free List */
 insert_event(&free_events,elem);
}
