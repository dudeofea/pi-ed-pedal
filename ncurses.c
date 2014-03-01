#include "print_array.h"

void print_array(double arr[], double scale, int size){
	clear();
	int row, col;
	getmaxyx(stdscr, row, col);
	if(size < col){
		col = size;
	}
	for(int i = 0; i < col; i++){
		if(arr[i] > 0){
			for(int j = row/2 - (int)arr[i]*scale; j < row/2; j++){
				mvaddch(j,i,'#');
			}
		}else{
			for(int j = row/2 + 1; j < row/2 - (int)arr[i]*scale + 1; j++){
                                mvaddch(j,i,'#');
                        }
		}
		mvaddch(row/2, i, '-');
	}
	refresh();
}

/*
int main(int argc, char* argv[]){
	float array[] = {5.0, -2.0, 10.0, 1.0};
	initscr();
	print_array(array, 2.0, 4);
	getch();
	print_array(array, 3.0, 2);
	getch();
	endwin();
	return 0;
}
*/
