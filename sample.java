
public class SudokuSolver implements Constants{
    int sudoku[SIZE][SIZE];
    /* Variables used to find solutions*/
    int sudoku_modified[SIZE][SIZE];
    int problem[SIZE][SIZE];
    int row_index[SIZE];
    int positive[SIZE];
    /* Availble state representing*/
    int column[SIZE][SIZE];
    int rows[SIZE][SIZE];
    int block[SIZE][SIZE/3][SIZE/3];
    /*column[col][val]=AVAILABLE means "val" can be put into column "col"
      rows[row][val]=AVAILABLE means "val" can be put into row "row"
      block[row/3][col/3][val]=AVAILABLE means "val" can be put into the block 
      that includes (row,col) mass.*/
    /* Variables*/
    int n_ans;   /* number of answers*/
    FILE *fp;     
    /*Functions*/
    /* In-Out functions and Initializing functions*/
    void get_sudoku(FILE* fp);    /* get sudoku from a file*/
    void sudoku_to_problem();
    void problem_to_sudoku(void);
    void print_table(int table[][SIZE],FILE*);   /* print a table*/
    void copy_table(int to[][SIZE],int from[][SIZE]);    /* copy two tables*/
    void save_table(int table[][SIZE],FILE *fp);
    void init();   /* initialization
		      /* Finding solutions functions*/
    int put(int k);    /* recursively put a number into sudoku table*/
    void update(int row,int col,int val);    /* place "val" into (row,col) and update available state */
    void remove_update(int row,int col,int val);    /* remove "val" from (row,col) and reverse last update*/
    void find_solutions();
    void create(); /* Create new puzzle functions*/
    /****************MAIN************/
    int main(int argc, char **argv){
	clock_t start,end;
	int row;
	char line[100],solution_file_name[100],input_file_name[100];   
	/* name of file in which solutions are written and name of input file*/
	start=clock(); 
	/* get the puzzle from a file*/
	if(argc>1){
	    strcpy(input_file_name,argv[1]);
	}
	else{
	    printf("Input a file name.\n");
	    fgets(line,sizeof(line),stdin);
	    sscanf(line,"%s",input_file_name);
	}
	for(row=0; row<SIZE; ++row)
	    row_index[row]=row;
	fp=fopen(input_file_name,"r");
	if(!fp){
	    printf("File not found.\n");
	    exit(1);
	}
	get_sudoku(fp);
	fclose(fp);
  
	/* print out the sudoku puzzle*/
	printf("The puzzle:\n");
	print_table(sudoku,stdout);

	/* Create a new file to store solutions*/
	strcpy(solution_file_name,input_file_name);
	strcat(solution_file_name,"-solution.txt");
	fp=fopen(solution_file_name,"w");
	if(!fp){
	    printf("Create File Error.\n");
	    exit(1);
	}

	/* Find, print out, and save solutions*/
	find_solutions();  /* find all solutions*/
	fclose(fp);
	/* Check number of answers*/
	if(n_ans==0)
	    printf("There is no solution.\n");
	else if(n_ans==1){
	    printf("There is one solution.\n");
	    printf("Solution is saved in file named %s\n",solution_file_name);
	}
	else{
	    printf("Thre are %d solutions.\n",n_ans);
	    printf("Solutions are saved in file named %s\n",solution_file_name);	
	}
	end=clock();
	printf("Execution time: %e(s)\n",(double)(end-start)/CLOCKS_PER_SEC);
	return 0;
    }
    /****************MAIN************/
    /* Find solutions function*/
    void find_solutions(){
	n_ans=0;
	init();   /* initialization*/
	put(0);   /* recursively place numbers*/
    }
    /* Initialization*/
    void init(){
	int row,col,val; 
	sudoku_to_problem();
	copy_table(problem,sudoku_modified);
	/* Initialize available state of sudoku puzzles!
	   If there is a conflict, exit program*/
	for(row=0; row<SIZE; ++row){
	    for(col=0; col<SIZE; ++col){
		if((val=sudoku_modified[row][col])>=0){
		    if(column[col][val]==!AVAILABLE||
		       rows[row][val]==!AVAILABLE||
		       block[row][col/3][val/3]==!AVAILABLE){
			fprintf(stderr,"The input problem has a conflict.\n");
			fprintf(stderr,"%d can't be in (%d,%d) grid.\n",row_index[row]+1,col+1,val+1);
			exit(1);
		    }
		    column[col][val]=!AVAILABLE;
		    rows[row][val]=!AVAILABLE;
		    block[row][col/3][val/3]=!AVAILABLE;
		}
	    }
	} 
    }
    /* Recursively put a number into sudoku table*/
    int put(int k){
	int val,col,row;
	row=k/9;  /* row number*/
	col=k%9;  /* column number*/
	/* If a number is originally placed in (rol,cow) mass
	   ---> no number can be put there*/
	if(sudoku_modified[row][col]!=EMPTY){
	    if(k<SIZE*SIZE-1) /* If this is not the last*/
		put(k+1);       /* put a number to the next mass.*/
	    else{             /* Otherwise, a solution is found.*/
		++n_ans;        /* Increment number of answers*/
		printf("#%d solution:\n",n_ans);
		problem_to_sudoku(); /* print the solution*/
	    }
	}

	else{
	    for(val=0; val<SIZE; ++val){
		/* If "val" can be put into (row,col) mass*/
		if(column[col][val]==AVAILABLE &&
		   rows[row][val]==AVAILABLE &&
		   block[row][col/3][val/3]==AVAILABLE){
		    update(row,col,val);
		    if(k<SIZE*SIZE-1){
			put(k+1);
		    }
		    else{
			++n_ans;
			printf("#%d solution:\n",n_ans);
			problem_to_sudoku();
		    }
		    remove_update(row,col,val);    /* remove "val" from (row,col) mass*/
		}
	    }
	}
	return n_ans;
    }
    /* Get sudoku puzzle from a file*/
    void get_sudoku(FILE *fp){
	char line[100],str[SIZE];
	int row,col;
	for(row=0; row<SIZE; ++row){
	    fgets(line,sizeof(line),fp);
	    sscanf(line,"%s",str);
	    for(col=0; col<SIZE; ++col){
		sudoku[row][col]=str[col]-'1';
	    }
	}
    }
    /* Swap two integers*/
    void swap_int(int *x, int *y){
	int tmp=*x;
	*x=*y;
	*y=tmp;
    }
    /* Swap two rows of a table*/
    void swap_row(int arr[][SIZE],int m, int n){
	int i;
	for(i=0; i<SIZE; ++i)
	    swap_int(&arr[m][i],&arr[n][i]);
    }
    /* Conver sudoku puzzle*/
    void sudoku_to_problem(void){
	int row,col,val;
	int i,j;

	for(row=0; row<SIZE; ++row){
	    for(col=0; col<SIZE; ++col){
		sudoku_modified[row][col]=EMPTY;
	    }
	}
  
	for(row=0; row<SIZE; ++row){
	    for(col=0; col<SIZE; ++col){
		if((val=sudoku[row][col])!=EMPTY){
		    sudoku_modified[val][row]=col;
		    ++positive[val];
		}
	    }
	}  
  
	for(i=0; i<SIZE-1; ++i){
	    for(j=i+1;j<SIZE; ++j){
		if(positive[j]>positive[i]){
		    swap_row(sudoku_modified,i,j);
		    swap_int(&row_index[i],&row_index[j]);
		}
	    }
	}
    }
    /* Problem to sudoku*/
    void problem_to_sudoku(){
	static sudoku_tmp[SIZE][SIZE],problem_tmp[SIZE][SIZE];
	int row,col,val;
  
	for(row=0; row<SIZE; ++row){
	    for(col=0; col<SIZE; ++col){
		problem_tmp[row_index[row]][col]=problem[row][col];
	    }
	}
  
	for(row=0; row<SIZE; ++row){
	    for(col=0; col<SIZE; ++col){
		sudoku_tmp[col][problem_tmp[row][col]]=row;
	    }
	}
	print_table(sudoku_tmp,stdout);
	save_table(sudoku_tmp,fp);
    }
    /* Print a table*/
    void print_table(int table[][SIZE],FILE *fp){
	int row,col;
	for(row=0; row<SIZE; ++row){
	    for(col=0; col<SIZE; ++col){
		if(table[row][col]>=0)
		    fprintf(fp,"%d ",table[row][col]+1);
		else
		    fprintf(fp,"* ");
	    }
	    fprintf(fp,"\n");
	}
	fprintf(fp,"\n");
    }
    /* Copy two tables*/
    void copy_table(int to[][SIZE],int from[][SIZE]){
	int row,col;
	for(row=0; row<SIZE; ++row){
	    for(col=0; col<SIZE; ++col){
		to[row][col]=from[row][col];
	    }
	}
    }
    /* Save table*/
    void save_table(int table[][SIZE],FILE *fp){
	int row,col;
	for(row=0; row<SIZE; ++row){
	    for(col=0; col<SIZE; ++col){
		if(table[row][col]>=0)
		    fprintf(fp,"%d",table[row][col]+1);
		else
		    fprintf(fp,"0");
	    }
	    fprintf(fp,"\n");
	}
	fprintf(fp,"\n");
    }
    /* Put a new number into sudoku table and update correspondent available state*/
    void update(int row,int col, int val){
	problem[row][col]=val;    /* value update*/
	column[col][val]=!AVAILABLE;  /* column status update*/
	rows[row][val]=!AVAILABLE;   /* rows status update*/
	block[row][col/3][val/3]=!AVAILABLE;   /* block status update*/ 
    }

    /* Remove a new number from sudoku table and update correspondent available state*/
    void remove_update(int row,int col, int val){
	problem[row][col]=EMPTY;
	column[col][val]=AVAILABLE;
	rows[row][val]=AVAILABLE;
	block[row][col/3][val/3]=AVAILABLE;
    }
    /* Number of empty grids in a puzzle*/
    int empty(){
	int i,j,cnt;
	cnt=0;
	for(i=0; i<SIZE; ++i){
	    for(j=0; j<SIZE; ++j){
		if(sudoku[i][j]==EMPTY)
		    ++cnt;
	    }
	}
	return cnt;
    }
