#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


int main(int argc, char*argv[]){
	if(argc!=3){
		printf("Must specify input and output_file!\n");
		return 1;
	}
	//arguments [string input_file, string output_file]
	char* input_file=argv[1];
	//char *output_file=argv[2];
	FILE *input= fopen(input_file,"r");

  //length determining,initial read
  unsigned long long addr;
  unsigned long long counter=0;
	char behavior[10];
  while(fscanf(input,"%c %llx\n", behavior,&addr) != EOF) {
    if(!strncmp(behavior,"L",2)) {
      printf("%lld\t->load", addr);
    }else {
      printf("%lld\t->store", addr);
    }
		counter++;
		printf("\tcounter: %lld\n",counter);
  }
	fclose(input);
	long* accesses=malloc(counter*sizeof(long));
	int* ls_type_queue=malloc(counter*sizeof(long));//1=L, 0=S;

	//assignment and read
	FILE *length= fopen(input_file,"r");
	int increment=0;
	unsigned long long acc;
  char p[10];
	while(fscanf(length, "%c %llx\n",p,&acc) != EOF) {
		accesses[increment]=acc;
	  if(!strncmp(p, "L", 2)) {
    	ls_type_queue[increment]=1;
			printf("Prediction array at %d set to L\n",increment);
    }else{
      ls_type_queue[increment]=0;
			printf("Prediction array at %d set to S\n",increment);
    }
		increment++;
  }
	fclose(length);
	//FILE *output = fopen(output_file,"w");



	//direct map cache
	int cacheSizeList[4] = {1000,4000,16000,32000};
	for(int i=0;i<4;i++){
		unsigned long long hit=0;
		int cacheSize=cacheSizeList[i];
		unsigned long long *cache=malloc(cacheSize*sizeof(unsigned long long));
		//for(int k=0;k<cacheSize;k++){printf("%lld\n",cache[k]);}
		for(int c=0;c<counter;c++){
			unsigned long long target=accesses[c]%(32);
			if(cache[target]==accesses[c]){hit++;}
			else{cache[target]=accesses[c];}
		}
		printf("%lld,%lld;",hit,counter);
		if(i!=3){printf(" ");}
		else{printf("\n");}
		free(cache);
	}

	//set-associative cache
	for(int i=1;i<5;i++){
		int way=(int)pow((double)2,(double)i);
		int hcList=malloc(way-1*sizeof(char));
		
	}


	//fully associative cache



	//set associative cache with no penalty on write miss
	for(int i=1;i<5;i++){
		int way=(int)pow((double)2,(double)i);

	}

	//set associative cache with next-line prefetching
	for(int i=1;i<5;i++){
		int way=(int)pow((double)2,(double)i);

	}

	//Prefetch-on-a-miss
	for(int i=1;i<5;i++){
		int way=(int)pow((double)2,(double)i);

	}



	free(accesses);
	free(ls_type_queue);
	return 0;
}
