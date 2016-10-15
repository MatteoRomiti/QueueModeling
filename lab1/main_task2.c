#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event.h"
#include "record.h"
#include "simtime.h"

#define ARRIVAL    1
#define DEPARTURE1  2
#define DEPARTURE2  3
#define DEPARTURE3  4

long seme = 14123451;

Event *event_list = NULL;
Record *queue=NULL;
Record *in_service1=NULL;
Record *in_service2=NULL;
Record *in_service3=NULL;
double lambda,mu;
int total_users = 0;
int sum_total_users = 0;
int queuing_users = 0;
int lost_users = 0;
int max_number_of_users = 5; //B
int number_of_servers = 3; //k
int queue_size;
int number_of_samples = 0;
int number_of_busy_servers = 0;
double cumulative_time_user = 0;
Time total_delay = 0;
Time last_event_time = 0; //total_delay = forall(users) sum(departure_time - arrival_time)
Time current_time;
Time idle_time1 = 0;
Time idle_time2 = 0;
Time idle_time3 = 0;
Time start_idle_time1 = 0.0;
Time start_idle_time2 = 0.0;
Time start_idle_time3 = 0.0;
Time busy_servers_times[] = {0.0, 0.0, 0.0}; // 3 = k
Time elapsed_time = 0.0; // 3 = k
Time start_x_servers_busy_time[] = {0.0, 0.0, 0.0}; // 3 = k

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
{ 
  Time delta;
  Record *rec;
  delta = negexp(1.0/lambda,&seme);
  schedule(ARRIVAL,current_time+delta); // schedule a future arrival
  //printf("next arrival added to the event_list\n");
  rec = new_record();
  rec->arrival = current_time;
  cumulative_time_user+=total_users*(current_time-last_event_time);
  total_users++;
  sum_total_users++;
  
  if (in_service1 == NULL){ 
    // the server was idle
    //printf("one record is now served in server 1\n");
    in_service1 = rec; // put the record in service 
    number_of_busy_servers++; // how many servers are busy
    start_x_servers_busy_time[number_of_busy_servers-1] = current_time; //update the start time 
   // update the idle_time
    idle_time1 = idle_time1 + (current_time - start_idle_time1); //start idle time is captured in the function departure
    //printf("idle_time1 = %f\n", idle_time1);
    //double service_time = negexp(1.0/mu,&seme);
    double service_time = geometric1(1.0/mu,&seme);
    schedule(DEPARTURE1,current_time+service_time); // schedule the departure of this arrival
    //printf("Service time 1 = %f\n",service_time);
    //printf("%f\n",service_time);
    //printf("Next departure added to the event_list\n");
  } else if(in_service2 == NULL){
    idle_time2 = idle_time2 + (current_time - start_idle_time2); //start idle time is captured in the function departure
    //printf("idle_time2 = %f\n", idle_time2);
    //start the service on the server 2
    in_service2 = rec;
    number_of_busy_servers++;
    start_x_servers_busy_time[number_of_busy_servers-1] = current_time; //update the start time 
    ////printf("one record is now served in server 2\n");
    //double service_time = negexp(1.0/mu,&seme);
    double service_time = geometric1(1.0/mu,&seme);
    schedule(DEPARTURE2,current_time+service_time); // schedule the departure of this arrival
    //printf("Service time 2 = %f\n",service_time);
    //printf("%f\n",service_time);
    //printf("Next departure added to the event_list\n");
  } else if(in_service3 == NULL){
    idle_time3 = idle_time3 + (current_time - start_idle_time3); //start idle time is captured in the function departure
    //printf("idle_time3 = %f\n", idle_time3);
    //start the service on the server 3
    in_service3 = rec;     
    number_of_busy_servers++;
    start_x_servers_busy_time[number_of_busy_servers-1] = current_time; //update the start time 
    //printf("one record is now served in server 3\n");
    //double service_time = negexp(1.0/mu,&seme);
    double service_time = geometric1(1.0/mu,&seme);
    schedule(DEPARTURE3,current_time+service_time); // schedule the departure of this arrival
    //printf("Service time 3 = %f\n",service_time);
    //printf("%f\n",service_time);
    //printf("Next departure added to the event_list\n");
  } else {
    if(queuing_users < queue_size){ 
      in_list(&queue,rec); // insert a record in the queue
      queuing_users++;
      //printf("All servers are busy. One user added to the queue. %d users in the queue \n", queuing_users);
    } else {
      //get rid of this unlucky user
      release_record(rec);
      lost_users++;
      //printf("Lost one user! \n");

    }
      
  }
  return;
}

void departure1(void)
{
  Record *rec;
  rec = in_service1;
  in_service1 = NULL;

  elapsed_time = current_time - start_x_servers_busy_time[number_of_busy_servers-1];
  busy_servers_times[number_of_busy_servers-1] +=  elapsed_time; // increment the time
  number_of_busy_servers--;
  
  cumulative_time_user+=total_users*(current_time-last_event_time);
  total_users--;
  number_of_samples++;
  total_delay+=current_time - rec->arrival;
  release_record(rec);
  //printf("One user has left from server 1\n");

  if (queue!=NULL)
   {
    in_service1 = out_list(&queue); // take a record from the queue and serve it
    queuing_users--;
    number_of_busy_servers++;
    elapsed_time = current_time - start_x_servers_busy_time[number_of_busy_servers-1];
    start_x_servers_busy_time[number_of_busy_servers-1] = current_time; //update the start time 
    busy_servers_times[number_of_busy_servers-1] +=  elapsed_time; 
    //printf("One user taken from the queue\n");
    //p is the probability of success on each trial (parameter of geom.distr)
    //from https://en.wikipedia.org/wiki/Geometric_distribution#Related_distributions
    //double p = 1-exp(-mu); 
    //double mean = (1-p)/p;
    //printf("Mean service time %f\n",mean);
    //schedule(DEPARTURE,current_time+geometric1(1.0/mu,&seme)); 
    // double service_time = negexp(1.0/mu,&seme);
    double service_time = geometric1(1.0/mu,&seme);
    schedule(DEPARTURE1,current_time+service_time); // schedule the departure of this arrival
    //printf("Service time 1 = %f\n",service_time);
    //printf("%f\n",service_time);
    //printf("Next departure added to the event_list\n"); 
   } else {
    // the queue is empty, so the server is idle
    start_idle_time1 = current_time;
    //printf("The server 1 is now idle\n\n");
   }
  return;
}

void departure2(void)
{
  Record *rec;
  rec = in_service2;
  in_service2 = NULL;

  elapsed_time = current_time - start_x_servers_busy_time[number_of_busy_servers-1];
  busy_servers_times[number_of_busy_servers-1] +=  elapsed_time; // increment the time
  number_of_busy_servers--;

  cumulative_time_user+=total_users*(current_time-last_event_time);
  total_users--;
  number_of_samples++;
  total_delay+=current_time - rec->arrival;
  release_record(rec);
  //printf("One user has left from server 2\n");
  if (queue!=NULL)
   {
    in_service2 = out_list(&queue); // take a record from the queue and serve it
    queuing_users--;
    number_of_busy_servers++;
    elapsed_time = current_time - start_x_servers_busy_time[number_of_busy_servers-1];
    start_x_servers_busy_time[number_of_busy_servers-1] = current_time; //update the start time 
    busy_servers_times[number_of_busy_servers-1] +=  elapsed_time; 
    //printf("One user taken from the queue\n");
    //p is the probability of success on each trial (parameter of geom.distr)
    //from https://en.wikipedia.org/wiki/Geometric_distribution#Related_distributions
    //double p = 1-exp(-mu); 
    //double mean = (1-p)/p;
    //printf("Mean service time %f\n",mean);
    //schedule(DEPARTURE,current_time+geometric1(1.0/mu,&seme)); 
    // double service_time = negexp(1.0/mu,&seme);
    double service_time = geometric1(1.0/mu,&seme);
    schedule(DEPARTURE2,current_time+service_time); // schedule the departure of this arrival
    //printf("Service time 2 = %f\n",service_time);
    //printf("%f\n",service_time);
    //printf("Next departure added to the event_list\n"); 
   } else {
    // the queue is empty, so the server is idle
    start_idle_time2 = current_time;
    //printf("The server 2 is now idle\n\n");
   }
  return;
}

void departure3(void)
{
  Record *rec;
  rec = in_service3;
  in_service3 = NULL;
  elapsed_time = current_time - start_x_servers_busy_time[number_of_busy_servers-1];
  busy_servers_times[number_of_busy_servers-1] +=  elapsed_time; // increment the time
  number_of_busy_servers--;
  cumulative_time_user+=total_users*(current_time-last_event_time);
  total_users--;
  number_of_samples++;
  total_delay+=current_time - rec->arrival;
  release_record(rec);
  //printf("One user has left from server 3\n");
  if (queue!=NULL)
   {
    in_service3 = out_list(&queue); // take a record from the queue and serve it
    queuing_users--;
    number_of_busy_servers++;
    elapsed_time = current_time - start_x_servers_busy_time[number_of_busy_servers-1];
    start_x_servers_busy_time[number_of_busy_servers-1] = current_time; //update the start time 
    busy_servers_times[number_of_busy_servers-1] +=  elapsed_time; 
    //printf("One user taken from the queue\n");
    //p is the probability of success on each trial (parameter of geom.distr)
    //from https://en.wikipedia.org/wiki/Geometric_distribution#Related_distributions
    //double p = 1-exp(-mu); 
    //double mean = (1-p)/p;
    //printf("Mean service time %f\n",mean);
    //schedule(DEPARTURE,current_time+geometric1(1.0/mu,&seme)); 
    // double service_time = negexp(1.0/mu,&seme);
    double service_time = geometric1(1.0/mu,&seme);
    schedule(DEPARTURE3,current_time+service_time); // schedule the departure of this arrival
    //printf("Service time 3 = %f\n",service_time);
    //printf("%f\n",service_time);
    //printf("Next departure added to the event_list\n"); 
   } else {
    // the queue is empty, so the server is idle
    start_idle_time3 = current_time;
    //printf("The server 3 is now idle\n\n");
   }
  return;
}

void results(void)
{
 cumulative_time_user+=total_users*(current_time-last_event_time); // ???
 double rho= lambda/mu;
 double total_busy_time = busy_servers_times[0]+ busy_servers_times[1]+busy_servers_times[2];
 double average_busy_servers = (busy_servers_times[0] + 2.0*busy_servers_times[1] + 3.0*busy_servers_times[2])/current_time;
 double measured_loss_probability = (double) lost_users/sum_total_users;
 // P-K formula M/M/1
 //double Cs2 = 1;
 //double E_S = 1.0/mu;
 
 //P-K formula M/G/1
 //lambda and mu must be values between 0 and 1
 double Cs2=1-mu; 
 double E_S = 1.0/mu; 
 double E_N= rho+rho*rho*(1+Cs2)/(2*(1-rho)); 
 double E_T = E_S+rho*E_S*(1+Cs2)/(2*(1-rho));

 printf("\n*************results******************\n\n");

 //printf("Final time %f\n",current_time);
 printf("Number of services %d\n",number_of_samples);
// printf("Measured average number of users %f\n",cumulative_time_user/current_time); //always true
  printf("Number of total users %d\n",sum_total_users);

 printf("Number of lost users %d\n",lost_users);

 //TRUE FOR M/M/1
/*
 printf("Theoretical average time in the system %f\n",1.0/(mu-lambda)); //true for M/M/1
 printf("Theoretical average time in the queue %f\n",rho/(mu-lambda)); //true for M/M/1
 printf("Theoretical average number of users %f\n",lambda/(mu-lambda)); //true for M/M/1
*/  

/*
 printf("Pollaczek-Khintchin formula \n");
 printf("Theoretical average number of users = %f\n",E_N);
 printf("Theoretical average time in the system = %f\n",E_T);
*/
/*
 printf("Measured average time in the system   %f\n",total_delay/number_of_samples); //always true
 printf("Measured idle server 1 probability   %f\n",idle_time1/current_time); //always true
 printf("Measured idle server 2 probability   %f\n",idle_time2/current_time); //always true
 printf("Measured idle server 3 probability   %f\n",idle_time3/current_time); //always true
 printf("server 1 was busy for %f seconds\n",busy_servers_times[0]); 
 printf("server 2 was busy for %f seconds\n",busy_servers_times[1]); 
 printf("server 3 was busy for %f seconds\n",busy_servers_times[2]); 
 printf("measured average number of busy servers: %f\n",average_busy_servers); 
 printf("measured customer loss probability: %f\n", measured_loss_probability);
*/
 //printf("Sum of all the busy times %f\n",total_busy_time);
 
 
 //short printf
 //printf("%d\n",number_of_samples);
 //printf("%f\n",cumulative_time_user/current_time); //always true
 //printf("%d\n",lost_users);
 printf("%f\n",total_delay/number_of_samples); //always true
 printf("%f\n",idle_time1/current_time); //always true
 printf("%f\n",idle_time2/current_time); //always true
 printf("%f\n",idle_time3/current_time); //always true
 //printf("%f \n",busy_servers_times[0]); 
 //printf("%f\n",busy_servers_times[1]); 
 //printf("%f\n",busy_servers_times[2]); 
 printf("%f\n", average_busy_servers); 
 printf("%f\n", measured_loss_probability);
 printf("\n*****************************************\n\n");
 exit(0);
}

int main()
{ 
  printf("\n\n*****************************************\n\n");
  Event *ev;
  Time maximum;
  
  queue_size = max_number_of_users - number_of_servers;

  printf("Insert arrival rate, lambda: ");
  get_input("%lf",&lambda);
  printf("lambda = %f\n",lambda);
  printf("Insert serive rate, mu: ");
  get_input("%lf",&mu);
  printf("mu     = %f\n",mu);
  printf("Insert simulation time: ");
  get_input("%lf",&maximum);
  printf("Max Time  = %f\n\n",maximum);
  double arrival_time = negexp(1.0/lambda,&seme);
  schedule(ARRIVAL, arrival_time);
  //printf("first arrival_time %f\n", arrival_time);
  //printf("first arrival added to the event_list\n");

  while (current_time<maximum){
    ev = get_event(&event_list); // take the last event 
    last_event_time = current_time;
    current_time = ev->time; // service time
    switch (ev->type){
      case ARRIVAL:           
        //printf("Start arrival\n");
        arrival();
        break;
      case DEPARTURE1: 
        //printf("Start departure1\n");
        departure1();
        break;
      case DEPARTURE2: 
        //printf("Start departure2\n");
        departure2();
        break;
      case DEPARTURE3: 
        //printf("Start departure3\n");
        departure3();
        break;
      default:       
        printf("This is awful!!!\n");
        exit(1);
    }
    release_event(ev);
  }
  results();
}

