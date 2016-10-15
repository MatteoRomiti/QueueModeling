// WITH RATE ADAPTATION

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
int queue_size=10; 
int queuing_users=0;
int inputs; //input from consolle
int events_on_change_high = 0;
int events_on_change_low = 0;
int changing = 0;


//everything is in SECONDS
Time maximum=60.0*60*10;	//simulation time
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

void arrival(double C, int change){
	  if (print==1){printf("ARRIVAL\n");}
	  Time delta;
	  Record *rec;
/*	  if (change == 1)
	  	  {
	  	  	events_on_change_high++;
	  	  	printf("arrival during changing to high\n");
	  	  }	  
	  if (change == 2)
	  	  {
	  	  	events_on_change_low++;
	  	  	printf("arrival during changing to low\n");
	  	  }	  
*/	  
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

void departure(double C, int change)
{
  Record *rec;
  rec = in_service;
  in_service = NULL;
/*	  if (change == 1)
	  	  {
	  	  	events_on_change_high++;
	  	  	printf("departure during changing to high\n");
	  	  }	  
	  if (change == 2)
	  	  {
	  	  	events_on_change_low++;
	  	  	printf("departure during changing to low\n");
	  	  }	  
*/	    

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
   	if (print==1){printf("the queue is empty, so the server is idle\n");};
   	
   }
     
  if (print==1){printf("queuing users: %d\n",queuing_users); 
  printf("total users: %d\n",total_users);};
  return;
}

void results(void)
{
 cumulative_time_user+=total_users*(current_time-last_event_time); // ???
 //printf("\n******************************************\n");
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
 printf ("events during change to high: %d\n", events_on_change_high);
 printf ("events during change to low: %d\n", events_on_change_low);
*/

 //printf("%f\n",current_time);
/* printf("%f\n",sum_high_time);
 printf("%f\n",sum_low_time);
 printf("%f\n",sum_change_time);
 printf("%f\n",big_delta);
 printf("%d\n",number_of_samples);
*/ 
 //printf("%f\n",idle_time/current_time); 
 //printf("%f\n",total_delay/number_of_samples); //always true 
 //printf("%f\n",cumulative_time_user/current_time); //cumulative_time_user+=total_users*(current_time-last_event_time);
/* printf ("%f\n",lost_users);
 printf ("%f\n",sum_total_users);
*/ 
 //printf ("%f\n",lost_users/sum_total_users);
/* printf ("%f\n",total_energy);
 printf ("%f\n", energy_per_second);
*/ 
 //printf ("%f\n", energy_consumption);

// printf ("total delay%f\n", total_delay);
/* printf ("%d\n", events_on_change_high);
 printf ("%d\n", events_on_change_low);
*/
 printf("\n*****************************************\n\n"); 

	FILE *fp;
	fp = fopen("out______put.txt", "a+");
	fprintf(fp, "%f\n", total_delay/number_of_samples);
	fclose(fp);

/*	double num = 123412341234.123456789; 
	char output[50];

	snprintf(output, 50, "%f", total_delay/number_of_samples);
	fputs(, fp);

	printf("%s", output);
*/
 exit(0);
}

int main()
{
  
 Event *ev;
 //capacity 
 Ch = pkt_size/sH; // one pkt each sH seconds
 Cl = pkt_size/sL; // one pkt each sL seconds Time maximum;
 
 print = 0;
 //printf("print proccess? [1 0]: ");
 //scanf("%d",&print);
 //get_input("%ld",&print);

 //printf("insert inputs? [1 0]: ");
 ////scanf("%d",&inputs);
 //get_input("%ld",&inputs);
  
 //if (inputs==1){
 //printf("Insert arrival rate, H: ");
 //get_input("%lf",&lambdaH);
 //printf("Insert arrival rate, lambdaL: ");
 //get_input("%lf",&lambdaL);
 

 //printf("Insert serive rate high, Capacity, Ch: ");
 //get_input("%lf",&Ch);
 //printf("Insert serive rate low, Capacity, Cl: ");
 //get_input("%lf",&Cl); 
 
 
 //printf("Insert tHL: ");
 //get_input("%lf",&tHL);
 //printf("Insert tLH: ");
 //get_input("%lf",&tLH); 
 
 
 
 //printf("Insert simulation time: ");
 //get_input("%lf",&maximum);


 //printf("\n\n*****************************************\n");
 //printf("\n[*]Ch= %f bps[*]cl= %f bps    ---- 1/Ch: %f   1/Cl: %f",Ch,Cl,1/Ch,1/Cl);	
 //printf("\n[*]LambdaH= %f [*]lambdaL= %f",lambdaH,lambdaL);
 //printf("\n[*]tHL=%f & tLH=%f\n", tHL,  tLH);
 //printf("\n[*]max time=%f\n", maximum);
 //printf("\n\n*****************************************\n\n");
 //};
 
 // scheduling the first event (an arrival)
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
	while (current_time<tH && go_on)
	  {	
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
			switch (ev->type)
			 {
			   case ARRIVAL:   arrival(C, 0);
					  break;
			   case DEPARTURE: departure(C, 0);               //what if a packet sHould go out in the change status time????????????? in this case we send it.<-ask
					  break;
			   default:       printf("This is awful!!!\n");
					  exit(1);
			 }
			release_event(ev);
		}
	}
	//////delta_state_time = current_time - start_state_time;
	//////if (print==1 && go_on){printf("________________ delta_state_time _____________%f\n",delta_state_time);}
	//////sum_high_time = sum_high_time + delta_state_time;

	if (go_on){sum_change_time = sum_change_time + tHL;}
	if (print==1 && go_on){printf("-----------------changing to low-----------------\n");}
	if (print==1 && go_on){printf("current_time %f\n",current_time);}  

	start_state_time = current_time;
	// PROBLEM: if this loop does not call at least one arrival or one departure, the time does not change  
	while(current_time<tH+tHL && go_on)
	  {	
		lambda=lambdaL; // the arrive time changes but we keep using the last mu
		C = Ch;						 //still working with the last lamda
		ev = get_event(&event_list); // take the last event
		last_event_time = current_time;
		current_time = ev->time; // service time
		if (current_time > maximum){
			go_on = 0;
		} else {	  	
			events_on_change_low++;
			switch (ev->type)
			 {
			   case ARRIVAL:   arrival(C, 2);
					  break;
			   case DEPARTURE: departure(C, 2);               //what if a packet sHould go out in the change status time????????????? in this case we send it.<-ask
					  break;
			   default:       printf("This is awful!!!\n");
					  exit(1);
			 }
			release_event(ev);
		}
	}
	//////delta_state_time = current_time - start_state_time;
	//////if (print==1 && go_on){printf("________________ delta_state_time _____________%f\n",delta_state_time);}
	//////sum_change_time = sum_change_time + delta_state_time;

		
	t_add = negexp(mean_low, &seme); 	// time spent by the system in low rate
	tL= current_time+t_add;
	//////printf("delta low %f\n", t_add);
	if (go_on) {sum_low_time = sum_low_time + t_add;}
	if (print==1 && go_on){printf("sum_low_time: %f\n",sum_low_time);}  

	if (print==1 && go_on){printf("-----------------start low rate for: %f-----------------\n",t_add);}
	if (print==1 && go_on){printf("current_time %f\n",current_time);}  

	start_state_time = current_time;
	while (current_time<tL && go_on)
	  {	
		lambda=lambdaL;
		C = Cl;
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
			   case ARRIVAL:   arrival(C, 0);
					  break;
			   case DEPARTURE: departure(C, 0);               //what if a packet sHould go out in the change status time????????????? in this case we send it.<-ask
					  break;
			   default:       printf("This is awful!!!\n");
					  exit(1);
			 }
			release_event(ev);
		}
	}

	//////delta_state_time = current_time - start_state_time;
	//////if (print==1 && go_on){printf("________________ delta_state_time _____________%f\n",delta_state_time);}
	//////sum_low_time = sum_low_time + delta_state_time;

	 
	if (go_on) {sum_change_time = sum_change_time + tLH;}
	if (print==1 && go_on){printf("-------------changing to high-------------\n");}
	if (print==1 && go_on){printf("current_time %f\n",current_time);}  

	start_state_time = current_time;
	// PROBLEM: if this loop does not call at least one arrival or one departure, the time does not change  
	while(current_time<tL+tLH && go_on) //still working with the last lambda
	  {	
	  	changing = 1;
		lambda=lambdaH;
		C = Cl;						 
		ev = get_event(&event_list);
		last_event_time = current_time;
		current_time = ev->time;
		if (current_time > maximum){
			go_on = 0;
		} else {	  	
			events_on_change_high;
			switch (ev->type)
			 {
			   case ARRIVAL:   arrival(C, 1);
					  break;
			   case DEPARTURE: departure(C, 1);               //what if a packet sHould go out in the change status time????????????? in this case we send it.<-ask
					  break;
			   default:       printf("This is awful!!!\n");
					  exit(1);
			 }
			release_event(ev);
		}
	 }
	//////delta_state_time = current_time - start_state_time;
	//////if (print==1 && go_on){printf("________________ delta_state_time _____________%f\n",delta_state_time);}
	//////sum_change_time = sum_change_time + delta_state_time;

	}

	big_delta = sum_change_time + sum_high_time + sum_low_time;
	//current_time and big_delta should be equal, but they are not
	//a conversion is needed
	//energy spent using big_delta (using the sum_x_time)
	total_energy = sum_high_time*powerH + sum_low_time*powerL + sum_change_time*((powerH+powerL)/2);
	//energy spent per second using big_delta
	energy_per_second = total_energy/big_delta;
	energy_consumption = energy_per_second*current_time;

	results(); 
}
