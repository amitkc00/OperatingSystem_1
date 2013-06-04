/***   a  .c  file for the producer's code   ***/
#include "mydonuts.h"

int		shmid, semid[3];

int main(int argc, char *argv[])
{
	int	in_ptr [NUMFLAVORS]; //To store position of empty slot in each donut flavor
	int	serial [NUMFLAVORS]; //To hold the count of Donuts in each donut flavor.
	int	i,j,k;
	struct donut_ring *shared_ring;
	struct timeval randtime;
	

	/* producer initializes serial counters and in-pointers  */
	for(i = 0; i < NUMFLAVORS; i++){
		in_ptr [i] = 0;
		serial [i] = 0;
	}


   	void sig_handler ( int );  /* declare signal handler function */
   	sigset_t   mask_sigs;
   	int  nsigs;
   	struct sigaction new_action;
   	int sigs [] = {SIGHUP, SIGINT, SIGQUIT, SIGBUS, SIGTERM, SIGSEGV, SIGFPE};
   	nsigs = sizeof (sigs) / sizeof (int);
   	sigemptyset (&mask_sigs);
   	
	for(i=0; i< nsigs; i++)
       		sigaddset (&mask_sigs, sigs [i]);
   	for(i = 0; i <  nsigs; i++){
      		new_action.sa_handler 	= sig_handler;
      		new_action.sa_mask 	= mask_sigs;
      		new_action.sa_flags 	= 0;
      		if(sigaction (sigs [i], &new_action, NULL) == -1){
        	 	perror("can't set signals: ");
         		exit(1);
      		}
   	}

	/* begin syscall shmget comment for shared memory */

	if((shmid = shmget (MEMKEY, sizeof(struct donut_ring),IPC_CREAT | 0600)) == -1){
		perror("shared get failed: ");
		exit(1);
	}

	/*  begin syscall shmat comment for shared memory */

	if((shared_ring = shmat (shmid, NULL, 0)) == (void *)-1){
		perror("shared attach failed: ");
		sig_handler(-1);
	}



	/* begin syscall semget comment for semaphores */

	for(i=0; i<NUMSEMIDS; i++) {
		if ((semid[i] = semget (SEMKEY+i, NUMFLAVORS, IPC_CREAT | 0600)) == -1){
			perror("semaphore allocation failed: ");
			sig_handler(-1);
		}
	}


	/*  begin syscall gettimeofday comment for time seed */

	gettimeofday (&randtime, (struct timezone *)0);
	unsigned short xsub1[3];
	xsub1[0] = (ushort) randtime.tv_usec;
	xsub1[1] = (ushort)(randtime.tv_usec >> 16);
	xsub1[2] = (ushort)(getpid());



	/* use the semsetall utility to set initial semaphore values */

	if(semsetall (semid [PROD], NUMFLAVORS,  NUMSLOTS) == -1){
                perror("semsetsall failed: ");
                sig_handler(-1);
	}
	if(semsetall (semid [CONSUMER], NUMFLAVORS,  0) == -1){
                perror("semsetsall failed: ");
                sig_handler(-1);
        }
	if(semsetall (semid [OUTPTR], NUMFLAVORS,  1) == -1){
                perror("semsetsall failed: ");
                sig_handler(-1);
        }

	while(1)
	{
		/* use nrand48 with xsub1 to get 32 bit random number */
		j=nrand48(xsub1) & 3;

		if( p(semid[PROD],j) == -1) {
			perror("p Operation Failed");
			sig_handler(-1);
		}

		// Count of available donut of flavor j	
		shared_ring->flavor[j][in_ptr[j]] = serial[j];

		// Points to the next empty slot for flavor j
     		in_ptr[j] = (in_ptr[j] + 1) % NUMSLOTS;

		serial[j]++;

		//Increase donut count for flavor j for consumer group semaphore
		if( v(semid[CONSUMER],j) == -1) {
			perror("v Operation Failed");
			sig_handler(-1);
		}

     		printf("donut type = %d, In ptr =  %d, serial ptr =  %d\n",j, in_ptr[j], serial[j]);
       } 
}

/*  including the producer signal handler below       */
void    sig_handler(int sig)
{
	int i;
	printf("In signal handler with signal # %d\n",sig);
	if(shmctl(shmid, IPC_RMID, 0) == -1){
		perror("handler failed shm RMID: ");
	}
	for(i = 0; i < NUMSEMIDS; i++){
		if(semctl (semid[i], 0, IPC_RMID,0) == -1){
		perror("handler failed sem RMID: ");
		}
	}
	exit(4);
}

