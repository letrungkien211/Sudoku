/*Project: Sudoku Creator
  Description: Create Sudoku Puzzles satifying the following:
  * It has one and only one solution (given)
  * It has exactly or more empty grids than a given number.
  * It is point symmetric with its centre is symmetric centre. 
  
  Author: Le Trung Kien
  Date: 01/04/2012*/

#include<stdio.h> 
#include<stdlib.h> 
#include<time.h> 
#include<string.h>

#define SIZE 9 
#define AVAILABLE 1 
#define TRUE 1 
#define FALSE 0 
#define EMPTY -1 
#define S_TIME 100000   // Simulation time
#define LIMIT_EMPTY 58
#define MAX_SAMPLE 9

/* Problem*/ 
int sudoku[SIZE][SIZE]; 
/* Variables used to find solutions*/ 
int sudoku_modified[SIZE][SIZE];   // modified sudoku puzzles
int problem[SIZE][SIZE]; 
int row_index[SIZE]; 
int positive[SIZE]; 
int result[SIZE][SIZE]; 
int result_flag=0;
/* Availble state representing*/ 
// Cosider the modified table.
int column[SIZE][SIZE]; 
int rows[SIZE][SIZE]; 
int block[SIZE][SIZE/3][SIZE/3]; 
 
/*column[col][val]=AVAILABLE means "val" can be put into column "col" 
  rows[row][val]=AVAILABLE means "val" can be put into row "row" 
  block[row][col/3][val/3]=AVAILABLE means "val" can be put into the block[row][col/3]*/ 
 
/* Variables*/ 
int n_ans;   // number of answers 
int max_empty;
int limit_empty;
int norm;
FILE *fp; 
 
/*Functions*/ 
/* In-Out functions and Initializing functions*/ 
void get_sudoku(FILE* fp);    // get sudoku from a file 
void sudoku_to_problem(); 
void problem_to_sudoku(void); 
void print_table(int table[][SIZE],FILE* fp);   // print a table 
void copy_table(int to[][SIZE],int from[][SIZE]);    // copy two tables 
int check_conflict(); // check confliction in input puzzle
void init();   // initialization 
 
/* Finding solutions functions*/ 
int put(int k);    // recursively put a number into sudoku table 
void update(int row,int col,int val);    // place "val" into (row,col) and update available state  
void remove_update(int row,int col,int val);    // remove "val" from (row,col) and reverse last update 
void find_solution(); 

/* Create new puzzle functions*/ 
void create();  // create sudoku puzzle
void generate(int n_empty); 
void save_result(int table[][SIZE]);

/****************MAIN************/ 
int main(int argc, char **argv){ 
  clock_t start,end; 
  int row;
  char line[100],filename[100];
  int s_time;
  srand(time(NULL));
  start=clock();
  
  /* get the puzzle*/
  if(argc<2){
    printf("Input a file name.\n");
    fgets(line,sizeof(line),stdin);
    sscanf(line,"%s",filename);
  }
  else
    strcpy(filename,argv[1]);
    
  fp=fopen(filename,"r");
  if(!fp){
    printf("File not found.\n");
    exit(1);
  }
  get_sudoku(fp); 
  fclose(fp); 
  printf("The given solution:\n"); 
  print_table(sudoku,stdout); // print the sudoku puzzle
  
  /* Check if there is any conflict in the input puzzle.*/
  if(check_conflict())
    return 0;
  
  if(argc>2){
    limit_empty=atoi(argv[2]);
  }
  else{
    printf("Please Enter number of empty grids you want.\n(MAXIMUM=%d)\n",LIMIT_EMPTY);
    fgets(line,sizeof(line),stdin);
    limit_empty=atoi(line);
  }
  
  while(limit_empty>LIMIT_EMPTY){
    printf("Too large number.Input again!\n");
    fgets(line,sizeof(line),stdin);
    limit_empty=atoi(line);
  }
 
  if(limit_empty>=55){
    norm=limit_empty-50;
  }
  else
    norm=4;
  max_empty=limit_empty-norm;

  if(limit_empty>=55){
    if(limit_empty==58){
      printf("Please wait. Depend on the puzzle, this process may take up to several ten minutes.\n");
      printf("Press Crl+C to terminate the process\n");
    }
    else
      printf("Please wait a minute or less.\n");
  }
  
  create();
  if(max_empty>=limit_empty){
    printf("SUCCESS.\n");
    printf("\nThe sudoku puzzle.\nNumber of empty grids=%d\n",max_empty);
    print_table(result,stdout);
    printf("Result is saved in result.txt.\n");
  }
  else
    printf("FAILURE.\n");
  
  if(max_empty==LIMIT_EMPTY){
    system("cat result.txt best.txt>new_best.txt");
    system("mv new_best.txt best.txt");
  }
  
  end=clock();
  printf("Time elapsed: %e(s)\n",(double)(end-start)/CLOCKS_PER_SEC);
  return 0; 
} 

/****************MAIN************/ 

/*finding solutions functions*/
void find_solution(){ 
  n_ans=0; 
  init(); 
  put(0); 
} 
// number of empty grids in sudoku table
int empty(int table[][SIZE]){ 
  int i,j,cnt; 
  cnt=0; 
  for(i=0; i<SIZE; ++i){ 
    for(j=0; j<SIZE; ++j){ 
      if(table[i][j]==EMPTY) 
	++cnt; 
    } 
  } 
  return cnt; 
} 
 
/* Create a puzzle with as many empty grids as possible
   by randomly assigning empty grids' positions.*/ 
void create(){ 
  int i,j,k;
  int tmp[SIZE][SIZE][2]; 
  int index_cnt; 
  int n_empty; 
  int s_time;

  //printf("Current biggest number of empty grids.\n");
  for(i=0; i<SIZE; ++i){ 
    for(j=0; j<SIZE; ++j){ 
      tmp[i][j][1]=sudoku[i][j]; 
      tmp[i][j][0]=EMPTY; 
    } 
  }
  for(s_time=0; s_time<S_TIME; ++s_time){
    n_empty=0;
    for(i=0; i<=SIZE/2; ++i){ 
      for(j=0; j<SIZE && !(i==SIZE/2&&j==SIZE/2+1); ++j){ 
	// P(rand_01(m,n)=0)=m/n
	// So expectancy of the number of empty grids 
	// will be max_empty+1
	k=rand_01(max_empty+1,SIZE*SIZE);
	if(i!=SIZE/2||j!=SIZE/2) 
	  n_empty+=2-2*k; 
	else 
	  n_empty+=1-k; 
	sudoku[i][j]=tmp[i][j][k]; 
	sudoku[SIZE-1-i][SIZE-1-j]=tmp[SIZE-1-i][SIZE-1-j][k]; 
      } 
    }
    if(n_empty>=limit_empty-norm && n_empty<=limit_empty && n_empty<54){ 
      find_solution(); 
      if(n_ans==1){
	if(n_empty>max_empty){
	  max_empty=n_empty;
	}
	if(max_empty>=limit_empty){
	  save_result(sudoku);
	  return;
	}
	generate(empty(sudoku));
      } 
    } 
  }

  //printf("DONE\n");  
  for(i=0; i<SIZE; ++i){ 
    for(j=0; j<SIZE; ++j){ 
      sudoku[i][j]=tmp[i][j][1];
    } 
  }
} 

// Generate more empty grids of puzzles found in simulating process
// Their number of empty grids >= limit_empty-norm
// So the odds of finding a puzzle with limit_empty number of empty grids
// recursively by removing numbers from them are fairly high.
void generate(int n_empty){
  int i,j,k; 
  int row,col; 
  int generate_tmp[SIZE][SIZE][2]; 

  if(max_empty>=limit_empty){
    save_result(sudoku);
    return;
  }

  for(i=0; i<SIZE; ++i){ 
    for(j=0; j<SIZE; ++j){ 
      generate_tmp[i][j][1]=sudoku[i][j]; 
      generate_tmp[i][j][0]=EMPTY; 
    } 
  }
  
  for(i=0; i<=SIZE/2; ++i){ 
    for(j=0; j<SIZE && !(i==SIZE/2&&j==SIZE/2+1); ++j){ 
      if(generate_tmp[i][j][1]!=EMPTY){
	// remove numbers from (i,j) and (SIZE-1-i,SIZE-i-j)
	k=0;
	sudoku[i][j]=generate_tmp[i][j][k]; 
	sudoku[SIZE-1-i][SIZE-1-j]=generate_tmp[SIZE-1-i][SIZE-1-j][k]; 	  
	find_solution(); 
	if(n_ans==1){  
	  // Recursively generate more
	  if(i!=SIZE/2||j!=SIZE/2){
	    if(n_empty+2>max_empty)
	      max_empty=n_empty+2;
	    generate(n_empty+2);
	  }
	  else {
	    if(n_empty+1>max_empty)
	      max_empty=n_empty+1;
	    generate(n_empty+1);
	  }
	  if(max_empty>=limit_empty){
	    save_result(sudoku);
	    return;
	  }
	}
	k=1;
	sudoku[i][j]=generate_tmp[i][j][k]; 
	sudoku[SIZE-1-i][SIZE-1-j]=generate_tmp[SIZE-1-i][SIZE-1-j][k];
      }
    } 
  }
}

/* Recursively put a number into sudoku table*/ 
int put(int k){ 
  int val,col,row; 
  if(n_ans>1) 
    return; 
  row=k/9;  // row number 
  col=k%9;  // column number 
  /* If a number is originally placed in (rol,cow) grid 
     ---> no number can be put there*/ 
  if(sudoku_modified[row][col]!=EMPTY){ 
    if(k<SIZE*SIZE-1) // If this is not the last 
      put(k+1);       // put a number to the next grid. 
    else{             // Otherwise, a solution is found. 
      ++n_ans;        // Increment number of answers 
    } 
  } 
  else{ 
    for(val=0; val<SIZE; ++val){ 
      /* If "val" can be put into (row,col) grid*/ 
      if(column[col][val]==AVAILABLE && 
	 rows[row][val]==AVAILABLE && 
	 block[row][col/3][val/3]==AVAILABLE){ 
	update(row,col,val); 
	if(k<SIZE*SIZE-1){ 
	  put(k+1); 
	} 
	else{ 
	  ++n_ans; 
	} 
	remove_update(row,col,val);    // remove "val" from (row,col) grid 
      } 
    } 
  } 
  return n_ans; 
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
 
  // initialize row_index
  for(row=0; row<SIZE; ++row) 
    row_index[row]=row; 

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
  static int sudoku_tmp[SIZE][SIZE],problem_tmp[SIZE][SIZE]; 
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
 


/* Initialization*/ 
int check_conflict(){
  int row,col,val;
  sudoku_to_problem();
  copy_table(problem,sudoku_modified);
  /* Initialy, any value can be put into any positions*/ 
  /* Rows initialization*/ 
  for(row=0; row<SIZE; ++row){ 
    for(val=0; val<SIZE; ++val){ 
      rows[row][val]=AVAILABLE; 
    } 
  } 
  /* Column initialization*/ 
  for(col=0; col<SIZE; ++col){      
    for(val=0; val<SIZE; ++val){ 
      column[col][val]=AVAILABLE; 
    } 
  } 
  /* Block initialization*/ 
  for(row=0; row<SIZE; ++row){ 
    for(col=0; col<SIZE/3; ++col){      
      for(val=0; val<SIZE/3; ++val){ 
	block[row][col][val]=AVAILABLE; 
      } 
    } 
  } 
   
  /* Initialize available state of sudoku puzzles!
     If there is a conflict, exit program*/
  for(row=0; row<SIZE; ++row){
    for(col=0; col<SIZE; ++col){
      val=sudoku_modified[row][col];
      if(val<0){
	printf("The input puzzle has an empty grid (%d,%d)\n",col+1,val+1);
	exit(1);
      }	  
      if(column[col][val]==!AVAILABLE||
	 rows[row][val]==!AVAILABLE||
	 block[row][col/3][val/3]==!AVAILABLE){
	fprintf(stderr,"The input solution has a conflict.\n");
	fprintf(stderr,"%d can't be in (%d,%d) grid.\n",row_index[row]+1,col+1,val+1);
	return 1;
      }
      column[col][val]=!AVAILABLE;
      rows[row][val]=!AVAILABLE;
      block[row][col/3][val/3]=!AVAILABLE;
    }
  }
  /* Initialize available state of sudoku puzzles!*/ 
  for(row=0; row<SIZE; ++row){ 
    for(col=0; col<SIZE; ++col){ 
      if((val=sudoku_modified[row][col])>=0){ 
	column[col][val]=AVAILABLE; 
	rows[row][val]=AVAILABLE; 
	block[row][col/3][val/3]=AVAILABLE; 
      } 
    } 
  } 
  return 0;
}

void init(){ 
  int row,col,val; 
   
  sudoku_to_problem(); 
  copy_table(problem,sudoku_modified); 
   
  /* Initialy, any value can be put into any positions*/ 
  /* Rows initialization*/ 
  for(row=0; row<SIZE; ++row){ 
    for(val=0; val<SIZE; ++val){ 
      rows[row][val]=AVAILABLE; 
    } 
  } 
  /* Column initialization*/ 
  for(col=0; col<SIZE; ++col){      
    for(val=0; val<SIZE; ++val){ 
      column[col][val]=AVAILABLE; 
    } 
  } 
  /* Block initialization*/ 
  for(row=0; row<SIZE; ++row){ 
    for(col=0; col<SIZE/3; ++col){      
      for(val=0; val<SIZE/3; ++val){ 
	block[row][col][val]=AVAILABLE; 
      } 
    } 
  } 
   
  /* Initialize available state of sudoku puzzles!*/ 
  for(row=0; row<SIZE; ++row){ 
    for(col=0; col<SIZE; ++col){ 
      if((val=sudoku_modified[row][col])>=0){ 
	column[col][val]=!AVAILABLE; 
	rows[row][val]=!AVAILABLE; 
	block[row][col/3][val/3]=!AVAILABLE; 
      } 
    } 
  } 
} 

/* Put a new number into sudoku table and update correspondent available state*/ 
void update(int row,int col, int val){ 
  problem[row][col]=val;    // value update 
  column[col][val]=!AVAILABLE;  // column status update 
  rows[row][val]=!AVAILABLE;   // rows status update 
  block[row][col/3][val/3]=!AVAILABLE;   // block status update  
} 
 
/* Remove a number from sudoku table and update correspondent available state*/ 
void remove_update(int row,int col, int val){ 
  problem[row][col]=EMPTY; 
  column[col][val]=AVAILABLE; 
  rows[row][val]=AVAILABLE; 
  block[row][col/3][val/3]=AVAILABLE; 
} 

int rand_01(int m,int n){
  return (rand()%n<m) ? 0 : 1;
}

void save_result(int table[][SIZE]){
  int row,col; 
  
  if(result_flag)
    return;
  result_flag=1;
  copy_table(result,table);
  fp=fopen("result.txt","w");
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
  fclose(fp);
}
