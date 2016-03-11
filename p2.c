#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>


pthread_mutex_t track_mutex;
pthread_cond_t  track_convar;


int numtrains;
pthread_t dispatch;

// Declare train structure 
typedef struct
{
    pthread_t       thread;
    pthread_cond_t  granted;
    int num;
    int loading_time;
    int crossing_time;
    int priority;
    char direction;
    char state;
} train;


// Each train instance has one of these threads.
// Loads, signals dispatcher and waits for approval to cross track.
// When approval is granted, it locks and crosses the track, then unlocks it.

void *load_and_cross(void * voiditem)
{

    train *item = (train*) voiditem;
    item->state = 'L';
    usleep(item->loading_time*100000);
    item->state = 'R';
    printf("Train %d is ready to go %c\n", item->num, item->direction); 
    pthread_mutex_lock(&track_mutex); 
	pthread_cond_signal(&track_convar);

 
    if(item->state != 'T'){
        pthread_cond_wait(&item->granted, &track_mutex);
    }

    item->state = 'T';
 
    printf("Train %d is ON the main track going %c\n",item->num,item->direction);
    usleep(item->crossing_time*100000);
    item->state = 'G'; 
    printf("Train %d is OFF the main track after going %c\n",item->num,item->direction);
    pthread_mutex_unlock(&track_mutex);
    pthread_cond_signal(&track_convar); 

    pthread_exit(0);

}



// function to find which of two trains should be next.
// This is called to compare each train that is ready.

int findnexttrain(train *first, train *second, char last)
{
    if (first->priority - second->priority == 0) {
        

        if(first->direction != second->direction){
            if(first->direction == last){
                return second->num;
            } else
                return first->num;
        } else if(first->direction == second->direction){
            if(first->loading_time == second->loading_time){
                if((first->num) < (second->num)){
                    return first->num;
                } else
                    return second->num;
            } else {
                if (first->loading_time < second->loading_time){
                    return first->num;
                } else
                    return second->num;
            }
        }

    } else if(first->priority == 1){        
        return first->num;
    } else if (second->priority == 1){
        return second->num;
    }
}




// Dispatches the trains that are ready.

void *dispatcher(void * voidthing)
{

    int j = 0;
    int on_track = 0;
	int finished = 0;
    int next = -1;
    int a = 0;
    char lasttrain = 'e';

    train * listt;
    listt = (train*) voidthing;
	while(finished < numtrains){
        pthread_mutex_lock(&track_mutex); 
		pthread_cond_wait(&track_convar, &track_mutex);
        
        finished = 0;
		for (j=0;j<numtrains; j++){
			if(listt[j].state == 'G'){
				finished++;
			} else if (listt[j].state == 'T'){
				on_track = 1;
			} else if (listt[j].state == 'R'){
				if (next == -1) {
                    next = j;
                    
                } else{
                    next = findnexttrain(&listt[next], &listt[j], lasttrain);
                }
                
			}
		    
		}

        pthread_cond_signal(&listt[next].granted);
        pthread_mutex_unlock(&track_mutex);
        lasttrain = listt[next].direction;
        next = -1;
	}


}





int main(int argc, char* argv[]) 
{

    if (argc != 3)
    {
        printf("Error: Incorrect Usage.\n");
        printf("Proper Usage: file.txt numtrains\n");
        exit(1);
    }


    // initializes variables to hold info from file, opens file
    char line [25];
    numtrains = atoi(argv[2]);
    int count = 0;

    pthread_mutex_init(&track_mutex, NULL);
    pthread_cond_init(&track_convar, NULL);

    
    FILE *fp = fopen(argv[1],"rt");
    if (!fp) 
    {
        perror("Could not open input file.");
        exit(1);
    }
	train trainlist[numtrains];

    // Creates dispatcher thread
    if (pthread_create(&dispatch, NULL, dispatcher, &trainlist))
    {
        perror("pthread create failed");
        exit(1);
    }




    // Populates array of train structs and creates separate threads.
    while(count < numtrains)
    {
        fgets(line, 25, fp);
        sscanf (line, "%c:%d,%d",&trainlist[count].direction, &trainlist[count].loading_time, &trainlist[count].crossing_time);
        if (isupper(trainlist[count].direction) != 0)
        {
            trainlist[count].direction = tolower(trainlist[count].direction);
            trainlist[count].priority = 1;
        } else {
            trainlist[count].priority = 0;
        }
        pthread_cond_init(&trainlist[count].granted,NULL);

        trainlist[count].num = count;
        if (pthread_create(&trainlist[count].thread, NULL, load_and_cross, &trainlist[count]))
        {
            perror("pthread create failed");
            exit(1);
        }
        count ++;
    }





   fclose(fp);  /* close the file prior to exiting the routine */




    int x =0;
    for (x = 0; x < numtrains; x++) {
        pthread_join(trainlist[x].thread, NULL);
    }

  


}
