// must compile with -std=c99 -Wall -o checkdiv 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mpi.h>


int main(int argc, char *argv[]){

	unsigned int x, n;
	unsigned int i; //loop index
	FILE * fp; //for creating the output file
	char filename[100]=""; // the file name
	
	float total_time_p2;

	clock_t start_p1, start_p3, end_p1,  end_p3;
	
	//MPI
	int comm_sz;
	int my_rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 


	/////////////////////////////////////////
	// start of part 1

	start_p1 = clock();
	// Check that the input from the user is correct.
	if(argc != 3){
		printf("usage:  ./checkdiv N x\n");
		printf("N: the upper bound of the range [2,N]\n");
		printf("x: divisor\n");
		exit(1);
	}  

	n = (unsigned int)atoi(argv[1]); 
	x = (unsigned int)atoi(argv[2]); 	


	// Process 0 must send the x and n to each process.
	// Other processes must, after receiving the variables, calculate their own range.


	end_p1 = clock();
	//end of part 1
	/////////////////////////////////////////


	/////////////////////////////////////////
	//start of part 2
	// The main computation part starts here
	double start_p2 = MPI_Wtime();

	unsigned int each;
	unsigned int last;

	if ((n-2+1) % comm_sz == 0){
		each = (n-2+1) / comm_sz;
		last = each;
	}
	else {
		each = (n-2+1) / comm_sz;
		last = (n-2+1) - (each*(comm_sz-1));
	}

	unsigned int s = my_rank*each + 2;
	unsigned int e;
	if (my_rank != comm_sz-1)
		e = s + each;
	else
		e = s + last;
	
	char * local;
	local = (char *)malloc(n);
	char * global;
	global = (char *)malloc(n);
	
	for (unsigned int j = s-2; j <= e; j+=1){
		if (j%x == 0)
			local[j-s+2] = 1;
		else
			local[j-s+2] = 0;
	}
	
	int counts[comm_sz];
	int dspl[comm_sz];
	int currbuf = e+1-s;
	for (unsigned int j = 0; j < comm_sz; j++){
		if (j != comm_sz-1) {
			counts[j] = each+1;
		}
		else {
			counts[j] = last+1;
		}
		dspl[j] = j*each;
	}
	//printf("%d %d %d\n", my_rank, counts[my_rank], dspl[my_rank]);
	
	//for (unsigned int j = 0; j <= each; j++)
		//printf("%d %d\n", j+s-2, local[j]);
	
	double end_p2 = MPI_Wtime();
	float time_p2 = end_p2 - start_p2;
	MPI_Reduce(&time_p2, &total_time_p2, 1, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Gatherv(local, currbuf, MPI_CHAR, global, counts, dspl, MPI_CHAR, 0, MPI_COMM_WORLD);
	
	// end of the main compuation part
	//end of part 2
	/////////////////////////////////////////


	/////////////////////////////////////////
	//start of part 3
	//forming the filename
	if(my_rank == 0 || my_rank > comm_sz){
		start_p3 = clock();
		
		strcpy(filename, argv[1]);
		strcat(filename, ".txt");

		if( !(fp = fopen(filename,"w+t")))
		{
		  printf("Cannot create file %s\n", filename);
		  exit(1);
		}
		
	
		for(i = 2; i <= n; i++) {
		  if(global[i] == 1)
			fprintf(fp,"%d\n", i);
		}
		if (n%x == 0)
			fprintf(fp,"%d\n", n);
		fclose(fp);
		
		end_p3 = clock();
	}

	
	//end of part 3
	/////////////////////////////////////////

	/* Print here the times of the three parts as indicated in the lab description, MPI reduce MAX */
	if (my_rank == 0 || my_rank > comm_sz)
		printf("Times of part1: %ld in s\npart2: %e in s\npart3: %ld in s", (end_p1-start_p1)/CLOCKS_PER_SEC, total_time_p2, (end_p3-start_p3)/CLOCKS_PER_SEC);
	
	//free(local);
	//free(global);
	MPI_Finalize();
	return 0;
	}
