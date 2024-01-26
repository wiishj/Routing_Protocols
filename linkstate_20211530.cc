#include <stdio.h>
#include <stdlib.h>

int n;
int check[100];
int dst[100][100]; //dst[from][to]=cost
int table[100][100][2]; //table[from][to][0]=parent, table[from][to][1]=cost
int p_table[100][100][2];

FILE *fp_tp, *fp_cg, *fp_out;

void set_table(){
	for(int i=0; i<n; i++){
		for(int j=0; j<n; j++){
			if(i==j){
				table[i][j][0]=i;
				table[i][j][1]=0;
			}
			else if(dst[i][j]>0){
				table[i][j][0]=i;
				table[i][j][1]=dst[i][j];
			}
		}
	}
}

void cmp_dst(int from){

	for(int j=2; j<n; j++){
		int spt_node, tmp=200;
		//find node to insert spt
		for(int i=0; i<n; i++){
			if(table[from][i][1]>0 && tmp>table[from][i][1] && !check[i]) {
				spt_node=i; //tie-breaking rule2
				tmp=table[from][i][1];
			}
		}
		check[spt_node]=1;
		
		for(int i=0; i<n; i++){ //spt_node->i까지 거리계산
			if(check[i] || table[spt_node][i][1]<0) continue;
						
			int cost=table[spt_node][i][1]+table[from][spt_node][1];
			if(table[from][i][1]<0 || cost < table[from][i][1]){
				table[from][i][1]=cost;
				table[from][i][0]=spt_node;
			}
			else if(cost == table[from][i][1]){
				if(table[from][i][0]>spt_node) table[from][i][0]=spt_node; //tie-breaking rule 3
			}
		}

	}
}

void dijkstra(){
	for(int i=0; i<n; i++){ //spt start node is i
		//init check arr
		for(int j=0; j<n; j++) check[j]=0;
		
		check[i]=1;
		cmp_dst(i); 
	}
}

void write_table(){
	for(int i=0; i<n; i++){
		for(int j=0; j<n; j++){
			if(table[i][j][0]<0) {
				p_table[i][j][0]=-999;
				p_table[i][j][1]=-999;
				continue;
			}
			
			int tmp=table[i][j][0];
			if(tmp==i) tmp=j;
			while(1) {
				if(table[i][tmp][0]==i) break;
				tmp=table[i][tmp][0];
			}
			p_table[i][j][0]=tmp;
			p_table[i][j][1]=table[i][j][1];
			fprintf(fp_out, "%d %d %d\n", j, tmp, table[i][j][1]);
			//printf("%d %d %d\n", j, tmp, table[i][j][1]);
		}
		fprintf(fp_out, "\n");
		//printf("\n");
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
		for(i=0; i<1001; i++){
			msg[i]=fgetc(fp_msg);
			if(msg[i]=='\n') break;
		}
		msg[i+1]='\0';

		if(table[from][to][0]<0) fprintf(fp_out, "from %d to %d cost infinite hops unreachable message %s",from, to, msg);
		else{
			fprintf(fp_out, "from %d to %d cost %d hops", from, to, table[from][to][1]);
			
			int next, cur=from;
			while(1){
				next=p_table[cur][to][0];
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
		dijkstra();

		fprintf(fp_out, "\n");
		write_table();
		write_msg(argv);
	}
}
int main(int argc, char* argv[]){
	
	//error handling
	if(argc!=4){
		fprintf(stderr, "usage: linkstate topologyfile messagesfile changesfile\n");
		exit(1);
	}
	
	fp_tp=fopen(argv[1], "r");
	FILE *fp_msg=fopen(argv[2], "r");
	fp_cg=fopen(argv[3], "r");
	fp_out=fopen("output_ls.txt", "w");
	
	if(fp_tp==NULL || fp_msg ==NULL || fp_cg==NULL){
		fprintf(stderr, "Error: open input file.\n");
		exit(1);
	}
	fclose(fp_msg);

	if(fp_out==NULL){
		fprintf(stderr, "Error: open output file.\n");
		exit(1);
	}

	/*step 0*/
	//topology file
	fscanf(fp_tp, "%d", &n);
	//init
	for(int i=0; i<n; i++){
		for(int j=0; j<n; j++){
			dst[i][j]=0;
			table[i][j][0]=-999;
			table[i][j][1]=-999;
		}
	}
	int from, to, cost;
	while(fscanf(fp_tp, "%d %d %d", &from, &to, &cost)!=EOF){
		dst[from][to]=cost;
		dst[to][from]=cost;
	}
	set_table();
	dijkstra();
	/*check
	for(int i=0; i<n; i++){
		for(int j=0; j<n; j++){
			printf("%d %d %d\n",j, table[i][j][0], table[i][j][1]);
		}
		printf("\n");
	}*/
	//writing at output file
	write_table();
	write_msg(argv[2]);
	
	/*step 2*/
	exe_change(argv[2]);

	printf("Complete. Output file written to output_ls.txt.\n");
	fclose(fp_tp);
	fclose(fp_cg);
	fclose(fp_out);
}
