#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <set>

using namespace std;

//maps to one spot in cache
string directMap(vector<long> accesses,int cacheSize);
//maps to any of n-ways in one set of the cache, uses LRU replacement
string setAssociative(vector<long> accesses,int way);
//maps to any set in the cache where set=ways=number of entries, uses LRU replacement
string setFullyAssociativeLRU(vector<long> accesses);
//maps to any set in the cache where set=ways=number of entries, uses Hot-Cold replacement
string setFullyAssociativeHC(vector<long> accesses);
//maps to n-ways in one set of the cache, does not penalize cache on miss for write instructions, LRU replacement
string setAssociativeWriteMiss(vector<long> accesses, vector<int> ls_type_queue, int way);
//maps to n-ways in one set of the cache, prefetches the next line to update LRU, LRU replacement
string setAssociativeNextLinePrefetch(vector<long> accesses,int way);
//maps to n-ways in one set of the cache, prefetches the next line only on a miss to update LRU, LRU replacement
string setAssociativePrefetchOnMiss(vector<long> accesses,int way);
//maps to n-ways in onse set of the cache, matains a LFU within a set of the cache,
//and updates based upon the least frequently used within a set
string setAssociativeSetLevelLFU(vector<long> accesses,int way);

//yeah it's my main
int main(int argc, char*argv[]){
	//checks for valid arguement count, returns otherwise
	if(argc!=3){
		printf("Must specify input and output_file!\n");
		return -1;
	}

	//Establishing I/O
	char* input_file=argv[1];
	FILE *input= fopen(input_file,"r");
	ofstream outfile(argv[2]);

	//parsing first read
	unsigned long long addr;
	unsigned long long counter=0;
	char behavior[10];
	while(fscanf(input,"%c %llx\n", behavior,&addr) != EOF) {
		if(!strncmp(behavior,"L",2)) {
		}else {
		}
		counter++;
	}
	fclose(input);

	//input vectors for instruction and type
	vector <long> accesses;
	vector <int> ls_type_queue;

	//assignment and read
	FILE *length= fopen(input_file,"r");
	int increment=0;
	unsigned long long acc;
	char p[10];

	//reading in from input and assignment
	while(fscanf(length, "%c %llx\n",p,&acc) != EOF) {
		accesses.push_back(acc);
		//accesses[increment]=acc;
		if(!strncmp(p, "L", 2)) {
			ls_type_queue.push_back(1);
		}else{
			ls_type_queue.push_back(0);
		}
		increment++;
	}
	fclose(length);

	//establishing kilobyte size because apparently it isn't 1000
	int kb=1024;

	//file output and function calls
	outfile<<directMap(accesses,1*kb)<<directMap(accesses,4*kb)<<directMap(accesses,16*kb)<<directMap(accesses,32*kb)<<endl;
	outfile<<setAssociative(accesses,2)<<setAssociative(accesses,4)<<setAssociative(accesses,8)<<setAssociative(accesses,16)<<endl;
	outfile<<setFullyAssociativeLRU(accesses)<<endl;
	outfile<<setFullyAssociativeLRU(accesses)<<endl;//should be HC
	outfile<<setAssociativeWriteMiss(accesses,ls_type_queue,2)<<setAssociativeWriteMiss(accesses,ls_type_queue,4)<<setAssociativeWriteMiss(accesses,ls_type_queue,8)<<setAssociativeWriteMiss(accesses,ls_type_queue,16)<<endl;
	outfile<<setAssociativeNextLinePrefetch(accesses,2)<<setAssociativeNextLinePrefetch(accesses,4)<<setAssociativeNextLinePrefetch(accesses,8)<<setAssociativeNextLinePrefetch(accesses,16)<<endl;
	outfile<<setAssociativePrefetchOnMiss(accesses,2)<<setAssociativePrefetchOnMiss(accesses,4)<<setAssociativePrefetchOnMiss(accesses,8)<<setAssociativePrefetchOnMiss(accesses,16)<<endl;
	outfile<<setAssociativeSetLevelLFU(accesses,2)<<setAssociativeSetLevelLFU(accesses,4)<<setAssociativeSetLevelLFU(accesses,8)<<setAssociativeSetLevelLFU(accesses,16)<<endl;
	//closing output and returning
	outfile.close();
	return 0;
}

string directMap(vector<long> accesses,int cacheSize){
	//default starting variables
	string output="";
	int hit=0;
	int miss=0;
	long cache [cacheSize/32];
	int entries=cacheSize/32;

	//initialization
	for(int c=0;c<entries;c++){
		cache[c]=0;
	}

	//looping through traces and implementing cache
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

	//result gathering and string formatting
	int total=hit+miss;
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append("; ");
	return output;
}
string setAssociative(vector<long> accesses,int way){
	//initial variables and vectors
	string output="";
	int hit=0;
	int miss=0;
	int numEntries=(16*1024/32);
	int numSets=numEntries/way;
	long cache [numEntries/way][way];
	int lruValues [numEntries/way][way];

	//array initialization
	for(int c=0;c<numEntries/way;c++){
		for(int k=0;k<way;k++){
			cache[c][k]= 0;
			lruValues[c][k]= -1;
		}
	}

	//iteration through instructions
	for(unsigned int i=0;i<accesses.size();i++){

		//initial variables for each instruction
		long instruction=accesses[i];
		int tag=instruction>>5;
		int setIndex=tag%numSets;
		int hFlag=0; //1 if hit, else 0

		//hit logic
		for(int w=0;w<way;w++){
			if(cache[setIndex][w]==(32*tag)){
				hit++;
				hFlag=1;
				lruValues[setIndex][w]=0;
			}
		}

		//miss logic
		if(hFlag==0){
			int fullFlag=1;//1 if set is full, else 0
			for(int m=0;m<way;m++){
				//if set is not full, replace first "empty" spot
				if(cache[setIndex][m]==0){
					cache[setIndex][m]=tag*32;
					lruValues[setIndex][m]=0;
					fullFlag=0;
					break;
				}
			}

			//if victim selection is needed, aka set is full
			if(fullFlag==1){
				int vIndex=0;//initial victim index
				for(int l=1;l<way;l++){
					if(lruValues[setIndex][l]>lruValues[setIndex][vIndex]){
						vIndex=l;//if compared lru index is greater, set victim index to compared index
					}
				}
				cache[setIndex][vIndex]=tag*32;
				lruValues[setIndex][vIndex]=0;
			}
			miss++;
		}

		//increment every LRU value not equal to -1, denoting an empty cache spot
		for(int lr=0;lr<numSets;lr++){
			for(int lc=0;lc<way;lc++){
				if(lruValues[lr][lc]!=-1){lruValues[lr][lc]++;}
			}
		}
	}

	//result gathering and string formatting
	int total=hit+miss;
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append("; ");
	return output;
}
string setFullyAssociativeLRU(vector<long> accesses){
	//initial variables and vectors
	string output="";
	int hit=0;
	int miss=0;
	int way=512;//number of entries==ways
	int numEntries=(16*1024/32);
	int numSets=numEntries/way;
	long cache [numEntries/way][way];
	int lruValues [numEntries/way][way];

	//initialization
	for(int c=0;c<numEntries/way;c++){
		for(int k=0;k<way;k++){
			cache[c][k]= 0;
			lruValues[c][k]= -1;
		}
	}

	//iteration for each instruction
	for(unsigned int i=0;i<accesses.size();i++){
		//initialization of variables for each instruction
		long instruction=accesses[i];
		int tag=instruction>>5;
		int setIndex=tag%numSets;

		//hit logic
		int hFlag=0; //determines hit or miss
		for(int w=0;w<way;w++){
			if(cache[setIndex][w]==(32*tag)){
				hit++;
				hFlag=1;
				lruValues[setIndex][w]=0;
			}
		}

		//miss logic
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

		//incremeting each element of LRU that is not equal to -1
		for(int lr=0;lr<numSets;lr++){
			for(int lc=0;lc<way;lc++){
				if(lruValues[lr][lc]!=-1){lruValues[lr][lc]++;}
			}
		}
	}

	//result gathering and string formatting
	int total=hit+miss;
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append("; ");
	return output;
}
/*string setFullyAssociativeHC(vector<long> accesses){
string output="";
int hit=0;
int miss=0;
int way=512;
int numEntries=(16*1024/32);
int numSets=numEntries/way;
long cache [numEntries/way][way];
for(int c=0;c<numEntries/way;c++){
for(int k=0;k<way;k++){
cache[c][k]= 0;
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
//lruValues[setIndex][w]=0;
//HC LOGIC


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
}*/
string setAssociativeWriteMiss(vector<long> accesses,vector<int> ls_type_queue,int way){
	//initial variables and vectors
	string output="";
	int hit=0;
	int miss=0;
	int numEntries=(16*1024/32);
	int numSets=numEntries/way;
	long cache [numEntries/way][way];
	int lruValues [numEntries/way][way];

	//initialization
	for(int c=0;c<numEntries/way;c++){
		for(int k=0;k<way;k++){
			cache[c][k]= 0;
			lruValues[c][k]= -1;
		}
	}


	//iteration for each intstruction
	for(unsigned int i=0;i<accesses.size();i++){
		//initialization variables for each instruction
		long instruction=accesses[i];
		int tag=instruction>>5;
		int setIndex=tag%numSets;

		//hit logic
		int hFlag=0; //1 if hit, 0 else
		for(int w=0;w<way;w++){
			if(cache[setIndex][w]==(32*tag)){
				hit++;
				hFlag=1;
				lruValues[setIndex][w]=0;
			}
		}

		//miss logic
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

			//no penalty for miss on write instruction
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

		//increment every value of the LRU unless it is -1
		for(int lr=0;lr<numSets;lr++){
			for(int lc=0;lc<way;lc++){
				if(lruValues[lr][lc]!=-1){lruValues[lr][lc]++;}
			}
		}
	}

	//result gathering and string formatting
	int total=hit+miss;
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append("; ");
	return output;
}
string setAssociativeNextLinePrefetch(vector<long> accesses,int way){
	//initialization for variables
	string output="";
	int hit=0;
	int total=0;
	int numEntries=(16*1024/32);
	int numSets=numEntries/way;
	long cache [numEntries/way][way];
	int lruValues [numEntries/way][way];

	//initialization
	for(int c=0;c<numEntries/way;c++){
		for(int k=0;k<way;k++){
			cache[c][k]= 0;
			lruValues[c][k]= -1;
		}
	}

	//iteration for each instruction
	for(unsigned int i=0;i<accesses.size();i++){
		//initial variables for each instruction
		long instruction=accesses[i];
		int tag=instruction>>5;
		int setIndex=tag%numSets;
		int hFlag=0; //1 if hit, else 0

		//hit logic
		for(int w=0;w<way;w++){
			if(cache[setIndex][w]==(32*tag)){
				hit++;
				hFlag=1;
				lruValues[setIndex][w]=0;
			}
		}

		//miss logic
		if(hFlag==0){
			int fullFlag=1;//1 if set is full, else 0
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
		}


		//prefetch next line
		tag=instruction/32+1;
		setIndex=tag%numSets;

		hFlag=0; //1 if set is full, else 0
		for(int w=0;w<way;w++){
			if(cache[setIndex][w]==(32*tag)){
				hFlag=1;
				//only reset lru, do not increment hit
				lruValues[setIndex][w]=0;
			}
		}

		//miss logic
		if(hFlag==0){
			int fullFlag=1;//1 if set full, else 0
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
		}

		//increment LRU only if value != -1
		for(int lr=0;lr<numSets;lr++){
			for(int lc=0;lc<way;lc++){
				if(lruValues[lr][lc]!=-1){lruValues[lr][lc]++;}
			}
		}
		total++;
	}


	//result gathering and string formatting
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append("; ");
	return output;
}
string setAssociativePrefetchOnMiss(vector<long> accesses,int way){
	//initial variables
	string output="";
	int hit=0;
	int total=0;
	int numEntries=(16*1024/32);
	int numSets=numEntries/way;
	long cache [numEntries/way][way];
	int lruValues [numEntries/way][way];

	//initialization
	for(int c=0;c<numEntries/way;c++){
		for(int k=0;k<way;k++){
			cache[c][k]= 0;
			lruValues[c][k]= -1;
		}
	}

	//iteration for each instruction
	for(unsigned int i=0;i<accesses.size();i++){
		//intial variables for instruction
		long instruction=accesses[i];
		int tag=instruction>>5;
		int setIndex=tag%numSets;
		int hFlag=0; //1 if hit, else 0

		//hit logic
		for(int w=0;w<way;w++){
			if(cache[setIndex][w]==(32*tag)){
				hit++;
				hFlag=1;
				lruValues[setIndex][w]=0;
			}
		}

		//miss logic
		if(hFlag==0){
			int fullFlag=1; //1 if cache is full, else 0
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

			//prefetch instruction only update on miss
			tag=instruction/32+1;
			setIndex=tag%numSets;

			hFlag=0; //only on miss
			for(int w=0;w<way;w++){
				if(cache[setIndex][w]==(32*tag)){
					hFlag=1;
					lruValues[setIndex][w]=0;
				}
			}

			//miss logic
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
			}
		}



		//update LRU
		for(int lr=0;lr<numSets;lr++){
			for(int lc=0;lc<way;lc++){
				if(lruValues[lr][lc]!=-1){lruValues[lr][lc]++;}
			}
		}
		total++;
	}


	//result gathering and string formatting
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append("; ");
	return output;
}
string setAssociativeSetLevelLFU(vector<long> accesses,int way){
	//inital variables
	string output="";
	int hit=0;
	int miss=0;
	int numEntries=(16*1024/32);
	int numSets=numEntries/way;
	long cache [numEntries/way][way];
 	int lfu [numEntries/way][way];

	//initialization
	for(int c=0;c<numEntries/way;c++){
		for(int k=0;k<way;k++){
			cache[c][k]= 0;
			lfu[c][k]=false;
		}
	}

	//iteration for each instruction
	for(unsigned int i=0;i<accesses.size();i++){
		//inital variables for each instruction
		long instruction=accesses[i];
		int tag=instruction>>5;
		int setIndex=tag%numSets;
		int hFlag=0; //1 if hit, 0 else

		//hit logic
		for(int w=0;w<way;w++){
			if(cache[setIndex][w]==(32*tag)){
				hit++;
				hFlag=1;
				lfu[setIndex][w]++;
			}
		}
		//miss logic
		if(hFlag==0){
			int fullFlag=1;//1 if set is full, else 0
			for(int m=0;m<way;m++){
				if(cache[setIndex][m]==0){
					cache[setIndex][m]=tag*32;
					lfu[setIndex][m]=1;
					fullFlag=0;
					break;
				}
			}

			//evict based on lowest frequency
			if(fullFlag==1){
				int vIndex=0;
				for(int foo=0;foo<way;foo++){
					if(lfu[setIndex][foo]<lfu[setIndex][vIndex]){vIndex=foo;}
				}
				cache[setIndex][vIndex]=tag*32;
				lfu[setIndex][vIndex]=1;
			}
			miss++;
		}
	}

	//result gathering and string formatting
	int total=hit+miss;
	string hitS= to_string(hit);
	string totalS=to_string(total);
	output.append(hitS);
	output.append(",");
	output.append(totalS);
	output.append("; ");
	return output;
}
