
#pragma once

#define ESC "\033"

//Format text
#define RESET 		0
#define BRIGHT 		1
#define DIM			2
#define UNDERSCORE	3
#define BLINK		4
#define REVERSE		5
#define HIDDEN		6

//Foreground Colours (text)

#define F_BLACK 	30
#define F_RED		31
#define F_GREEN		32
#define F_YELLOW	33
#define F_BLUE		34
#define F_MAGENTA 	35
#define F_CYAN		36
#define F_WHITE		37

//Background Colours
#define B_BLACK 	40
#define B_RED		41
#define B_GREEN		42
#define B_YELLOW	44
#define B_BLUE		44
#define B_MAGENTA 	45
#define B_CYAN		46
#define B_WHITE		47


#define home() 			               printf(ESC "[H") //Move cursor to the indicated row, column (origin at 1,1)
#define clrscr()		                  printf(ESC "[2J") //lear the screen, move to (1,1)
#define gotoxy(x,y)		               printf(ESC "[%d;%dH", y, x);
#define visible_cursor()               printf(ESC "[?251");
//Set Display Attribute Mode	<ESC>[{attr1};...;{attrn}m
#define resetcolor()                   printf(ESC "[0m")
#define set_display_atrib(color)       printf(ESC "[%dm",color)
