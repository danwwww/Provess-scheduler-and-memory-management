#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define MAX_LINE 1024
#define MAX_PROCESSES 200
#define MAX_pages 2000
#define MIN_MEMORY 16
#define INT_MAX 2147483647;

struct process{
    int first_arrival_time;
    float last_arrival_time;
    int id;
    int required_memory;
    int required_time;
    int used_memory;
    int remaining_time;
    int pages[MAX_pages];

};

//The function used to cal the throughput
void cal_throughput(int *through_put, int interval){
    int average=0;
    int min=INT_MAX;
    int max=-1;

    for (int i=0;i<interval;i++){
        if(through_put[i]>max){
            max=through_put[i];
        }
        if(through_put[i]<min){
            min=through_put[i];
        }
        average+=through_put[i];
    }
    if(average%interval==0){
        average=average/interval;
    }
    else{
        average=average/interval+1;
    }
    fprintf(stdout,"Throughput %d, %d, %d\n",average,min,max);
}

//The function used to cal the turnaround
void cal_turnaround(int turnaround, int total_process){

    if(turnaround%total_process==0){
        turnaround=turnaround/total_process;
    }
    else{
        turnaround=turnaround/total_process+1;
    }
    fprintf(stdout,"Turnaround time %d\n",turnaround);
}

void cal_time_overhead(float *time_overhead,int total_process){
    float max=-1;
    float average;
    float t_process=total_process;

    for (int i=0;i<total_process;i++){
        if(time_overhead[i]>max){
            max=time_overhead[i];
        }
        average+=time_overhead[i];
    }

        average=average/t_process;


    fprintf(stdout,"Time overhead %0.2f %0.2f\n",max,average);



};

//The compare function used to sort the processes according to their arrival time and ID
int cmp_struct(const void *a, const void *b)
{
   struct process*p1=(struct process*)a;
   struct process*p2=(struct process*)b;
    if(p1->last_arrival_time!=p2->last_arrival_time)
        return (p1->last_arrival_time>p2->last_arrival_time)?1:-1;
    else return p1->id-p2->id;
}

// The function used in the customerized scheduling(Shortest job first), sort the queue according to their job time, The first process will not be sorted.
void sort_by_time(struct process arr[], int size)
{
    int i, j, k;
    struct process tmp;

    for (i = 1; i < size - 1; i++) {
        k = i;
        for (j = i + 1; j < size; j++) {
            if (arr[j].required_time < arr[k].required_time) {
                k = j;
            }
        }
        tmp = arr[k];
        arr[k] = arr[i];
        arr[i] = tmp;
    }
}


//second comp
int cmp_int (const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

//function used to pop the element from the array
void struct_array_pop(struct process *array, int index, int array_length)
{
    for(int i = index; i < array_length - 1; i++){
        array[i] = array[i + 1];
    }
}



void array_pop(int *array, int index, int array_length)
{
    for(int i = index; i < array_length - 1; i++){
        array[i] = array[i + 1];
    }
}

//function used to allocate the memory
float allocate_memory(struct process processes[], int totoal_pages,float memory_array[],int ID_array[],int *memory, float time){
    float load_time=0;
    float counter=0;
    for (int i=0;i< totoal_pages;i++){
        if (memory_array[i]==-1){
            memory_array[i]=counter*0.1+time;
            ID_array[i]=processes[0].id;

            processes[0].pages[processes[0].used_memory/4]=i;
            processes[0].used_memory+=4;
            load_time+=2;
            *memory-=4;
            counter+=1;
        }
         //sort the page in the pages list of this process
        qsort(processes[0].pages,processes[0].used_memory/4, sizeof(int),cmp_int);
        if(*memory==0 || processes[0].used_memory==processes[0].required_memory){
            break;
        }
    }
    return load_time;
}

void print_running(struct process processes[],int init_memory,int memory,float time,float load_time,int memory_options){


    if(memory_options!=1){
        int mem_use;
        if((init_memory-memory)*100%init_memory==0){
            mem_use=(init_memory-memory)*100/init_memory;
        }
        else{
            mem_use=(init_memory-memory)*100/init_memory+1;
        }



        fprintf(stdout,"%.0f, RUNNING, id=%d, remaining-time=%d, load-time=%.0f, mem-usage=%d%%, mem-addresses=[%d", time,processes[0].id,processes[0].remaining_time,load_time,mem_use,processes[0].pages[0]);
        for (int i=1;i<processes[0].used_memory/4;i++){
            fprintf(stdout,",%d",processes[0].pages[i]);
    }
    fprintf(stdout,"]\n");
    }

    else{
        fprintf(stdout,"%.0f, RUNNING, id=%d, remaining-time=%d\n", time,processes[0].id,processes[0].remaining_time);
    }

}

void print_evicted(float time,int evicted[], int used_pages){
    fprintf(stdout,"%.0f, EVICTED, mem-addresses=[%d", time,evicted[0]);
    for (int k=1;k<used_pages;k++){
        fprintf(stdout,",%d",evicted[k]);
    }
    fprintf(stdout,"]\n");

}





int main(int argc, char *argv[]) {
    FILE *fp;
    char *file;
    char buf[MAX_LINE];
    char seps[]   = " ,\t\n";
    struct process processes[MAX_PROCESSES];

    int through_put[MAX_PROCESSES];
    float time_overhead[MAX_PROCESSES];
    int next_id,next_memory,next_required_time,next_arrival,total_pages;
    int total_process=0,total_process_remain=0,process_count=0,interval=0,completed_pross=0,turnaround =0;
    int current_running=-1;
    float time=0,load_time = 0;

    int schedule;
    int init_memory=40;
    int memory =40;
    int round_robin=10;
    int quantum=10;
    int memory_options;

    int opt;


    while ((opt = getopt(argc, argv, "f:a:m:s:q:")) != -1) {
        switch (opt) {
            case 'f':
                file = optarg;
                break;
            case 'a':
                // First Come First Serve
                if (strcmp(optarg,"ff")==0){
                    schedule=1;
                }
                // Round-Robin
                if (strcmp(optarg,"rr")==0){
                    schedule=2;
                }

                //Shortest job first
                if (strcmp(optarg,"cs")==0){
                    schedule=3;
                }
                break;
            case 'm':
                if (strcmp(optarg,"u")==0){
                    memory_options=1;
                }
                if (strcmp(optarg,"p")==0){
                    memory_options=2;
                }
                if (strcmp(optarg,"v")==0){
                    memory_options=3;
                }
                if (strcmp(optarg,"cm")==0){
                    memory_options=4;
                }
                break;
            case 's':
                init_memory=atoi(optarg);
                memory = atoi(optarg);
                break;
            case 'q':
                round_robin = atoi(optarg);
                quantum=atoi(optarg);
                break;
            default:
                continue;

        }
    }

    //if the scheduler is not round-robin
    if(schedule==1||schedule==3){
        round_robin = 9999;
        quantum=9999;
    }



    //count the total number of processes
    fp = fopen(file,"r");
    while(fgets(buf,MAX_LINE,fp)!=NULL){
        total_process_remain+=1;
        total_process+=1;
    }
    fclose(fp);

    // initialize memory

    total_pages=memory/4;
    float memory_array[total_pages];
    int ID_array[total_pages];

        for (int i =0;i<(total_pages);i++){
            memory_array[i]=-1;
            ID_array[i]=-1;
        }


    //read all the process into the process queue
    fp = fopen(file,"r");
    fgets(buf,MAX_LINE,fp);
    next_arrival = atof(strtok(buf, seps));
    next_id= atoi(strtok(NULL, seps));
    next_memory =atoi(strtok(NULL, seps));
    next_required_time =atoi(strtok(NULL, seps));



    while(1){

        //record through_put
        int temp=time;
        if((temp-1)%60==0&&temp!=1){
            through_put[interval]=total_process-total_process_remain-completed_pross;
            interval+=1;
            completed_pross=total_process-total_process_remain;

        }


        //check whether need to add the process and add if required
        while(time==next_arrival){

            processes[process_count].first_arrival_time=next_arrival;
            processes[process_count].last_arrival_time=next_arrival;
            processes[process_count].id=next_id;
            processes[process_count].required_memory=next_memory;
            processes[process_count].required_time=next_required_time;

            //unlimited memory
            if (memory_options==1){
                processes[process_count].used_memory=next_memory;
            }
                //Swapping-x and virtual memory
            else{
                processes[process_count].used_memory=0;
            }

            processes[process_count].remaining_time=next_required_time;
            process_count+=1;


            //swap according to arrival time and ID -- round robin and FCFS
            if (schedule!=3){
                qsort(processes, process_count, sizeof(struct process), cmp_struct);
            }

            //Shortest job first, sort the queue according to their job time
            // the processing job will not be sorted since it is SJB instead of SRT
            else{
                sort_by_time(processes,process_count);

            }


            if(fgets(buf,MAX_LINE,fp)!=NULL){
                next_arrival = atoi(strtok(buf, seps));
                next_id= atoi(strtok(NULL, seps));
                next_memory =atoi(strtok(NULL, seps));
                next_required_time =atoi(strtok(NULL, seps));
            }
            else{
                next_arrival=-1;
                break;
            }

        }



        //If there is no process running
        if(process_count==0){
            time+=1;
        }
        //If there is process running, however the process is loading the memory
        else if (load_time!=0){
            load_time-=1;
            time+=1;
        }

        //Check whether the running process is finished. Deal with the finished process
        else if (processes[0].remaining_time==0){
            process_count-=1;
            total_process_remain-=1;
            int evicted[MAX_pages];

            if(memory_options!=1){
                int used_pages=processes[0].used_memory/4;
                for (int i=0;i<used_pages;i++){
                    evicted[i]=processes[0].pages[i];
                    memory_array[processes[0].pages[i]]=-1;
                    ID_array[processes[0].pages[i]]=-1;
                    memory+=4;
                }
                print_evicted(time,evicted,used_pages);

            }

            fprintf(stdout,"%.0f, FINISHED, id=%d, proc-remaining=%d\n", time,processes[0].id,process_count);
            current_running=-1;
            int temp_v=time-processes[0].first_arrival_time;
            turnaround+=temp_v;
            time_overhead[total_process-total_process_remain-1]=(time-processes[0].first_arrival_time)/processes[0].required_time;

            struct_array_pop(processes, 0, process_count + 1);
            quantum=round_robin;



            //end if all the process has been completed
            if (total_process_remain==0){

                //print throughput
                through_put[interval]=total_process-completed_pross;
                interval+=1;
                cal_throughput(through_put,interval);
                cal_turnaround(turnaround,total_process);

                cal_time_overhead(time_overhead,total_process);
                fprintf(stdout,"Makespan %0.f\n",time);

                return 0;

            }

        }

        //round-robin, if the quantum is elapsed, reset the quantum and put the process to the end of the queue
        else if (quantum==0){
            quantum=round_robin;
            struct process temp_process=processes[0];
            temp_process.last_arrival_time=time+0.1;
            struct_array_pop(processes, 0, process_count);
            processes[process_count-1]=temp_process;
            current_running=-1;




        }


        //the logical for executing the process
        else{

            //the process is already in the memory, direct execute the process
            if(processes[0].used_memory==processes[0].required_memory){
                if(quantum==round_robin&&processes[0].id!=current_running){
                    print_running(processes,init_memory,memory,time,load_time,memory_options);
                }


                processes[0].remaining_time-=1;
                quantum-=1;
                time+=1;

            }

            // the process is not fully in the memory, but virtual_memory is used
            else if(processes[0].used_memory>=MIN_MEMORY&&memory_options==3){

                // There is still empty pages in the memory, allocate them to the running process
                if (memory>=4&&processes[0].used_memory!=processes[0].required_memory){
                    load_time+=allocate_memory(processes,total_pages,memory_array,ID_array,&memory,time);
                    processes[0].required_memory-=load_time/2;
                    print_running(processes,init_memory,memory,time,load_time,memory_options);
                    current_running=processes[0].id;
                }

                else{
                processes[0].remaining_time-=1;
                quantum-=1;
                time+=1;
                }
            }


            //memory is not enough, allocate the memory first
            else{
                load_time=0;
                //there are still memory left, allocate these memory first
                if (memory>=4){
                    load_time+=allocate_memory(processes,total_pages,memory_array,ID_array,&memory,time);
                }

                //the process is not fully in the memory, however it is VR, thus execute the process if the used memory>16kb,


                //if the memory is still not enough
                //swapping-x
                if(memory_options==2){
                    int counter= 0;
                    int evicted[MAX_pages];
                    int evicted_count=0;

                    while(processes[0].used_memory!=processes[0].required_memory){

                        for(int j=0;j<processes[counter].used_memory/4;j++){
                            //release memory
                            evicted[evicted_count]=processes[counter].pages[j];
                            evicted_count+=1;

                            memory_array[processes[counter].pages[j]]=-1;
                            ID_array[processes[counter].pages[j]]=-1;
                            processes[counter].pages[j]=-1;
                            memory+=4;
                        }
                        processes[counter].used_memory=0;

                        //if there are already enough memory left;
                        if(memory>=(processes[0].required_memory-processes[0].used_memory)){
                            qsort(evicted,evicted_count, sizeof(int),cmp_int);

                            print_evicted(time,evicted,evicted_count);

                            load_time+=allocate_memory(processes,total_pages,memory_array,ID_array,&memory,time);
                            break;
                        }
                        counter+=1;
                    }

                }



                // virtual memory
                else if(memory_options==3){

                    int counter= 1;
                    int evicted[MAX_pages];
                    int evicted_count=0;

                    //if the memory allocated is already enough
                    if(processes[0].used_memory>=MIN_MEMORY){
                        int page_fault=(processes[0].required_memory-processes[0].used_memory)/4;
                        processes[0].remaining_time+=page_fault;

                    }


                    //if it is not enough
                    while(processes[0].used_memory<MIN_MEMORY&&processes[0].used_memory!=processes[0].required_memory){

                        if(processes[counter].used_memory>0){
                            evicted[evicted_count]=processes[counter].pages[0];
                            evicted_count+=1;

                            memory_array[processes[counter].pages[0]]=-1;
                            ID_array[processes[counter].pages[0]]=-1;
                            processes[counter].pages[0]=-1;

                            array_pop(processes[counter].pages, 0,processes[counter].used_memory/4);

                            memory+=4;
                            processes[counter].used_memory-=4;
                        }

                        //if there are already enough memory left;
                        if(memory>=(MIN_MEMORY-processes[0].used_memory)){
                            qsort(evicted,evicted_count, sizeof(int),cmp_int);

                            print_evicted(time,evicted,evicted_count);

                            load_time+=allocate_memory(processes,total_pages,memory_array,ID_array,&memory,time);
                            int page_fault=(processes[0].required_memory-processes[0].used_memory)/4;
                            processes[0].remaining_time+=page_fault;

                            break;
                        }

                        //Move to the next process
                        if(processes[counter].used_memory==0){
                            counter+=1;
                        }

                    }

                }

                //Customised memory management--Least Recent Used memory management
               else if(memory_options==4){
                    int evicted[MAX_pages];
                    int evicted_count=0;

                   while(processes[0].used_memory!=processes[0].required_memory){
                        int min_index;
                        float min_time=9999;
                       for(int i=0;i<total_pages;i++){
                           //find the least recently used pages
                           if (memory_array[i]!=-1 &&memory_array[i]<min_time){
                                    min_index=i;
                                    min_time=memory_array[min_index];
                           }
                       }

                       for(int i=0;i<process_count;i++){
                           if (processes[i].id==ID_array[min_index]){
                               for(int j=0;j<processes[i].used_memory/4;j++){
                                   if(processes[i].pages[j]==min_index){
                                       evicted[evicted_count]=processes[i].pages[j];
                                       evicted_count+=1;

                                       memory_array[min_index]=-1;
                                       ID_array[min_index]=-1;
                                       processes[i].pages[j]=-1;

                                       array_pop(processes[i].pages, j,processes[i].used_memory/4);
                                       memory+=4;
                                       processes[i].used_memory-=4;
                                       break;

                                   }
                               }
                           }
                       }
                       if(memory>=(processes[0].required_memory-processes[0].used_memory)){
                           qsort(evicted,evicted_count, sizeof(int),cmp_int);

                           print_evicted(time,evicted,evicted_count);

                           load_time+=allocate_memory(processes,total_pages,memory_array,ID_array,&memory,time);
                           break;
                       }





                   }







                }


                print_running(processes,init_memory,memory,time,load_time,memory_options);
                current_running=processes[0].id;


            }



        }












    }


}