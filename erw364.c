#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h> 

int main(int argc, char* argv[]) {
	
	long buckets, threads;
	
	//take in command line input
	if(argc != 4){
		printf("needs 4 inputs: ./exe buckets threads filename\n");
		exit(1);
	}

	buckets = strtol(argv[1], NULL, 10); //0->20
	threads = strtol(argv[2], NULL, 10); //0->100
	char *filename = argv[3];
	
	char data[100000];
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL) {
        printf("Error! opening file");
        exit(1);
    }
	
	
    fscanf(fp, "%[^\n]", data); 
    fclose(fp);
	
	//split file into tokens, split by spaces
	char * tok = strtok(data, " ");
	
	//n = total number of tokens
	unsigned int n = atoi(tok);
	float toks[n+2];
    
	//will store results in bin, init with 0s
	unsigned int bin[buckets];
	memset( bin, 0, buckets*sizeof(int) );
	
	unsigned int each = 20 / buckets;
	
	//put all token from file into toks array
	int count = 0;
	while( tok != NULL ) {
		toks[count] = atof(tok);
		count++;
		tok = strtok(NULL, " ");
	}
	
	//start parallel section, threads is from user input
	#pragma omp parallel num_threads(threads)
	{
		//local array with local results, init with 0s
		unsigned int local[threads][buckets];
		memset( local, 0, threads*buckets*sizeof(int) );
		unsigned int id = omp_get_thread_num();
		
		//goes through each token in toks, and sorts them into the correct bin locally
		#pragma omp for 
		for (int i = 1; i <= n; i++) {
			for (int j = 0; j < buckets; j++){ //each bin's maximum is checked again current token to sort it
				if (toks[i] < j*each+each) {
					local[id][j]++;
					break;
				}
			}
		}
		
		//put local results into global bin using atomic
		for (int j = 0; j < buckets; j++){
			for (int k = 0; k < threads; k++){
				#pragma omp atomic
				bin[j] += local[k][j];
			}
		}
	}
	
	//out of parallel section, process 0 prints results to screen
	for (int i = 0; i < buckets; i++){
		printf("bin[%d] = %d\n", i, bin[i]);
	}
	
	return 0;

}
