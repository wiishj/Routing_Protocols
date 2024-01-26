#include <stdio.h>
#include <stdlib.h>

int n;

int dst[100][100]; //dst[from][to]=cost
int table[100][100][2]; //table[from][to][0]=next, table[from][to][1]=dst
FILE *fp_tp, *fp_cg, *fp_out;

void set_table(){
	for(int i=0; i<n; i++){
		//i의 table 채우기
		for(int j=0; j<n; j++){
			if(i==j){ //자기자신
				table[i][j][0]=i;
				table[i][j][1]=0;
			}
			else if(dst[i][j]>0){ //from i to j인 길 존재
				table[i][j][0]=j;
				table[i][j][1]=dst[i][j];
			}

		}
	}
}
int cmp_dst(int from, int to){//direct node간 update 수행
	int cost=table[from][to][1];
	int is_cg=0;
	for(int i=0; i<n; i++){
		if(table[from][i][1]>0){ //from table에 존재하는
			if(table[to][i][1]<0 ||table[to][i][1]>table[from][i][1]+cost){
				table[to][i][1]=table[from][i][1]+cost;
				table[to][i][0]=from;
				is_cg=1;
			}
			else if(table[to][i][1]==table[from][i][1]+cost){
				if(table[to][i][0]>from) {
					table[to][i][0]=from;
					is_cg=1;
				}
			}
		}
	}
	return is_cg;
}

void exchange(){
	while(1){
		int flag=0, is_cg;
		for(int i=0; i<n; i++){
			for(int j=0; j<n; j++){
				if(dst[i][j]>0 && table[i][j][0]==j){//direct node 간에 exchange
					is_cg=cmp_dst(i, j);
					if(is_cg) flag=1; 
				}
			}
		}
		if(!flag) return; //더이상 변화가 없다
	}
}
void write_table(){
	for(int i=0; i<n; i++){
		for(int j=0; j<n; j++){
			if(table[i][j][0]<0) continue;
			fprintf(fp_out, "%d %d %d\n", j, table[i][j][0], table[i][j][1]);
		}
		fprintf(fp_out, "\n");
	}
}

void write_msg(char* argv){
	FILE* fp_msg=fopen(argv, "r");
	while(1){
		int from, to;
		char msg[1001];
		
		//src, dst node와 message 정보 담기
		from=fgetc(fp_msg)-'0';
		if(from+'0'==EOF) break;
		fgetc(fp_msg);
		to=fgetc(fp_msg)-'0';
		fgetc(fp_msg);
		
		
		int i;
		for(i=0; i<1000; i++){
			msg[i]=fgetc(fp_msg);
			if(msg[i]=='\n') break;
		}
		msg[i+1]='\0';

		if(table[from][to][0]<0){
			fprintf(fp_out, "from %d to %d cost infinite hops unreachable message %s",from, to, msg); 
		}
		else{
			fprintf(fp_out, "from %d to %d cost %d hops", from, to, table[from][to][1]);
			int next, cur=from;
			while(1){
				next=table[cur][to][0];
				fprintf(fp_out, " %d", cur);
				if(next==to) break;
				cur=next;
			}
			fprintf(fp_out, " message %s", msg);
		}
		
	}
	fclose(fp_msg);
}

void exe_change(char* argv){
	int from, to, cost;
	while(1){
		if(fscanf(fp_cg, "%d %d %d", &from, &to, &cost)==EOF) break;
		if(cost<0){
			dst[from][to]=0;
			dst[to][from]=0;
		}
		else{
			dst[from][to]=cost;
			dst[to][from]=cost;
		}
	
		//init table
		for(int i=0; i<n; i++){
			for(int j=0; j<n; j++){
				table[i][j][0]=-999;
				table[i][j][1]=-999;
			}
		}

		set_table();
		exchange();
		
		fprintf(fp_out, "\n");
		write_table();
		write_msg(argv);
	}
}

int main(int argc, char* argv[]){

	//error handling
	if(argc!=4) {
		fprintf(stderr, "usage: distvec topologyfile messagesfile changesfile\n");
		exit(1);
	}
	
	fp_tp=fopen(argv[1], "r");
	FILE *fp_msg=fopen(argv[2], "r");
	fp_cg=fopen(argv[3], "r");
	fp_out=fopen("output_dv.txt", "w");

	if(fp_tp==NULL || fp_msg==NULL || fp_cg==NULL ){
		fprintf(stderr, "Error: open input file.\n");
		exit(1);
	}
	fclose(fp_msg);

	if(fp_out==NULL){
		fprintf(stderr, "Error: open output file.\n");
		exit(1);
	}
	
	/* step 0 */
	//topology file
	fscanf(fp_tp, "%d", &n);
	//init
	for(int i=0; i<n; i++){
		for(int j=0; j<n; j++){
			dst[i][j]=0;
			table[i][j][0]=-999; //next node 존재 x
			table[i][j][1]=-999; //next node까지 도달 못함
		}
	}
	int from, to, cost;
	while(fscanf(fp_tp, "%d %d %d", &from, &to, &cost)!=EOF){
		dst[from][to]=cost;
		dst[to][from]=cost;
	}
	set_table();
	exchange();
	
	//writing at output file
	write_table();
	write_msg(argv[2]);
	
	/* step 1 */
	exe_change(argv[2]);
	
	printf("Complete. Output file written to output_dv.txt.\n");

	fclose(fp_tp);
	fclose(fp_cg);
	fclose(fp_out);
	
}
