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

#define SIZE 9         // Sudoku table size
#define AVAILABLE 1  
#define TRUE 1      
#define FALSE 0 
#define EMPTY -1       // Empty grid
#define S_TIMES 1000     // Simulation times.
#define LIMIT_TIME 20   // Maximum execution time.
#define MAX_SAMPLE 9
#define MIN 49


/* Problem*/ 
int sudoku[SIZE][SIZE];    // Original sudoku puzzle
/* Variables used to find solutions*/ 
int sudoku_modified[SIZE][SIZE];   // modified sudoku puzzle
int problem[SIZE][SIZE];       // a copy of modified sudoku puzzle used to find solutions
int row_index[SIZE];           // index of a row of modified puzzle
int positive[SIZE];            // number of non-empty grids in a row of modified puzzle
int result[SIZE][SIZE];        // the final puzzle created (best one)

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
int max_empty;   // biggest number of empty grids
FILE *fp; 
 
/*Functions*/ 
/* In-Out functions and Initializing functions*/ 
void get_sudoku(FILE* fp);    // get sudoku from a file 
void sudoku_to_problem();     // modify sudoku puzzle
void problem_to_sudoku(void); // reverse modified puzzle
void print_table(int table[][SIZE],FILE* fp);   // print a table 
void copy_table(int to[][SIZE],int from[][SIZE]);    // copy two tables 
int check_conflict(); // check confliction in input puzzle
void initialize();   // initialization 
 
/* Finding solutions functions*/ 
int put(int k);    // recursively put a number into sudoku table 
void update(int row,int col,int val);    // place "val" into (row,col) and update available state  
void remove_update(int row,int col,int val);    // remove "val" from (row,col) and reverse last update 
void find_solution();   // find solutions

/* Create new puzzle functions*/ 
void create();  // create sudoku puzzle
void generate(int n_empty); // generate more empty grids from a puzzle with n_empty number of empty grids

/****************MAIN************/ 
int main(int argc, char **argv){ 
  clock_t start,end; 
  int row;
  char line[100],filename[100];   // input file name
  
  // Seed random numbers
  srand(time(NULL));
  start=clock();
  /* Input process*/
  // If a filename is not input from command line
  if(argc<2){
    printf("Input a file name.\n");
    fgets(line,sizeof(line),stdin);
    sscanf(line,"%s",filename);
  }
  // If it is
  else
    strcpy(filename,argv[1]);
  
  // Open the file
  fp=fopen(filename,"r");
  if(!fp){
    printf("File not found.\n");
    exit(1);
  }
  
  // Get the sudoku
  get_sudoku(fp); 
  fclose(fp);
  // Print out the given solution
  printf("The given solution:\n"); 
  print_table(sudoku,stdout);
  // Check if there is any conflict in the input puzzle.
  if(check_conflict())
    return 0;
 
  // Start with max_empty=MIN
  max_empty=MIN;

  // Create a new puzzle
  create();
 
  // Show the execution time
  end=clock();
  printf("Time elapsed: %e(s)\n",(double)(end-start)/CLOCKS_PER_SEC);
  return 0; 
} 

/****************MAIN************/ 

/*finding solutions functions*/
// Terminate as soon as n_ans>1
void find_solution(){ 
  n_ans=0;  
  initialize();   // initialize 
  put(0);         // recursively put numbers to modified table
} 

// return the number of empty grids in a table
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
  // tmp[SIZE][SIZE][1] is a copy of given solution
  // tmp[SIZE][SIZE][0] is an empty table
  int tmp[SIZE][SIZE][2];
  int index_cnt; 
  int n_empty;    // number of empty grids
  int s_cnt;

  clock_t start=clock();

  // Initialize array tmp[][][]
  for(i=0; i<SIZE; ++i){ 
    for(j=0; j<SIZE; ++j){ 
      tmp[i][j][1]=sudoku[i][j]; 
      tmp[i][j][0]=EMPTY; 
    } 
  }
  
  // Simulating S_TIMES times, however, program will be terminated if processing time exceeds limited time
  for(s_cnt=0; s_cnt<S_TIMES && clock()<start+LIMIT_TIME*CLOCKS_PER_SEC; ++s_cnt){
    // Start with n_empty=0
    n_empty=0;
    for(i=0; i<=SIZE/2; ++i){ 
      for(j=0; j<SIZE && !(i==SIZE/2&&j==SIZE/2+1); ++j){ 
	// P(rand_01(m,n)=0)=m/n
	// So expectancy of the total number of empty grids 
	// will be (max_empty+1)
	k=rand_01(max_empty+1,SIZE*SIZE);
	
	// Add 1 or 2 to n_empty if k=0
	if(i!=SIZE/2||j!=SIZE/2) 
	  n_empty+=2-2*k;
	else 
	  n_empty+=1-k;    
	// Assign new empty grid(s)
	sudoku[i][j]=tmp[i][j][k]; 
	sudoku[SIZE-1-i][SIZE-1-j]=tmp[SIZE-1-i][SIZE-1-j][k]; 
      } 
    }
    // In practice, the chance to have n_empty>=54 at this stage
    // is minuscule, that is why those cases are skiped to reduce 
    // processing quantity. Only n_empty>=MIN is considered also make the program
    // faster while still get a fairly big number of empty grids in the end.
    if(n_empty>=MIN && n_empty<54){ 
      find_solution(); 
      if(n_ans==1){
	generate(empty(sudoku));    // Generate more empty grids
      } 
    } 
  }

  // Return sudoku table to its original state
  // This is not necessary in this particular program
  // but a good thing for further extention of program
  for(i=0; i<SIZE; ++i){ 
    for(j=0; j<SIZE; ++j){ 
      sudoku[i][j]=tmp[i][j][1];
    } 
  }

  // Print out the final result
  printf("Puzzle with number of empty grids is %d.\n", max_empty);
  print_table(result,stdout);
  
} 

// Generate more empty grids of puzzles found in simulating process
// Their number of empty grids >= limit_empty-norm
// So the odds of finding a puzzle with limit_empty number of empty grids
// recursively by removing numbers from them are fairly high.
void generate(int n_empty){
  int i,j,k; 
  int row,col; 
  int generate_tmp[SIZE][SIZE][2]; 

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
	    if(n_empty+2>max_empty){
	      max_empty=n_empty+2;
	      copy_table(result,sudoku);
	    }
	    generate(n_empty+2);
	  }
	  else {
	    if(n_empty+1>max_empty){
	      max_empty=n_empty+1;
	      copy_table(result,sudoku);
	    }
	    generate(n_empty+1);
	  }
	}
	// reverse lastest change
	k=1;
	sudoku[i][j]=generate_tmp[i][j][k]; 
	sudoku[SIZE-1-i][SIZE-1-j]=generate_tmp[SIZE-1-i][SIZE-1-j][k];
      }
    } 
  }
}

/* Recursively put a number into sudoku table
Backtracking Algorithm*/ 
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
 
/* Convert sudoku puzzle*/ 
// This is 1-1 transformation, so all the rules of original sudoku puzzle
// can be converted to the rules in modified puzzle.
// More specifically:
// RULE 1: A number appears one and only one time in all rows, columns
// ---> remain the same in modified puzzle
// RULE 2: A number appears one and only one time in a block.
// ---> diffent in modified puzzle

/* In original sudoku puzzle:
   There are 9 blocks, we can see that if grid (i,j) and (k,l) are in the same block
   i/3=k/3 and j/3=l/3
   So we represent the available state of all values in
   9 blocks by three dimension array block_original[3][3][9]
   So if grid(i,j)=k then block_original[i/3][j/3][k]=!AVAILABLE

   ---> This rule can be simply change in modified puzzle
   block_original [i/3][j/3][k]=!AVAILABLE means that 
   in #row k, three consecutive grids (k,i1),(k,i1+1),(k,i1+2) are not equal to j.
   in which, i1=i-i%3;
   so block[SIZE][SIZE/3][SIZE/3] is used to represent this rule
 */
void sudoku_to_problem(void){ 
  int row,col,val; 
  int i,j; 
 
  // initialize row_index
  for(row=0; row<SIZE; ++row) 
    row_index[row]=row; 

  // initialize sodoku_modified table
  for(row=0; row<SIZE; ++row){ 
    for(col=0; col<SIZE; ++col){ 
      sudoku_modified[row][col]=EMPTY; 
    } 
  } 
   
  // insert numbers to sudoku_modified table correspondent
  // with numbers and their positions in original sudoku table
  for(row=0; row<SIZE; ++row){ 
    for(col=0; col<SIZE; ++col){ 
      if((val=sudoku[row][col])!=EMPTY){ 
	// #i row of sudoku_modified table
	// represent 9 positions of i in original table.
	// In other word, sudoku_modified[i][j]=k means
	// i in #j row belongs to #k row
	sudoku_modified[val][row]=col;
	++positive[val];  // increment the number of non-empty grids in #val row
      } 
    } 
  }   
   
  /* This the breakpoint that makes the program many times faster*/
  // In sudoku puzzle, set (1,2,...,9) is permutable without loss of generality.
  // In modified sudoku puzzle, the same thing happens.
  // Naturally, the number we start putting to sudoku puzzle is the number that ocurrs most
  // frequently. The number of occurs of X in sudoku puzzle is the number of non-empty grids
  // of #X row in modified sudoku puzzle.
  // So, this step will sort the rows of modified puzzle so that previous row has more empty grids
  // than the next row.
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
/* Convert back the modified puzzle to original puzzle*/
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

void initialize(){ 
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

/* Return 0 or 1
   P(0)=m/n and P(1)=1-m/n;
*/
int rand_01(int m,int n){
  return (rand()%n<m) ? 0 : 1;
}
