#ifndef TEXTMODE_H
#define TEXTMODE_H


#include <stdint.h>
#include <ports.h>
#include <string.h>	//memset, memcpy
#include <display.h>
#include <tty.h>

#define COLS 80U
#define ROWS 25U

#define TEXTMODE_TABSIZE	8


#define BLACK 0
#define BLUE 1 		
#define GREEN 2		
#define CYAN 3		
#define RED 4	
#define MAGENTA 5
#define BROWN 6		//   |
#define LIGHTGREY 7 //
#define GREY 8
#define LIGHTBLUE 9
#define LIGHTGREEN 10
#define LIGHTCYAN 11
#define LIGHTRED 12
#define LIGHTMAGENTA 13
#define YELLOW 14
#define WHITE 15

#define GETX (gpos % COLS)
#define GETY ((gpos - GETX) / COLS)

#define CARD_COLOR	1
#define CARD_MONO   0

//#define va_start(ap, X) __builtin_va_start(ap, X)
//#define va_arg(ap, type) __builtin_va_arg(ap, type)
//#define va_end(ap) __builtin_va_end(ap)
extern struct display *textmode_display_new(struct tty *mtty);
void move_cursor(void);
void gotoxy(uint8_t x,uint8_t y);
void puts(char *s);
void putch(char ch);
void set_attribut(uint8_t vogr, uint8_t higr);
uint16_t attribut(uint8_t fore,uint8_t back);
void clearscreen(uint8_t character, uint16_t attr);
uint8_t detect_vid(void);		
void scroll_down(void);
void kprintf(char *format,...);
void TextOut(uint8_t *text, uint8_t xpos, uint8_t ypos);
void TextOutColor(uint8_t *text, uint8_t xpos, uint8_t ypos, uint8_t vogr, uint8_t higr);
uint8_t getypos();
uint8_t getxpos();

#endif
