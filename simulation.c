#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define Q_LIMIT 100 /* limit on the queue length */

/*mnemonics for servers state */
#define BUSY 1
#define IDLE 0

int next_event_type, num_cust_delayed, num_delays_required, num_events, num_in_q, server_status;

float area_num_in_q, area_server_status, mean_interarrival,
      mean_service, time, time_arrival[Q_LIMIT + 1],
      time_last_event, time_next_event[3], tota1_of_delays;

FILE *infile, *outfile;

void initialize (void) ;
void timing (void) ;
void arrive(void) ;
void depart(void) ;
void report(void);
void update_time_avg_stats(void);
float expon(float mean) ;
float getrand();


int main()
{
    //open input and output files
    infile = fopen("mm1alt.in", "r");
    outfile = fopen("mm1alt.out", "w");

    //specify the number of events for the timing events
    num_events = 2;

    //read input parameters
    fscanf(infile, "%f %f %d", &mean_interarrival, &mean_service, &num_delays_required);

    //write report heading and input paramaters
    fprintf(outfile, "Single-server queueing system with fixed run");
    fprintf(outfile, "length\n\n") ;
    fprintf (outfile, "Mean interarrival time%11.3f minutes\n\n", mean_interarrival) ;
    fprintf (outfile, "Mean service time%16.3f minutes\n\n" ,  mean_service);
    fprintf(outfile, "Number of customers %14d minutes\n\n", num_delays_required);

    // initialize the simulation
    initialize();


    /* run the simulation until it terminates after an end-simulation event occurs*/
    while (num_cust_delayed < num_delays_required)
    {
        //determine the next event
        timing();

        //update the time-average statistical accumulators
        update_time_avg_stats();

        //invoke the next event function
        switch(next_event_type)
        {
        case 1:
            arrive();
            break;
        case 2:
            depart();
            break;
        }
    }

    report();

    fclose(infile);
    fclose(outfile);

    return 0;
}

void initialize(void)
{
    //initialize the simulation clock
    time = 0.0;

    // initialiize state variables
    server_status = IDLE;
    num_in_q = 0;
    time_last_event = 0.0;


    //initialize statistical counters
    num_cust_delayed = 0;
    tota1_of_delays = 0.0;
    area_num_in_q = 0.0;
    area_server_status =0.0;

    /* Initialize event list.
    Since no customers are present, the
    departure (service completion) event is eliminated from
    consideration. The end-simulation event (type 3) is SCheduled
    for time time_end. */

    time_next_event[1] = time + expon(mean_interarrival);
    time_next_event[2] = 1.0e+30;

}

void timing(void){
    // timing function
    int i;
    float min_time_next_event = 1.0e+29;
    next_event_type = 0;

    // determine the event type of the next even to occur
    for(i=1; i <= num_events; ++i){
        if(time_next_event[i] < min_time_next_event){
            min_time_next_event =  time_next_event[i];
            next_event_type = i;
        }
    }

    //check to see whether the event list is empty
    if(next_event_type == 0){
        //event list is empty, so stop the simulation
        fprintf(outfile, "\nEvent list at time %f", time);
        exit(1);
    }

    // the event list is not empty so advance the simulation clock
    time = min_time_next_event;
}

void arrive(void){
    //arrival event function
    float delay;

    // schedule next arrival
    time_next_event[1] = time + expon(mean_interarrival);

    // check to see whether the server is busy
    if(server_status == BUSY){
        //server is busy, increment customers in queue
        ++num_in_q;

        //check whether an overflow condition exists
        if(num_in_q > Q_LIMIT){
            // the queue has overflowed, so stop the simulation
            fprintf(outfile, "Overflow of the array, time arrival at time %f", time);
            exit(2);
        }
        //else there is still room in the queue, so store the time of arrival of the customer
        time_arrival[num_in_q] = time;
    }
    else{
        //the server is idle, customer enters service immediately
        delay = 0.0;
        tota1_of_delays += delay;

        // increment number of customers delayed and change the server status
        ++num_cust_delayed;
        server_status == BUSY;

        //schedule service completion.
        time_next_event[2] = time + expon(mean_service);
    }

}

void depart(void){
    //depart event function
    int i;
    float delay;

    //check to see whether the queue is empty
    if (num_in_q == 0){
        // make the server idle. eliminate the departure event from consideration
        server_status == IDLE;
        time_next_event[2] = 1.0e+30;
    }
    else{
        //queue is not empty, so decrement the number of customers in queue.
        -- num_in_q;

        // compute the delay of customer who is beginning service and update total delay accumulator
        delay = time - time_arrival[i]; //////
        tota1_of_delays += delay;

        //increnent the number of customers delayed and schedule departure.
        ++num_cust_delayed;
        time_next_event[2] = time + expon(mean_service);

        // move customers in queue (if any) one place up
        for(i = 1; i <= num_in_q; ++i){
            time_arrival[i] = time_arrival[i+1];
        }
    }
}

void report(void){
    /* compute and write estimates of desired measures of performance.*/
    fprintf(outfile, "\n\nAverage delay in queue%11.3f minutes\n\n", (tota1_of_delays/num_cust_delayed));
    fprintf(outfile, "\n\nAverage number in queue%10.3f\n\n", (area_num_in_q/time));
    fprintf(outfile, "\n\nServer Utilization%15.3f minutes\n\n", (area_server_status/time));
    fprintf(outfile, "\n\nNumber of delays completed%7d\n\n", num_cust_delayed);
    fprintf(outfile, "Time simulation ended %12.3f", time);
}

void update_time_avg_stats(void){
    //update area accumulators for time-average statistics
    float time_since_last_event;

    //compute time since last event and update last-time-event marker
    time_since_last_event = time - time_last_event;
    time_last_event = time;

    /* Update area under number-in-queue function. */
    area_num_in_q += num_in_q * time_since_last_event;

    /*'Update area under server-busy indicator function. */
    area_server_status += server_status * time_since_last_event;
}

float getrand(){
    float r = rand();
    return(r / RAND_MAX);
}

float expon(float mean){
    float u;
    // u = rand();
    u = getrand();

    return -mean * log(u);
}


