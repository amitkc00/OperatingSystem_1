///***   a  .c  file for the consumer code   ***/

#include "mydonuts.h"
#include <time.h>


int main(int argc, char *argv[])
{
	int i,j,k,x,y;
	struct donut_ring *shared_ring;
	int shmid, semid[3];
	
	if((shmid = shmget(MEMKEY, sizeof(struct donut_ring), 0)) == -1){
		perror("shared get failed: ");
		exit(-1);
	}

	/* Now its Start Attaching shared memory */
	if((shared_ring=shmat(shmid, NULL, 0)) == (void *)-1){
		perror("shared attach failed: ");
		exit(2);
   	}

	for(i=0; i<NUMSEMIDS; i++) {
		if ((semid[i] = semget(SEMKEY+i, NUMFLAVORS, 0)) == -1){
			perror("semaphore allocation failed: ");
			exit(-1);
		}
	}


	/* use microsecond component for uniqueness */
	struct timeval randtime;

	//  begin syscall gettimeofday comment for time seed
	gettimeofday (&randtime, (struct timezone *)0);

	int temp;			
	unsigned short xsub1[3];
       	xsub1[0] = (ushort) randtime.tv_usec;
       	xsub1[1] = (ushort)(randtime.tv_usec >> 16);
       	xsub1[2] = (ushort)(getpid());
	
	for(i=1; i<=NO_OF_DOZENS; i++){

		printf("\n#### Flavor,DonutValue #####");
	
		for(k=0;k<12;k++){

			/* use nrand48 with xsub1 to get 32 bit random number */
       			j=nrand48(xsub1) & 3;
			//printf("\nRandom Value of j = %d \n",j); //comment
	
			if( p(semid[OUTPTR],j) == -1 ) {
				perror("\np operation for OUTPTR failed\n");
				exit(-1);
			}
	
			if( p(semid[CONSUMER],j) == -1) {
				perror("\np operation for CONSUMER failed \n");
				exit(-1);
			}
	
			temp = shared_ring->flavor[j][shared_ring->outptr[j]];
			printf("\n%d,%d",j,temp); //comment
	
			shared_ring->outptr[j]=(((shared_ring->outptr[j])+1) % NUMSLOTS);
			//printf("\nValue of new Outptr is = %d\n",shared_ring->outptr[j]);
	
			if( v(semid[OUTPTR],j) == -1) {
				perror("\nv Operation on OUTPTR failed\n");
				exit(-1);
			}

			if( v(semid[PROD],j) == -1) {
				perror("\nv Operation on PROD failed \n");
				exit(-1);
			}

		}
		usleep(100);
		printf("\n### A DOZEN COLLECTED ###\n");
	}
	if((i-1)==NO_OF_DOZENS)
		printf("\n### LOOP COMPLETED ###");
}
/*
	// Printing to Outputfile.

	printf("Jelly			Coconut			Chocolate		Sugar \n"); 
	for(y=0;y<12;y++)
	{
		for(x=0;x<NUMFLAVORS;x++)
		{
			if(out[x][y] != 0) { 		
				//printf("%3d\t\t\t",out[x][y]);
				printf("%d\t\t\t",out[x][y]);
			}
			
		}
		printf("\n");	
	} 
*/
