#include "print_array.h"

void print_array(double (*arr)[2], int size){
	clear();
	double scale = 0.1;
	int row, col, pos;
	double current_max_index;
	static double max_index = 0;
	double max = 0;
	for (int i = 0; i < size; ++i)
	{
		if (arr[i][0] > max)
		{
			max = arr[i][0];
			current_max_index = i;
		}
	}
	max_index = current_max_index;
	getmaxyx(stdscr, row, col);
	if(size < col){
		col = size;
	}
	pos = row - 3;
	for(int i = 0; i < col; i++){
		if(arr[i] > 0){
			for(int j = pos - (int)arr[i][0]*scale; j < pos; j++){
				mvaddch(j,i,'#');
			}
		}
		mvaddch(pos, i, '-');
	}
	if (max > 1.5)
	{
		mvprintw(row - 1, 0, "Max: %lf, Index: %lf", max, max_index);
		mvaddch(pos + 1, (int)max_index, '^');
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
