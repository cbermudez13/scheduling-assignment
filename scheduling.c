/*PA1
* Written by Carlos Bermudez
* Compile: gcc scheduling.c -o scheduling
* Usage: ./scheduling [input file (optional)]
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define MAXWORDS 10
#define FCFS 1
#define SJF  2
#define RR   3
#define TRUE 1
#define FALSE 0
#define MAX_PROCESSES 100
#define NEW 0
#define READY 1
#define RUNNING 2
#define FINISHED 3
#define PRINT(fp,fmt, ...) {                      \
                printf (fmt, __VA_ARGS__);      \
                fprintf (fp, fmt, __VA_ARGS__); \
}
        
int cpu_status = 0;                     /*0 = No current PID running in CPU, otherwise contains PID*/
FILE *inputfile, *outputfile;

/* Queues */

struct QUEUE {
    int front, rear, size, capacity;
    int* array;
};


struct QUEUE* queue_init(int max) {
/* Initialize the queue 'q' with empty values */
    struct QUEUE* queue = (struct QUEUE*)malloc(sizeof(struct QUEUE));
    queue->capacity = max;
    queue->front = queue->size = 0;
 
    //see the enqueue
    queue->rear = max - 1;
    queue->array = (int*)malloc(queue->capacity * sizeof(int));
    return queue;
}


int isempty(struct QUEUE* queue) {
    return (queue->size == 0);
}

int isfull(struct QUEUE* queue) {
    return (queue->size == queue->capacity);
}


int dequeue(struct QUEUE* queue) {
    if (isempty(queue))
        return INT_MIN;
    int x = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return x;
}
 
// Function to get front of queue
int front(struct QUEUE* queue) {
    if (isempty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}
 
// Function to get rear of queue
int rear(struct QUEUE* queue) {
    if (isempty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}


void enqueue(struct QUEUE* queue, int x)
{
    if (isfull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = x;
    queue->size = queue->size + 1;
}

typedef struct process{                       //Processes struct
      int pid; 
      char *name;
      int arrival;
      int burst;
      int status;
      int time_remaining;
      int wait_time;
      int turn_around;
      int finished_time;
} PROCESS;
PROCESS processes[MAX_PROCESSES];

int compare(const void *a, const void *b){
  PROCESS *pa = (PROCESS *) a;
  PROCESS *pb = (PROCESS *) b;
  return (pa->arrival - pb->arrival);
}

void fcfs_algorithm(int processcount, int runfor){
  PRINT(outputfile,"%d processes\nUsing First Come First Served\n\n", processcount);
  int time;
  struct QUEUE* q_ready = queue_init(MAX_PROCESSES);
  int p;
  
  for(time = 0; time < runfor+1; time++){
    
    for(p = 0; p < processcount; p++){
      if(processes[p].arrival == time){                          /*Check for arrival of processes*/
        PRINT(outputfile,"Time %d: %s arrived\n",time,processes[p].name);
        enqueue(q_ready,processes[p].pid);
        processes[p].status = READY;
      }
    }
    
    if(cpu_status != 0){
      processes[cpu_status-1].time_remaining-= 1;
      if(processes[cpu_status-1].time_remaining == 0){
        PRINT(outputfile,"Time %d: %s finished\n",time,processes[cpu_status-1].name);
        processes[cpu_status-1].turn_around = time - processes[cpu_status-1].arrival;
        processes[cpu_status-1].finished_time = time;
        processes[cpu_status-1].status = FINISHED;
        cpu_status=0;
      } else {
        for(int j = 0; j < q_ready->size; j++){
          processes[q_ready->array[j+q_ready->front]-1].wait_time += 1;
        }
      }
    }
    
    if(!isempty(q_ready) && cpu_status == 0){
        p = dequeue(q_ready);
        PRINT(outputfile,"Time %d: %s selected (burst %d)\n",time,processes[p-1].name,processes[p-1].burst);
        cpu_status = p;
        processes[p-1].status = RUNNING;
      }
  }
  PRINT(outputfile,"Finished at time %d\n\n", time-1);
  for(int j = 0; j < processcount; j++){
    if(processes[j].status == FINISHED) {
      processes[j].wait_time = processes[j].finished_time - processes[j].burst - processes[j].arrival;
      PRINT(outputfile,"%s wait %d turnaround %d\n", processes[j].name, processes[j].wait_time, processes[j].turn_around);
    } 
    else if(processes[j].status == RUNNING) {
      PRINT(outputfile, "%s wait %d did not complete\n", processes[j].name, processes[j].wait_time); 
    } else
      PRINT(outputfile, "%s could not be scheduled\n", processes[j].name);
  }
}

int find_shortest(int *v, int processcount) {
   int shortest_time = INT_MAX;
   int found = FALSE;
   
   for(int i = 0; i < processcount; i++){
      if (processes[i].status == RUNNING || processes[i].status == READY)
          if (processes[i].time_remaining < shortest_time) {
              shortest_time = processes[i].time_remaining;
              *v = i;
              found = TRUE;
          }
   }
   return(found);
}
  
void sjf_algorithm(int processcount, int runfor){
  PRINT(outputfile,"%d processes\nUsing Shortest Job First (Pre)\n\n", processcount);
  int time;
  int p, t;
  
  for(time = 0; time < runfor+1; time++){
  
    for(p = 0; p < processcount; p++){
      if(processes[p].arrival == time){                          /*Check for arrival of processes*/
        PRINT(outputfile,"Time %d: %s arrived\n",time,processes[p].name);
        processes[p].status = READY; 
      }
    }
    
    if(cpu_status != 0){
      processes[cpu_status-1].time_remaining-= 1;        /*Update time remaining*/
      if(processes[cpu_status-1].time_remaining == 0){   
        PRINT(outputfile,"Time %d: %s finished\n",time,processes[cpu_status-1].name);
        processes[cpu_status-1].turn_around = time - processes[cpu_status-1].arrival;
        processes[cpu_status-1].finished_time = time;
        processes[cpu_status-1].status = FINISHED;
        cpu_status=0;
      } 
    }

    if(find_shortest(&t, processcount)){
        if(cpu_status != processes[t].pid) {            /*do not process if current pid in cpu is still shortest process time*/
           PRINT(outputfile,"Time %d: %s selected (burst %d)\n",time,processes[t].name,processes[t].time_remaining);
           cpu_status = processes[t].pid;               /*assign cpu to the pid with shortest time. Context change happens*/
           if(cpu_status != 0)
              processes[cpu_status-1].status = READY;   //SJF preemptive: takes process from the CPU 
           processes[t].status = RUNNING;               //put in the CPU the process with shortest remainder time
        }  
    }
    
    if(cpu_status == 0 && time != runfor){
      PRINT(outputfile,"Time %d: IDLE\n", time);
    }
  }
  PRINT(outputfile,"Finished at time %d\n\n", time-1);
  for(int j = 0; j < processcount; j++){
    if(processes[j].status == FINISHED) {
      processes[j].wait_time = processes[j].finished_time - processes[j].burst - processes[j].arrival;
      PRINT(outputfile,"%s wait %d turnaround %d\n", processes[j].name, processes[j].wait_time, processes[j].turn_around);
    } 
    else if(processes[j].status == RUNNING) {
      PRINT(outputfile, "%s wait %d did not complete\n", processes[j].name, processes[j].wait_time); 
    } else
      PRINT(outputfile, "%s could not be scheduled\n", processes[j].name);
  }
}

void rr_algorithm(int processcount, int runfor, int quantum){
  PRINT(outputfile,"%d processes\nUsing Round-Robin\nQuantum %d\n\n", processcount, quantum);
  int time;
  struct QUEUE* q_ready = queue_init(MAX_PROCESSES);
  int p;
  int current_quantum = quantum;
  
  for(time = 0; time < runfor+1; time++){
    //PRINT(outputfile,"Time %d:          init Quantum=%d\n",time, current_quantum);
    current_quantum--;
   
    for(p = 0; p < processcount; p++){
      if(processes[p].arrival == time){                          /*Check for arrival of processes*/
        PRINT(outputfile,"Time %d: %s arrived\n",time,processes[p].name);
        enqueue(q_ready,processes[p].pid);
        processes[p].status = READY;
      }
    }
    
    if(cpu_status != 0){                    /*if cpu is running a process*/
      processes[cpu_status-1].time_remaining-= 1;
      if(processes[cpu_status-1].time_remaining == 0){
        PRINT(outputfile,"Time %d: %s finished\n",time,processes[cpu_status-1].name);
        processes[cpu_status-1].turn_around = time - processes[cpu_status-1].arrival;
        processes[cpu_status-1].finished_time = time;
        processes[cpu_status-1].status = FINISHED;
        cpu_status=0;                /*cpu is free*/
        current_quantum = 0;
      } else {
        for(int j = 0; j < q_ready->size; j++){
          processes[q_ready->array[j+q_ready->front]-1].wait_time += 1;
        }
      }
    }
    
    if(!isempty(q_ready) && cpu_status == 0){           /*if cpu is free and there is a process waiting*/
        p = dequeue(q_ready);
        PRINT(outputfile,"Time %d: %s selected (burst %d)\n",time,processes[p-1].name,processes[p-1].time_remaining);
        cpu_status = p;
        processes[p-1].status = RUNNING;
        current_quantum = quantum;
    }

    if(cpu_status != 0 && current_quantum <= 0){     //Check if current process in the CPU needs get out by quantum
        enqueue(q_ready,processes[cpu_status-1].pid);    /*put current process in waiting queue*/
        processes[cpu_status-1].status = READY;
        p = dequeue(q_ready);                        /*get the first process from waiting queue*/
        PRINT(outputfile,"Time %d: %s selected (burst %d)\n",time,processes[p-1].name,processes[p-1].time_remaining);
        cpu_status = p;
        processes[p-1].status = RUNNING;
        current_quantum = quantum;      /*restart quantum*/
    }

    if(cpu_status == 0 && time != runfor){
      PRINT(outputfile,"Time %d: IDLE\n", time);
    }

  }
  PRINT(outputfile,"Finished at time %d\n\n", time-1);
  for(int j = 0; j < processcount; j++){
    if(processes[j].status == FINISHED) {
      processes[j].wait_time = processes[j].finished_time - processes[j].burst - processes[j].arrival;
      PRINT(outputfile,"%s wait %d turnaround %d\n", processes[j].name, processes[j].wait_time, processes[j].turn_around);
    } 
    else if(processes[j].status == RUNNING) {
      PRINT(outputfile, "%s wait %d did not complete\n", processes[j].name, processes[j].wait_time); 
    } else
      PRINT(outputfile, "%s could not be scheduled\n", processes[j].name);
  }

}

int main(int argc, char *argv[]) {
    char *line = NULL;
    char *a[MAXWORDS];  //Array of words of a line
    size_t len = 0;
    ssize_t nread;     // Length of a line in the input file
    int i, runfor = 0;
    int processcount = 0;
    int processtotal = 0; //Qty of processes read from input file
    int algorithm = 0;
    int quantum = 0;
    char *processname;
    int arrival = 0;
    int burst = 0;
    int npid = 0;
    int max_time = 0; 
    
    

    if (argc == 2)                                //If there is a 2nd argument
        inputfile = fopen(argv[1], "r");          //Open the file in the 2nd argument
    else
        inputfile = fopen("processes.in", "r");   //By default will open processes.in 

    if (inputfile == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    outputfile = fopen("processes.out", "w");
    if (outputfile == NULL) {
        perror("fopen for output");
        exit(EXIT_FAILURE);
    }
    //Read every single 'line' in the input file
    while ((nread = getline(&line, &len, inputfile)) != -1) {
        
		    char * word = strtok(line, " ");           //Get the 1st word (token) in the 'line'
        i = 0;                                     //Initialize Number of Words
	      while( word != NULL && i < MAXWORDS) {     //Get all words in the 'line'
           a[i] = (char *) malloc(sizeof(word));   //Allocate memory for the string or word
           sprintf(a[i], word, "%s");              //Store the 'word' into the array
           word = strtok(NULL, " ");               //Get the next word (token) in the 'line'
           i++;                                    //Number of words
        }
        /*Next code segment analyze every single line */
	      if(!strcmp(a[0], "processcount") && i>0) { //If 1st word of the line is 'processcount'
           processcount = atoi(a[1]);              //Obtain 'processcount' from the 2nd string of the line 
        }
        else if(!strcmp(a[0], "runfor") && i>0) {      //If 1st word of the line is 'runfor'
           runfor = atoi(a[1]);                   //Obtain 'runfor' value from the 2nd string of the line 
        }
        else if(!strcmp(a[0], "use") && i>0) {         //If 1st word of the line is 'use'
           if(!strcmp(a[1], "fcfs")) algorithm = FCFS;  //Obtain 'algorithm' name from the 2nd string of the line
           if(!strcmp(a[1], "sjf"))  algorithm = SJF;
           if(!strcmp(a[1], "rr"))   algorithm = RR;
        }
        else if(!strcmp(a[0], "quantum") && i>0) {    //If 1st word of the line is 'quantum'
           quantum = atoi(a[1]);                 //Obtain 'quantum' value from the 2nd string of the line
        }
        else if(!strcmp(a[0], "process") && i>6) {    //If 1st word of the line is 'process'
           
           processname = (char *) malloc(sizeof(a[2])); //Initialize 'processname' string
           sprintf(processname, a[2], "%s");     //Obtain 'processname' from the 3rd string of the line
           arrival = atoi(a[4]);                 //Obtain 'arrival' value (ascii to int) from the 5th string of the line
           burst = atoi(a[6]);                   //Obtain 'burst' value (ascii to int) from the 7th string of the line
           processtotal++;                       //Increase qty process read from input file
           processes[npid].name = (char *) malloc(sizeof(a[2]));
           sprintf(processes[npid].name, processname, "%s"); //Enter procecess into PCB processes
           processes[npid].arrival = arrival;
           processes[npid].burst = burst;
           processes[npid].status = NEW;
           processes[npid].time_remaining = burst;
           processes[npid].wait_time = 0;
           processes[npid].turn_around = 0;
           npid++;
           max_time+=burst;
           
        }
        else if(a[0][0] == 'e' && a[0][1] == 'n' && a[0][2] == 'd') {  //If 1st word of the line is 'end'
           break;                                //Stop reading input file
        }
        else 
           printf( "   -----> Ignoring %s\n", a[0]);
    }
    free(line);                                 //Release memory
    fclose(inputfile);                          //Close input file
    
    
    //Perform some validations
    if(processcount != processtotal) {
       printf( " -----> processcount read from input file do not correspond to qty of processes. Please check input file\n");
       exit(EXIT_SUCCESS);
    }
    if(algorithm == RR && quantum == 0) {
       printf( " -----> Round-Robin algorithm requires a 'quantum' value. Please check input file\n");
       exit(EXIT_SUCCESS);
    }

    qsort(processes,npid,sizeof(PROCESS),compare);        //Sort processes by arrival time
    for(int i = 0; i < processcount; i++){
      processes[i].pid = i + 1;                           //System assign the pid by arrival time
    }
    
    //Call the given algorithm
    switch (algorithm) {
    case FCFS:  //First-Come First-Served
        fcfs_algorithm(processcount, runfor);
        break;
    case SJF:  //Preemptive Shortest Job First
        sjf_algorithm(processcount,runfor);
        break;
    case RR:  //Round-Robin
        rr_algorithm(processcount,runfor,quantum);
        break;
    default: 
         printf("No algorithm detected in input file\n");
         exit(EXIT_FAILURE);
    }
    fclose(outputfile);                          //Close output file
    exit(EXIT_SUCCESS);
}
