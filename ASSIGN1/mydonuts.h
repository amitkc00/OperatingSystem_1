#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>


#define	SEMKEY			(key_t)549351882
#define	MEMKEY			(key_t)549351882
#define	NUMFLAVORS	 	4
#define	NUMSLOTS       		50
#define	NUMSEMIDS	 	3
#define	PROD		 	0
#define	CONSUMER	 	1
#define	OUTPTR		 	2
#define	NO_OF_DOZENS	 	10

struct	donut_ring{
	int	flavor  [NUMFLAVORS]  [NUMSLOTS];
	int	outptr  [NUMFLAVORS];
};

extern int		p (int, int);
extern int		v (int, int);
