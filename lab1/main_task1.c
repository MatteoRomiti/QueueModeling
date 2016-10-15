#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event.h"
#include "record.h"
#include "simtime.h"

#define ARRIVAL    1
#define DEPARTURE  2

long seme = 14123451;

Event *event_list = NULL;
Record *queue=NULL;
Record *in_service=NULL;
double lambda,mu;
int total_users;
double cumulative_time_user;
Time total_delay,last_event_time; //total_delay is the sum of all the service times, not the time spent in the queue
int number_of_samples;
Time current_time;
Time idle_time;
Time start_idle_time;

extern double negexp(double,long *);

void schedule(int type, Time time)
{
   Event *ev;
   ev = new_event();
   ev->type = type;
   ev->time = time;
   insert_event(&event_list,ev);
   return;
}

/*
**  Function : void get_input(char *format,void *variable)
**  Return   : None
**  Remarks  : To be used instead of scanf. 'format' is a scanf format,
**             'variable' the pointer to the variable in which the input
**             is written.
*/
void get_input(char *format,void *variable)
{
    static char linebuffer[255];
    char *pin;

    fgets(linebuffer, 255, stdin);  /*  Gets a data from stdin without  */
    pin = strrchr (linebuffer, '\n'); /*  flushing problems       */
    if (pin!=NULL) *pin = '\0';

    sscanf(linebuffer,format,variable); /* Read only the beginning of the   */
          /* line, letting the user to write  */
          /* all the garbage he prefer on the */
          /* input line.          */
}

void arrival(void)
{ Time delta;
  Record *rec;
  delta = negexp(1.0/lambda,&seme);
  schedule(ARRIVAL,current_time+delta); // schedule a future arrival
  // printf("next arrival added to the event_list\n");
  rec = new_record();
  rec->arrival = current_time;
  cumulative_time_user+=total_users*(current_time-last_event_time);
  total_users++;
  if (in_service == NULL)
    { // the server was idle
      in_service = rec; // put the record in service 
      // update the idle_time
      idle_time = idle_time + (current_time - start_idle_time); //start idle time is captured in the function departure
      
	  //double service_time = negexp(1.0/mu,&seme);
	  double service_time = geometric1(1.0/mu,&seme);
	  schedule(DEPARTURE,current_time+service_time); // schedule the departure of this arrival
	  //printf("Service time %f\n",service_time);
	  //printf("%f\n",service_time);
	}
  else
    in_list(&queue,rec); // insert a record in the queue
  return;
}

void departure(void)
{
  Record *rec;
  rec = in_service;
  in_service = NULL;
  cumulative_time_user+=total_users*(current_time-last_event_time);
  total_users--;
  number_of_samples++;
  total_delay+=current_time - rec->arrival;
  release_record(rec);
  if (queue!=NULL)
   {
    in_service = out_list(&queue); // take a record from the queue and serve it
    //p is the probability of success on each trial (parameter of geom.distr)
    //from https://en.wikipedia.org/wiki/Geometric_distribution#Related_distributions
  	//double p = 1-exp(-mu); 
    //double mean = (1-p)/p;
	  //printf("Mean service time %f\n",mean);
	  //schedule(DEPARTURE,current_time+geometric1(1.0/mu,&seme)); 
	  // double service_time = negexp(1.0/mu,&seme);
	  double service_time = geometric1(1.0/mu,&seme);
	  schedule(DEPARTURE,current_time+service_time); // schedule the departure of this arrival
  	//printf("%f\n",service_time);
   } else {
    // the queue is empty, so the server is idle
    start_idle_time = current_time;
   }
  return;
}



void results(void)
{
 cumulative_time_user+=total_users*(current_time-last_event_time); // ???
 double rho= lambda/mu;

 printf("\n*************results******************\n\n");
 //ALWAYS TRUE
 //printf("Final time %f\n",current_time);
 printf("Number of services %d\n",number_of_samples);
 printf("Measured average service time   %f\n",total_delay/number_of_samples); //always true
 printf("Measured idle server probability   %f\n",idle_time/current_time); //always true
 printf("Measured average number of users %f\n",cumulative_time_user/current_time); //always true
 //TRUE FOR M/M/1
/*
 printf("Theoretical average time in the system %f\n",1.0/(mu-lambda)); //true for M/M/1
 printf("Theoretical average time in the queue %f\n",rho/(mu-lambda)); //true for M/M/1
 printf("Theoretical average number of users %f\n",lambda/(mu-lambda)); //true for M/M/1
*/  
 // TASK ONE
 
 // P-K formula M/M/1
 double Cs2 = 1;
 double E_S = 1.0/mu;
 
 /*
 //P-K formula M/G/1
 lambda and mu must be values between 0 and 1
 double Cs2=1-mu; 
 double E_S = 1.0/mu; 
 */
 
 double E_N= rho+rho*rho*(1+Cs2)/(2*(1-rho)); 
 double E_T = E_S+rho*E_S*(1+Cs2)/(2*(1-rho));

 printf("Pollaczek-Khintchin formula \n");
 printf("Theoretical average number of users = %f\n",E_N);
 printf("Theoretical average time in the system = %f\n",E_T);

 printf("\n*****************************************\n\n");
 exit(0);
}

void main()
{	
 printf("\n\n*****************************************\n\n");
 Event *ev;
 Time maximum;

 cumulative_time_user =0.0;
 total_delay=0.0;
 last_event_time=0.0;
 number_of_samples=0.0;

 idle_time = 0.0;
 
 current_time = 0.0;
 total_users = 0;
 
 printf("Insert arrival rate, lambda: ");
 get_input("%lf",&lambda);
 printf("lambda = %f\n",lambda);
 printf("Insert serive rate, mu: ");
 get_input("%lf",&mu);
 printf("mu     = %f\n",mu);
 printf("Insert simulation time: ");
 get_input("%lf",&maximum);
 printf("Max Time  = %f\n\n",maximum);

 schedule(ARRIVAL,negexp(1.0/lambda,&seme));
 printf("first arrival added to the event_list\n");

 while (current_time<maximum)
  {
    ev = get_event(&event_list); // take the last event
    last_event_time = current_time;
    current_time = ev->time; // service time
    switch (ev->type)
     {
       case ARRIVAL:         	
     		arrival();
        break;
       case DEPARTURE: 
        departure();
  	    break;
       default:       printf("This is awful!!!\n");
          exit(1);
     }
    release_event(ev);
  }
 results();

}
