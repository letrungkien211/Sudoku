/*Project: Sudoku
  Description: Solving any sudoku puzzle in approximately 0(s)
  Author: Le Trung Kien
  Date: 01/03/2012*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define SIZE 9
#define AVAILABLE 1
#define TRUE 1
#define FALSE 0
#define EMPTY -1

/* Problem*/
int sudoku[SIZE][SIZE];  
int problem[SIZE][SIZE];

/* Availble state representing*/
int column[SIZE][SIZE];
int rows[SIZE][SIZE];
int block[SIZE/3][SIZE/3][SIZE];
/*column[col][val]=AVAILABLE means "val" can be put into column "col"
  rows[row][val]=AVAILABLE means "val" can be put into row "row"
  block[row/3][col/3][val]=AVAILABLE means "val" can be put into the block 
      that includes (row,col) mass.*/

/* Variables*/
int n_ans=0;   // number of answers
FILE *fp;

/*Functions*/
void get_sudoku(FILE* fp);    // get sudoku from a file
void print_table(int table[][SIZE],FILE*);   // print a table
void copy_table(int to[][SIZE],int from[][SIZE]);    // copy two tables
void save_table(int table[][SIZE],FILE*);
void init();   // initialization
void put(int k);    // recursively put a number into sudoku table
void update(int row,int col,int val);    // place "val" into (row,col) and update available state 
void remove_update(int row,int col,int val);    // remove "val" from (row,col) and reverse last update

/****************MAIN************/
int main(){
  clock_t start=clock(),end;
  int i;
  init();   // initialization
  printf("The puzzle:\n");
  print_table(sudoku,stdout); // print the sudoku puzzle
  put(0);   // start putting from k=0
  if(n_ans==0)
    printf("There is no solution.\n");
  else if(n_ans==1)
    printf("There is one solution.\n");
  else
    printf("Thre are %d solutions.\n",n_ans);
  end=clock();  
  printf("Time elapsed: %e(s)\n",(double)(end-start)/CLOCKS_PER_SEC);
  return 0;
}
/****************MAIN************/

/* Recursively put a number into sudoku table*/
void put(int k){
  int val,col,row;
  
  row=k/9;  // row number
  col=k%9;  // column number
  /* If a number is originally placed in (rol,cow) mass
     ---> no number can be put there*/
  if(sudoku[row][col]!=EMPTY){
    if(k<SIZE*SIZE-1) // If this is not the last
      put(k+1);       // put a number to the next mass.
    else{             // Otherwise, a solution is found.
      ++n_ans;        // Increment number of answers
      printf("#%d solution:\n",n_ans);
      print_table(problem,stdout); // print the solution
      //save_table(problem,stdout);  // save the solution 
    }
  }
  else{
    for(val=0; val<SIZE; ++val){
      /* If "val" can be put into (row,col) mass*/
      if(column[col][val]==AVAILABLE &&
	 rows[row][val]==AVAILABLE &&
	 block[row/3][col/3][val]==AVAILABLE){
	update(row,col,val);
	if(k<SIZE*SIZE-1){
	  put(k+1);
	}
	else{
	  ++n_ans;
	  printf("#%d solution:\n",n_ans);
	  print_table(problem,stdout);
	  //save_table(problem,stdout);  // save the solution 
	}
	remove_update(row,col,val);    // remove "val" from (row,col) mass
      }
    }
  }
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
    printf("\n");
  }
  printf("\n");
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
void init(){
  int row,col,val;
  char line[100],filename[100];
  printf("Input a file name.\n");
  fgets(line,sizeof(line),stdin);
  sscanf(line,"%s",filename);
  fp=fopen(filename,"r");
  if(!fp){
    printf("File not found.\n");
    exit(1);
  }
  get_sudoku(fp);
  fclose(fp);
  copy_table(problem,sudoku);
  
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
  for(row=0; row<SIZE/3; ++row){
    for(col=0; col<SIZE/3; ++col){     
      for(val=0; val<SIZE; ++val){
	block[row][col][val]=AVAILABLE;
      }
    }
  }
  
  /* Initialize available state of sudoku puzzles!*/
  for(row=0; row<SIZE; ++row){
    for(col=0; col<SIZE; ++col){
      if((val=sudoku[row][col])>=0){
	column[col][val]=!AVAILABLE;
	rows[row][val]=!AVAILABLE;
	block[row/3][col/3][val]=!AVAILABLE;
      }
    }
  }
}

/* Put a new number into sudoku table and update correspondent available state*/
void update(int row,int col, int val){
  problem[row][col]=val;    // value update
  column[col][val]=!AVAILABLE;  // column status update
  rows[row][val]=!AVAILABLE;   // rows status update
  block[row/3][col/3][val]=!AVAILABLE;   // block status update
  
}

/* Remove a new number from sudoku table and update correspondent available state*/
void remove_update(int row,int col, int val){
  problem[row][col]=EMPTY;
  column[col][val]=AVAILABLE;
  rows[row][val]=AVAILABLE;
  block[row/3][col/3][val]=AVAILABLE;
}

void save_table(int table[][SIZE],FILE *fp){
  int row,col;
  for(row=0; row<SIZE; ++row){
    for(col=0; col<SIZE; ++col){
      if(table[row][col]>=0)
	fprintf(fp,"%d",table[row][col]+1);
      else
	fprintf(fp,"0");
    }
    printf("\n");
  }
  printf("\n");
}
