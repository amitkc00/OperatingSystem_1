#include "assign2header.h"

struct donut_ring       shared_ring;
int                     space_count[NUMFLAVORS];
int                     donut_count[NUMFLAVORS];
pthread_mutex_t         prod[NUMFLAVORS];
pthread_mutex_t         cons[NUMFLAVORS];
pthread_cond_t          prod_cond[NUMFLAVORS];
pthread_cond_t          cons_cond[NUMFLAVORS];
pthread_t               thread_id[NUMCONSUMERS+NUMPRODUCERS];
//pthread_t sig_wait_id;


//Producer Block
void* producer(void* arg) {
	
	unsigned short xsub[3];
    struct timeval randtime;

	int donut_flavor; //To Store Donut Flavor
	int donutflavor_count,donutflavor_buffer;

        gettimeofday(&randtime,(struct timezone*) 0);
        xsub[0] = (ushort)randtime.tv_usec;
        xsub[1] = (ushort)(randtime.tv_usec>>16);
        xsub[2] = (ushort)(getpid());


	while (1) {
		donut_flavor = nrand48 ( xsub ) & 3;
		//printf("\nProducer Flavor = %d\n",donut_flavor);
		pthread_mutex_lock ( &prod [donut_flavor] );
		while(space_count[donut_flavor]== 0 ) {
			pthread_cond_wait(&prod_cond[donut_flavor], &prod[donut_flavor] );
		}
		
		shared_ring.serial[donut_flavor] = shared_ring.serial[donut_flavor] + 1;
		donutflavor_count = shared_ring.serial[donut_flavor];
		donutflavor_buffer= shared_ring.in_ptr[donut_flavor];
		//printf("\nDonut Count = %d , Donut Location = %d\n",donutflavor_count,donutflavor_buffer);


		shared_ring.flavor[donut_flavor][donutflavor_buffer] = donutflavor_count;
		space_count[donut_flavor]--;
		shared_ring.in_ptr[donut_flavor] = ((shared_ring.in_ptr[donut_flavor] + 1) % NUMSLOTS);
	
		pthread_mutex_unlock(&prod[donut_flavor]); //Unlocking Producer Mutex.

		pthread_mutex_lock(&cons[donut_flavor]); //Locking Consumer Mutex.
		donut_count[donut_flavor]++;
		pthread_mutex_unlock(&cons[donut_flavor]); //Unlocking Consuer Mutex.

		pthread_cond_signal(&cons_cond[donut_flavor]);	
		//printf("\nTesting1\n");
	}

		//printf("\nTesting2\n");
	printf("\nProducer Thread Done.\n");
	return NULL;
}


//Consumer Block
void* consumer(void* arg) {

	unsigned short  xsub [3];
	struct timeval  randtime;

    int donut_flavor; //To Store Donut Flavor
    int donutflavor_count,donutflavor_buffer;

	gettimeofday(&randtime,(struct timezone*) 0);
	xsub[0] = (ushort)randtime.tv_usec;
	xsub[1] = (ushort)(randtime.tv_usec>>16);
	xsub[2] = (ushort)(getpid());
	
	int ii,jj;
	int i,j,count_dozens;
	int output[12][NUMFLAVORS];


	for(ii=0;ii<NUMDOZENS;ii++) {
		
		//Initialization of output array to 0.
		for(i=0;i<12;i++) {
			for(j=0;j<NUMFLAVORS;j++)
				output[i][j]=0;
		}

		for(jj=0;jj<12;jj++) {
	
			donut_flavor = nrand48 ( xsub ) & 3;
		
			while(donut_count[donut_flavor]==0)
				pthread_cond_wait(&cons_cond[donut_flavor],&cons[donut_flavor]);

			donutflavor_buffer=shared_ring.outptr[donut_flavor];
			donutflavor_count=shared_ring.flavor[donut_flavor][donutflavor_buffer];
		
			//printf("\nDonut Flavor for Consumer = %d\n",donut_flavor);
			//printf("\nDonut Count = %d , Donut Location = %d\n",donutflavor_count,donutflavor_buffer);

			output[jj][donut_flavor]=donutflavor_count;

			shared_ring.outptr[donut_flavor] = ((shared_ring.outptr[donut_flavor]+1) % NUMSLOTS);
			donut_count[donut_flavor]++;
	
			pthread_mutex_unlock(&cons[donut_flavor]); //Unlocking Consumer Mutex.
	
			pthread_mutex_lock(&prod[donut_flavor]); //Locking Producer Mutex.
			space_count[donut_flavor]++;
			pthread_mutex_unlock(&prod[donut_flavor]); //Unloclong Producer Mutex.

			pthread_cond_signal(&prod_cond[donut_flavor]);
		}

		printf("\nDonut Flavor\n");
		printf("Jelly \t\tChoco \t\tCoffee \t\tPlain\n");
		for(i=0;i<12;i++) {
			for(j=0;j<NUMFLAVORS;j++)
				printf("%d\t\t",output[i][j]);	
			printf("\n");
		}
		printf("\nDozen No = %d\n",ii+1);
	}

	printf("\nConsumer Thread Done. \n");
	return NULL;
}

int main() {
	int tot_thread=NUMPRODUCERS+NUMCONSUMERS;
	int rc; //To store return code from pthread_create.
	int i;

	//Assigning Thread IDs.
	for(i=0;i<tot_thread;i++)
		thread_id[i]=i;

	for(i=0;i<NUMFLAVORS;i++) {
		pthread_mutex_init(&prod[i],NULL);
		pthread_mutex_init(&cons[i],NULL );
		pthread_cond_init(&prod_cond[i],NULL);
		pthread_cond_init(&cons_cond[i],NULL);
		shared_ring.outptr[i] = 0;
		shared_ring.in_ptr[i] = 0;
		shared_ring.serial[i] = 0;
		space_count[i] = NUMSLOTS;
		donut_count[i] = 0;
	}

	for(i=0;i<NUMPRODUCERS;i++) {
		if(rc=pthread_create(&thread_id[i],NULL,producer,NULL)) {
			fprintf(stderr,"Error: pthread_create, rc: %d\n", rc);
			return 1;
		}
	}

	for(i=NUMPRODUCERS;i<tot_thread;i++) {
		if(rc=pthread_create(&thread_id[i],NULL,consumer,NULL)) {
			fprintf(stderr,"Error: pthread_create, rc: %d\n", rc);
			return 1;
		}
		//printf("\nNew Consumer Thread Created\n");
	}
	
	//Join Operation for Consumers.	
	for(i=NUMPRODUCERS;i<tot_thread;i++) {
		if(rc=pthread_join(thread_id[i],NULL)) {
			fprintf(stderr,"Error: pthread_join for consumers rc: %d\n", rc);
			return 2;
		}
	}
}
