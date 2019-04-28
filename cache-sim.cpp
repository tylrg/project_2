#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string directMap(vector<long> accesses,int cacheSize);
string setAssociative(vector<long> accesses,int way);

string setAssociativeWriteMiss(vector<long> accesses, vector<int> ls_type_queue, int way);

int main(int argc, char*argv[]){
	if(argc!=3){
		printf("Must specify input and output_file!\n");
		return 1;
	}

	char* input_file=argv[1];
	//char *output_file=argv[2];
	FILE *input= fopen(input_file,"r");


	unsigned long long addr;
	unsigned long long counter=0;
	char behavior[10];
	while(fscanf(input,"%c %llx\n", behavior,&addr) != EOF) {
		if(!strncmp(behavior,"L",2)) {
			//printf("%lld\t->load", addr);
		}else {
			//printf("%lld\t->store", addr);
		}
		counter++;
		//printf("\tcounter: %lld\n",counter);
	}
	fclose(input);


	// long* accesses;
	// int* ls_type_queue;
	// accesses= new long[counter*sizeof(long)];
	// ls_type_queue= new int[counter*sizeof(long)];

	vector <long> accesses;
	vector <int> ls_type_queue;
	//assignment and read
	FILE *length= fopen(input_file,"r");
	int increment=0;
	unsigned long long acc;
	char p[10];
	while(fscanf(length, "%c %llx\n",p,&acc) != EOF) {
		accesses.push_back(acc);
		//accesses[increment]=acc;
		if(!strncmp(p, "L", 2)) {
			//ls_type_queue[increment]=1;
			ls_type_queue.push_back(1);
			//printf("Prediction array at %d set to L\n",increment);
		}else{
			//ls_type_queue[increment]=0;
			ls_type_queue.push_back(0);
			//printf("Prediction array at %d set to S\n",increment);
		}
		increment++;
	}
	fclose(length);
	//FILE *output = fopen(output_file,"w");
	int kb=1024;
	cout<<directMap(accesses,1*kb)<<directMap(accesses,4*kb)<<directMap(accesses,16*kb)<<directMap(accesses,32*kb)<<endl;
	cout<<setAssociative(accesses,2)<<setAssociative(accesses,4)<<setAssociative(accesses,8)<<setAssociative(accesses,16)<<endl;
	cout<<setAssociativeWriteMiss(accesses,ls_type_queue,2)<<setAssociativeWriteMiss(accesses,ls_type_queue,4)<<setAssociativeWriteMiss(accesses,ls_type_queue,8)<<setAssociativeWriteMiss(accesses,ls_type_queue,16)<<endl;


	return 0;
}

string directMap(vector<long> accesses,int cacheSize){
	string output="";
	int hit=0;
	int miss=0;
	long cache [cacheSize/32];
	int entries=cacheSize/32;
	for(int c=0;c<entries;c++){
		cache[c]=0;
	}
	//cout<<cacheSize<<" "<< entries << endl;
	for(unsigned int i=0;i<accesses.size();i++){
		long instruction=accesses[i];
		int tag=instruction>>5;
		int index= tag%entries;
		if(cache[index]==(32*tag)){
			hit++;
		}
		else{
			cache[index]=tag*32;
			miss++;
		}
	}

	int total=hit+miss;
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append(";");
	return output;
}
string setAssociative(vector<long> accesses,int way){
	string output="";
	int hit=0;
	int miss=0;

	int numEntries=(16*1024/32);
	int numSets=numEntries/way;
	long cache [numEntries/way][way];
	int lruValues [numEntries/way][way];
	for(int c=0;c<numEntries/way;c++){
		for(int k=0;k<way;k++){
			cache[c][k]= 0;
			lruValues[c][k]= -1;
		}
	}


	for(unsigned int i=0;i<accesses.size();i++){
		long instruction=accesses[i];
		int tag=instruction>>5;
		int setIndex=tag%numSets;

		int hFlag=0; //determines hit or miss
		for(int w=0;w<way;w++){
			if(cache[setIndex][w]==(32*tag)){
				hit++;
				hFlag=1;
				lruValues[setIndex][w]=0;
			}
		}
		//if miss
		//double check this shit
		if(hFlag==0){
			int fullFlag=1;
			for(int m=0;m<way;m++){
				if(cache[setIndex][m]==0){
					cache[setIndex][m]=tag*32;
					lruValues[setIndex][m]=0;
					fullFlag=0;
					break;
				}
			}

			//if victim selection is needed
			if(fullFlag==1){
				int vIndex=0;
				for(int l=1;l<way;l++){
					if(lruValues[setIndex][l]>lruValues[setIndex][vIndex]){
						vIndex=l;
					}
				}
				cache[setIndex][vIndex]=tag*32;
				lruValues[setIndex][vIndex]=0;
			}
			miss++;
		}

		for(int lr=0;lr<numSets;lr++){
			for(int lc=0;lc<way;lc++){
				if(lruValues[lr][lc]!=-1){lruValues[lr][lc]++;}
			}
		}
	}

	int total=hit+miss;
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append(";");
	return output;
}

string setAssociativeWriteMiss(vector<long> accesses,vector<int> ls_type_queue,int way){
	string output="";
	int hit=0;
	int miss=0;

	int numEntries=(16*1024/32);
	int numSets=numEntries/way;
	long cache [numEntries/way][way];
	int lruValues [numEntries/way][way];
	for(int c=0;c<numEntries/way;c++){
		for(int k=0;k<way;k++){
			cache[c][k]= 0;
			lruValues[c][k]= -1;
		}
	}


	for(unsigned int i=0;i<accesses.size();i++){
		long instruction=accesses[i];
		int tag=instruction>>5;
		int setIndex=tag%numSets;

		int hFlag=0; //determines hit or miss
		for(int w=0;w<way;w++){
			if(cache[setIndex][w]==(32*tag)){
				hit++;
				hFlag=1;
				lruValues[setIndex][w]=0;
			}
		}
		//if miss
		//double check this shit
		if((hFlag==0)&&(ls_type_queue[i]!=0)){
			int fullFlag=1;
			for(int m=0;m<way;m++){
				if(cache[setIndex][m]==0){
					cache[setIndex][m]=tag*32;
					lruValues[setIndex][m]=0;
					fullFlag=0;
					break;
				}
			}

			//if victim selection is needed
			if(fullFlag==1){
				int vIndex=0;
				for(int l=1;l<way;l++){
					if(lruValues[setIndex][l]>lruValues[setIndex][vIndex]){
						vIndex=l;
					}
				}
				cache[setIndex][vIndex]=tag*32;
				lruValues[setIndex][vIndex]=0;
			}
			miss++;
		}
		else if((hFlag==0)&&(ls_type_queue[i]==0)){
			miss++;
		}

		for(int lr=0;lr<numSets;lr++){
			for(int lc=0;lc<way;lc++){
				if(lruValues[lr][lc]!=-1){lruValues[lr][lc]++;}
			}
		}
	}

	int total=hit+miss;
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append(";");
	return output;
}
