// Programmer: Maharshi Shah    September 23, 2018
// Operating Systems COP 4600
// Shared Memory - Project 2     
// U13381853                   

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define SHMKEY ((key_t) 2700)

#define SEMKEY ((key_t) 400L)
#define NSEMS 1   

typedef struct {
  int value; 
} shared_mem;


shared_mem *total;                               //shared variable for processes

int sem_id;// semaphore id
//semaphore buffers
static struct sembuf OP = {0,-1,0};
static struct sembuf OV = {0,1,0};

struct sembuf *P =&OP;
struct sembuf *V =&OV;
// semapore union used to generate semaphore

typedef union{
int val;
struct semid_ds *buf;
ushort *array;
} semunion;

//POP (wait()) function for semaphore to protect critical section

int POP()
{
int status;
status = semop(sem_id, P,1);
return status;
}

int VOP()
{
int status;
status = semop(sem_id, V,1);
return status;
}


void process1()
{
    int i = 0;
    while (i < 100000)                          //Increases value of total by 1 100,000 times
    {	
    	POP();
        if(total->value < 1100000)
	{
	total->value = total->value + 1;
	}
	VOP();
	i++;
    }
    printf ("\nFrom Process 1:counter %d\n", total->value);
}

void process2()
{
    int i = 0;
    while (i < 200000)                          //Increases value of total by 1 200,000 times
    {

        POP();
        if(total->value < 1100000) 
	{
	total->value = total->value + 1;
	}
	VOP();
	i++;

    }

    printf ("\nFrom Process 2:counter %d\n", total->value);
}


void process3()
{
    int i = 0;
    while (i < 300000)                          //Increases value of total by 1 300000 times
    {
     	POP();
	if(total->value < 1100000) 
	{
	total->value = total->value + 1;
	}
       	VOP();
        i++;
    }
    printf ("\nFrom Process 3:counter %d\n", total->value);
}

void process4()
{
    int i = 0;
    while (i < 500000)                          //Increases value of total by 1 500,000 times
    {
        
     	POP();
	if(total->value < 1100000) 
	{
	total->value = total->value + 1;
	}
	VOP();
	i++;
    }
    printf ("\nFrom Process 4:counter %d\n", total->value);
}


int main(void)
{
    //required variable declaration
    int shmid,ID, status, pid1, pid2, pid3,pid4;		
    char *shmadd;
    int semnum = 0;
    shmadd = (char *) 0;
    int   value, value1;
    semunion semctl_arg;
    semctl_arg.val = 1;
       
        /* Create semaphores */
	sem_id = semget(SEMKEY, NSEMS, IPC_CREAT | 0666);
	if(sem_id < 0)
	 printf("Error in creating the semaphore./n");
	
	/* Initialize semaphore */
	value1 =semctl(sem_id, semnum, SETVAL, semctl_arg);
	value =semctl(sem_id, semnum, GETVAL, semctl_arg);
	
	if (value < 1) 
	printf("Eror detected in SETVAL.\n");
	

//Shared memory segment connection

    if ((shmid = shmget(SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }

    if ((total = (shared_mem *) shmat(shmid, shmadd, 0)) == (shared_mem*) - 1)
    {
        perror("shmat");
        exit(0);
    }

    total->value = 0;

// First Child creation and initiation
    if ((pid1 = fork()) == 0)                               
    {
        process1();                                         
    }

//Second Child process creation and initiation
    if ((pid1 != 0) && (pid2 = fork()) == 0)               
    {
        process2();                                         
    }

//Third Child process creation and initiation
    if ((pid1 != 0) && (pid2 != 0) && (pid3 = fork()) == 0) 
    {
        process3();        
    }

//Fourth Child process creation and initiation   
    if ((pid1 != 0) && (pid2 != 0) && (pid3 != 0) && (pid4 = fork()) == 0) 
    {
        process4();        
    }

    //Loop until child processes finish
    for(;(ID = wait(&status)) != -1;)                       
    {
        printf("Child with ID: %d has just exited\n", ID);     
    }

    //Detaching shared memory 
    if(shmdt (total) == -1){
        perror("shmdt");
        exit(-1);
    }
    
    //Removing shared memory
    if ((pid1 != 0) && (pid2 != 0) && (pid3 != 0) && (pid4 != 0))
    {
        if ((shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0)) == -1)
        {
            perror("shmctl");
            exit(-1);
        }
    //After all children have finished, print result
        printf("\n\tEnd of Simulation\n");                    
    
	//Deallocate semaphore
	semctl_arg.val = 0;
	status = semctl(sem_id, 0, IPC_RMID, semctl_arg);
	if(status < 0){
	printf("Error in removing the semaphore.\n");	
	}
}

    return 0;
}
