// WITHOUT RATE ADAPTATION: the capacity is always high
// TRANSITION ON/OFF is possible

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
Record *queue = NULL;
Record *in_service = NULL;

float lambda; // lambdaH or lambdaL depending on the rate used; arrival's mean value = 1/lambda
float lambdaH = 100; // lambda for high rate: lambdaH packets per second arrive
float lambdaL = 5; // lambda for low rate: lambdaL packets per second arrive
float Ch, Cl, C; // capacity
float pkt_size = 12000;
float sum_total_users = 0; // counter to keep track of all the users
float lost_users=0;
float powerH=1; //power consumption on high rate per second
float powerL=0.5; //power consumption on low rate per second
float on_off_power = 0;
float hpktpower=0.4; // power consumption on high rate per packet sent
float lpktpower=0.2; // power consumption on low rate per packet sent
float total_energy=0; // energy spent by the system
float energy_per_second = 0;
float energy_consumption = 0;
float sH = 0.01; // seconds needed to transmit a packet in high rate 
float sL = 0.2; // seconds needed to transmit a packet in low rate 

int number_of_samples = 0; // completed services
int total_users = 0; // users in the system (queuing + in service)
int print; // 1 for a verbose run, 0 otherwise
int go_on = 1; // flag used to end the process
int mean_high = 60; // the time (seconds) spent in high rate is a random variable with mean = mean_high
int mean_low = 60; // the time spent in low rate is a random variable with mean = mean_low
int queue_size=20; 
int queuing_users=0;
int inputs; //input from consolle
int on_off_switching = 0;


//everything is in SECONDS
Time maximum=60.0*60*10;	//simulation time: seconds
Time cumulative_time_user = 0.0; // I don't care
Time total_delay = 0.0; // time spent in the system by all the users
Time last_event_time = 0.0; // I don't care
Time current_time = 0.0; // the time
Time sum_high_time = 0.0; // time spent in high rate
Time sum_low_time = 0.0; // time spent in low rate
Time sum_change_time = 0.0; //time spent in changing the state
Time idle_time = 0.0; 
Time start_state_time = 0.0; //used to compute delta_state_time
Time delta_state_time = 0.0; //used when the simulation ends
Time start_idle_time = 0.0;
Time big_delta = 0.0; // sum of all the sum_x_time
Time t_delta, service_time; // used in event scheduling
Time tH,tL; // time in high rate, time in low rate
Time tLH = 1; //transition time
Time tHL = 1; //transition time

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

    fgets(linebuffer, 255, stdin);	/*  Gets a data from stdin without  */
    pin = strrchr (linebuffer, '\n');	/*  flusHing problems		    */
    if (pin!=NULL) *pin = '\0';

    sscanf(linebuffer,format,variable);	/* Read only the beginning of the   */
					/* line, letting the user to write  */
					/* all the garbage he prefer on the */
					/* input line.			    */
}

void arrival(double C){
	  if (print==1){printf("ARRIVAL\n");}
	  Time delta;
	  Record *rec;
	  
	  delta = negexp(1.0/lambda,&seme); //next arrival happens in delta seconds
	  t_delta=current_time+delta;
	  schedule(ARRIVAL,t_delta); // schedule a future arrival
	  if (print==1){
	  	printf("CURRENT TIME: %f\n",current_time);
	  	printf("next arrival scheduled with DELTA: %f\n",delta);
	  }
	  rec = new_record();
	  rec->arrival = current_time;
	  cumulative_time_user+=total_users*(current_time-last_event_time);
	  total_users++;
	  sum_total_users++;
	  if (print==1){
	  	printf("SUM OF USERS %f\n",sum_total_users);
	  }
	  
	  if (in_service == NULL){	
		  // the server was idle
		  in_service = rec; 
		  idle_time = idle_time + (current_time - start_idle_time); //start idle time is captured in the function departure
		  on_off_switching++;
		  service_time = pkt_size/C;
		  schedule(DEPARTURE,current_time + service_time); // schedule the departure of this arrival
		  if (print==1){printf("next DEPARTURE scheduled with service time: %f\n",service_time);}
	  } else {
		if(queuing_users < queue_size){ 
		  in_list(&queue,rec); // insert a record in the queue
		  queuing_users++;
			//printf("One user added to the queue. %d users in the queue \n", queuing_users);
		} else {
		  release_record(rec);
		  lost_users++;
		  if (print==1){
		  	printf("one user lost\n");
		  	printf("lost users: %f\n",lost_users);
		  }
		  total_users--;
		}
	  }
	  if (print==1){printf("queuing users: %d\n",queuing_users);
	  printf("total_users: %d\n",total_users);};
	  return;
	}

void departure(double C)
{
  Record *rec;
  rec = in_service;
  in_service = NULL;
  cumulative_time_user+=total_users*(current_time-last_event_time);
  total_users--;
  number_of_samples++;
  total_delay+=current_time - rec->arrival;
  release_record(rec);
  if (print==1){printf("DEPARTURE, current time: %f\n", current_time);};
  if (queue!=NULL)
   {
    in_service = out_list(&queue); // take a record from the queue and serve it
    //schedule(DEPARTURE,current_time+geometric0(1.0/mu,&seme));
	service_time = pkt_size/C;
	schedule(DEPARTURE,current_time + service_time); // schedule the departure of this arrival
    if (print==1){printf("next DEPARTURE scheduled with service time: %f\n",service_time);};
	queuing_users--;
	
   } else {
   	// the queue is empty, so the server is idle
   	start_idle_time = current_time;
   	on_off_switching++;
   	if (print==1){printf("the queue is empty, so the server is idle\n");};
   	
   }
     
  if (print==1){printf("queuing users: %d\n",queuing_users); 
  printf("total users: %d\n",total_users);};
  return;
}

void results(void)
{
 cumulative_time_user+=total_users*(current_time-last_event_time); // ???
 printf("\n******************************************\n");
/* printf("final time %f\n",current_time);
 printf("high time %f\n",sum_high_time);
 printf("low time %f\n",sum_low_time);
 printf("transition time %f\n",sum_change_time);
 printf("measured simulation time %f\n",big_delta);
 printf("number of services %d\n",number_of_samples);
 printf("Idle server probability %f\n",idle_time/current_time); 
 printf("Measured average time in the system %f\n",total_delay/number_of_samples); //always true 
 printf("??? Average number of users %f\n",cumulative_time_user/current_time); //cumulative_time_user+=total_users*(current_time-last_event_time);
 printf ("lost_users: %f\n",lost_users);
 printf ("total sum_users: %f (arrivals)\n",sum_total_users);
 printf ("Loss probability: %f\n",lost_users/sum_total_users);
 printf ("Total energy consumption: %f\n",total_energy);
 printf ("energy per second: %f\n", energy_per_second);
 printf ("energy consumption: %f\n", energy_consumption);
*/

 printf("%f\n",current_time);
/* printf("%f\n",sum_high_time);
 printf("%f\n",sum_low_time);
 printf("%f\n",sum_change_time);
 printf("%f\n",big_delta);
 printf("%d\n",number_of_samples);
*/ printf("%f\n",idle_time/current_time); 
 printf("%f\n",total_delay/number_of_samples); //always true 
 printf("%f\n",cumulative_time_user/current_time); //cumulative_time_user+=total_users*(current_time-last_event_time);
/* printf ("%f\n",lost_users);
 printf ("%f\n",sum_total_users);
*/ printf ("%f\n",lost_users/sum_total_users);
/* printf ("%f\n",total_energy);
 printf ("%f\n", energy_per_second);
*/ printf ("%f\n", energy_consumption);

 printf("\n*****************************************\n\n"); 
  
 exit(0);
}

int main(){

	Event *ev;
	//capacity 
	Ch = pkt_size/sH; // one pkt each sH seconds
	Cl = pkt_size/sL; // one pkt each sL seconds Time maximum;

	print = 0;
	float ti;
	ti=negexp(1.0/lambdaH,&seme); // start with lambdaH
	schedule(ARRIVAL,ti);

	Time t_add;


	if (print==1){
		printf("start cycle\n");
		printf("current_time %f\n",current_time);
	}  

	while (go_on){

		t_add = negexp(mean_high, &seme); 	//time spent by the system in "high rate"
		//////printf("delta high %f\n", t_add);
		tH = current_time + t_add; 	// create the time of high rate
		sum_high_time = sum_high_time + t_add;
		if (print==1){printf("sum_high_time: %f\n",sum_high_time);}  
		if (print==1){printf("-----------------start high rate for: %f-----------------\n",t_add);}
		if (print==1){printf("current_time %f\n",current_time);}  

		start_state_time = current_time;
		while (current_time<tH && go_on){
			lambda=lambdaH; 
			C = Ch;
			ev = get_event(&event_list); // take the last event
			last_event_time = current_time;
			current_time = ev->time; // when the event takes place
			if (current_time > maximum){
				//state interruption
				sum_high_time = sum_high_time - t_add; //this state does not last t_add
				delta_state_time = current_time - start_state_time; //compute how much this state actually lasted
				sum_high_time = sum_high_time + delta_state_time; //add the correct time interval
				go_on = 0;
			} else {
				switch (ev->type){
					case ARRIVAL:   arrival(C);
						  break;
					case DEPARTURE: departure(C);               //what if a packet sHould go out in the change status time????????????? in this case we send it.<-ask
						  break;
					default:       printf("This is awful!!!\n");
						  exit(1);
					}
				release_event(ev);
			}
		}

		t_add = negexp(mean_low, &seme); 	// time spent by the system in low rate
		tL= current_time+t_add;
		//////printf("delta low %f\n", t_add);
		if (go_on) {sum_low_time = sum_low_time + t_add;}
		if (print==1 && go_on){printf("sum_low_time: %f\n",sum_low_time);}  

		if (print==1 && go_on){printf("-----------------start low rate for: %f-----------------\n",t_add);}
		if (print==1 && go_on){printf("current_time %f\n",current_time);}  

		start_state_time = current_time;
		while (current_time<tL && go_on){	
			lambda=lambdaL;
			C = Ch;
			ev = get_event(&event_list); // take the last event
			last_event_time = current_time;
			current_time = ev->time; // service time
			if (current_time > maximum){
				//state interruption
				sum_low_time = sum_low_time - t_add; //this state does not last t_add
				delta_state_time = current_time - start_state_time; //compute how much this state actually lasted
				sum_low_time = sum_low_time + delta_state_time; //add the correct time interval
				//////printf("INTERRUPTION sum_low_time: %f\n",sum_low_time);
				//////printf("INTERRUPTION: delta_state_time %f\n",delta_state_time);
				go_on = 0;
			} else {
				switch (ev->type)
				 {
				   case ARRIVAL:   arrival(C);
						  break;
				   case DEPARTURE: departure(C);               //what if a packet sHould go out in the change status time????????????? in this case we send it.<-ask
						  break;
				   default:       printf("This is awful!!!\n");
						  exit(1);
				 }
				release_event(ev);
			}
		}
	}

	big_delta =  sum_high_time + sum_low_time;
	//current_time and big_delta should be equal, but they are not
	//a conversion is needed
	//energy spent using big_delta (using the sum_x_time)
//	total_energy = (big_delta - idle_time)*powerH + on_off_switching*on_off_power;
	total_energy = big_delta*powerH + on_off_switching*on_off_power;
	//energy spent per second using big_delta
	energy_per_second = total_energy/big_delta;
	energy_consumption = energy_per_second*current_time;

	results(); 
}
