// Shell for Computer "Radio 86RK"
// (c) 12-05-2014 vinxru (aleksey.f.morozov@gmail.com)

// Modified for Palmira by Viktor Pykhonin


#include "shell.h"
#include "n.h"
#include <string.h>
#include "interface.h"
#include "radio86rk/video.h"
#include "radio86rk/bios.h"
#include "radio86rk/radio8screenvars.h"
#include "stdlib.h"
#include <stdio.h>

uchar  activePanel;
uchar* fileCursorAddr;


//----- Palmira redefenitions -----

void initPalmiraScreen() {
  asm {
;    LXI H, 0C001h; VG_75+1
;    MVI M, 0
;    DCR	L
;    MVI M, 04DH
;    MVI M, 064H
;;    MVI M, 0CCH
;;    MVI M, 058H
;    MVI M, 00CH
;    MVI M, 048H
;    INR L
;    MVI M, 27H
;    MOV A, M
;WAIT_VRTC:
;    MOV A, M
;    ANI 20H
;    JZ WAIT_VRTC
;    LXI H, 0C608h; VT_57+8
;    MVI M, 80H
;    MVI L, 4
;    MVI M, 0D0H
;    MVI M, 0B6H
;    INR L
;    MVI	M, 1FH
;    MVI	M, 49H
;    MVI L, 8
;    MVI	M, 0A4H
;
;    VG75[1] = 0; 
;    VG75[0] = 0x4d
;    VG75[0] = 0x64
;    VG75[0] = 0x77
;    VG75[0] = 0x50
;    VG75[1] = 0x23
    mvi  a,50h
    DI
    OUT  98h+1
    mvi  A,80h+1
    OUT  98h+1
    ;EI
  }
}

#define PALMIRA_SCREEN_ECONOMY(MEM_ADDR, FULL_HEIGHT, HEIGHT, TOP_INVISIBLE, FONT, BPL, FILL_EOL, HIDDEN_ATTRIB, CHAR_GEN) \
  register uchar *v; \
  uchar i; \
  memset((uchar*)MEM_ADDR, 0, (HEIGHT)*(BPL)+(TOP_INVISIBLE)*2+2); \
  for(v=(uchar*)(MEM_ADDR)-1, i=TOP_INVISIBLE; i; --i) \
    v+=2, *v = 0xF1; \
  if(FILL_EOL) \
    for(i = HEIGHT; i; --i) \
      v += (BPL), *v = 0xF1; \
  ((uchar*)MEM_ADDR)[(HEIGHT)*(BPL)+(TOP_INVISIBLE)*2+1] = 0xFF; \
  radio86rkVideoMem = (uchar*)(MEM_ADDR) + (TOP_INVISIBLE)*2 + 7; \
  radio86rkVideoBpl = (BPL);


/*#define RADIO86RK_SCREEN_STD(MEM_ADDR, FULL_HEIGHT, HEIGHT, TOP_INVISIBLE, FONT, BPL, FILL_EOL, HIDDEN_ATTRIB, CHAR_GEN) \
  register uchar *v; \
  uchar i; \
  memset((uchar*)(MEM_ADDR), 0, (FULL_HEIGHT)*(BPL)); \
  if(FILL_EOL) { \
    v = (uchar*)(MEM_ADDR)-1; \
    for(i = FULL_HEIGHT; i; --i) \
      { v += radio86rkVideoBpl; *v = 0xF1; } \
  } \
  radio86rkVideoMem = (uchar*)(MEM_ADDR) + (TOP_INVISIBLE)*(BPL) + ((HIDDEN_ATTRIB) ? 9 : 8); \
  radio86rkVideoBpl = (BPL); */


void showTextCursor(uchar x, uchar y)
{
  /*x = x + 1 ? 6 + x : 255;
  y += 4;
  //HARDWARE_CURSOR
  asm {
    LXI H, 0C001H
    MVI M, 80H
    DCR L
    LDA showTextCursor_1
    MOV M, A
    LDA showTextCursor_2
    MOV M, A
  }
  */
  gotoxy(x,y);
}


//---------------------------------------------------------------------------


void drawInit() {
//  RADIO86RK_SCREEN_STD(0xB6D0, 30, 25, 3, 0x77, 78, 0, 0, 0);
  //PALMIRA_SCREEN_ECONOMY(0xB6D0, 37, 31, 4, 0x77, 75, 1, 0, 0);
  clrscr();
  initPalmiraScreen();
  
  maxFiles = /*getMemTop()*/0xC000 - START_FILE_BUFFER;
  maxFiles /= sizeof(FileInfo)*2;
  activePanel = 0;
  fileCursorAddr = 0;  
}


//---------------------------------------------------------------------------

/*void drawInit() {
  INTERFACE_VIDEO_MODE
  maxFiles = getMemTop() - START_FILE_BUFFER;
  maxFiles /= sizeof(FileInfo)*2;
  activePanel = 0;
  fileCursorAddr = 0;  
}*/

//---------------------------------------------------------------------------

/*void showTextCursor(uchar x, uchar y) {
  HARDWARE_CURSOR
}*/

//---------------------------------------------------------------------------

void hideTextCursor() {
  showTextCursor(255, 255);
}

//---------------------------------------------------------------------------

void drawSwapPanels() {
  activePanel = 32-activePanel;
}

void setaddr(uchar* a)
{
  asm
  {
    di
    mov  a,l
    out  98h + 1
    mov  a,h
    ori  40h
    out  98h + 1
    ei
  }
}

void setch(uchar c)
{
  asm
  {
    OUT  98h
  }
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void hLine(uchar* a, uchar c, uchar w);
void vLine(uchar* a, uchar h, uchar c);

// Скрыть курсор
uchar attrHidden = 0xF4;
uchar attrShow = 0x4F;

void hideFileCursor() {
  if(!fileCursorAddr) return;
  //fileCursorAddr[0] = ' ';
  setaddr(fileCursorAddr);
  setch(' ');
  //fileCursorAddr[13] = ' ';
  setaddr(fileCursorAddr+13);
  setch(' ');
  hLine(fileCursorAddr + (int)radio86rkVideoAttrMem,attrHidden, 14);
  fileCursorAddr = 0;
}

//---------------------------------------------------------------------------

void showFileCursor() {
  // Скрыть прошлый курсор
  hideFileCursor();
  
  // Адрес нового курсора в видеопамяти
  fileCursorAddr = charAddr(panelA.cursorX*15 + 2 + activePanel, 2 + panelA.cursorY);
  //fileCursorAddr[0]  = '[';
  setaddr(fileCursorAddr);
  setch('[');
  //fileCursorAddr[13] = ']';
  setaddr(fileCursorAddr + 13);
  setch(']');
  hLine(fileCursorAddr + (int)radio86rkVideoAttrMem, attrShow, 14);
}


//---------------------------------------------------------------------------

void drawRect(uchar* a, uchar w, uchar h) {
  uchar i;
  //a[0] = 0x04; memset(a+1, 0x1C, w); a[w] = 0x10; a += VIDEO_BPL;
  setaddr(a);
  setch(0x4);
  for (i = w-1; i; --i)
    setch(0x1C);
  setch(0x10);
  a += VIDEO_BPL;
  for(i=h; i; --i) {
    setaddr(a);
    setch(0x06);
    setaddr(a+w);
    setch(0x11);
    a += VIDEO_BPL;
    //a[0] = 0x06; a[w] = 0x11; a += VIDEO_BPL;
  }
  setaddr(a);
  setch(0x02);
  for (i = w-1; i; --i)
    setch(0x1C);
  setch(0x01);
  //a[0] = 0x02; memset(a+1, 0x1C, w); a[w] = 0x01;
}

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
char buf[16];

void drawFilesCountInt(ulong* total, uint filesCnt) { 
  uchar v;
  uchar* a;
  a = charAddr(2+activePanel, 4+ROWS_CNT);
  //i2s32((char*)a, total, 10, ' ');
  i2s32(buf, total, 10, ' ');
  print2(a, buf);
  print2(a+11, "bajt w    ");
  if(filesCnt < 1000)
  {
    //i2s((char*)a + 18, filesCnt, 3, ' ');
    i2s(buf, filesCnt, 3, ' ');
    //sprintf(buf, "%d", filesCnt);
    print2(a + 18, buf);
  }
  i2s(buf, filesCnt, 3, ' ');
  print2(a + 18, buf);
  v = filesCnt % 100;
  print2(a+22, (!(v >= 10 && v < 20) && v % 10 == 1) ? "fajle " : "fajlah ");
}

//---------------------------------------------------------------------------

void drawPanel(uchar* a) { 
  drawRect(a, 30, ROWS_CNT + 4);
  //memset(a + (VIDEO_BPL * (2 + ROWS_CNT) + 1), 0x1C, 29); // hLine
  hLine(a + (VIDEO_BPL * (2 + ROWS_CNT) + 1), 0x1C, 29); // hLine
  vLine(a + (VIDEO_BPL + 15), ROWS_CNT+1, 0x1B);
  print2(a + (VIDEO_BPL + 6 ), "imq");
  print2(a + (VIDEO_BPL + 21), "imq");  
}

void fillRect(uchar* a, uchar w, uchar h, uchar c);

//---------------------------------------------------------------------------
// Рисуем весь экран

void drawScreenInt() {
  // Очистка экрана
  fillRect(radio86rkVideoMem, 64, ROWS_CNT+8, ' ');

  // Рисуем подсказку    
  print(1, ROWS_CNT+7, "F1 FREE F2 NEW  F3 VIEW  F4 EDIT F5 COPY F6 REN  F7 DIR  F8 DEL"); 

  // Рисуем панели
  drawPanel(radio86rkVideoMem + 1);
  drawPanel(radio86rkVideoMem + 33);

  // Рисуем заголовки панелей
  drawPanelTitle(1);
  swapPanels();
  drawPanelTitle(0);
  swapPanels();

  // Курсора нет
  fileCursorAddr = 0;
}

//---------------------------------------------------------------------------
// Перерисовываем заголовок панели

void drawPanelTitle(uchar active) {
  char* p; 
  ushort l;
  uchar x;

  // Восстанавливаем рамку
  //memset((char*)charAddr(activePanel + 2, 0), 0x1C, 29);
  hLine((uchar*)charAddr(activePanel + 2, 0), 0x1C, 29);

  // Выводим путь по центру
  p = panelA.path1;
  if(p[0]==0) p = "/";
  l = strlen(p);
  if(l>=27) p=p+(l-27), l=27;
  x = 2 + (30 - l) / 2 + activePanel;
  printn(x, 0, l, p);
  if(!active) return;
  print(x-1, 0, "[");
  print(x+l, 0, "]");
}

//---------------------------------------------------------------------------

void drawFile2(uchar* a, FileInfo* f) {
  print2n(a, 8, f->fname); 
  setaddr(a+8);
  setch((f->fattrib & 0x80) ? '*' : ((f->fattrib & 0x06) ? 0x7F : ' '));
  //a[8] = (f->fattrib&0x80) ? '*' : ((f->fattrib & 0x06) ? 0x7F : ' ');
  print2n(a + 9, 3, f->fname + 8); 
}

//---------------------------------------------------------------------------

void drawColumn(uchar i) {
  uchar* a;
  uchar y;
  register ushort n = panelA.offset+i*ROWS_CNT;
  FileInfo* f = panelA.files1 + n;  

  a = charAddr(i*15+3+activePanel,2);
  for(y=ROWS_CNT; y; --y, a+=VIDEO_BPL) {
    if(n>=panelA.cnt) {
      print2cn(a, 12, ' ');
      continue;
    }
    drawFile2(a, f);
    ++f; ++n; 
  }
}

//---------------------------------------------------------------------------

void drawFile(uchar x, uchar y, FileInfo* f) {
  drawFile2(charAddr(x*15+3+activePanel, 2+y), f);
}

//---------------------------------------------------------------------------
// Вывод информации о файле под курсором. Имя.

void drawFileInfo1(char* buf) {
  printm(3+activePanel, ROWS_CNT+3, 10, buf);
}

//---------------------------------------------------------------------------

void drawFileInfoDir() {
  drawFileInfo1("     <DIR>");
}

//---------------------------------------------------------------------------
// Вывод информации о файле под курсором. Размер и дата.

void drawFileInfo2(char* buf) {
  printm(14+activePanel, ROWS_CNT+3, 16, buf);
}

//---------------------------------------------------------------------------
// Рисуем командную строку и путь в ней

void drawCmdLineWithPath() {
  register ushort o, l;// , old;

  print(1, ROWS_CNT+6, getPalmiraModeStr());

  // Выводим только последние 30 символов пути
  print(6, ROWS_CNT+6, "/");
  l = strlen(panelA.path1);
  if(l>=25) o=l-25, l=25; else o=0;
  printn(7, ROWS_CNT+6, l, panelA.path1+o);
  print(l+7, ROWS_CNT+6, ">");

  // Сохраняем значение для drawCmdLine
  panelA.cmdLineOff = l+8;

  // Выводим ком строку
  drawCmdLine();
}

//---------------------------------------------------------------------------
// Рисуем окно

void drawWindow(const char* title) { 
  //register uchar i; 

  hideTextCursor();
  
  fillRect(charAddr(WINDOW_X-5,WINDOW_Y-3), 49, 11, ' ');
  drawRect(charAddr(WINDOW_X-4,WINDOW_Y-2), 46, 7);

  print((64-strlen(title)) >> 1, WINDOW_Y-2, title);
}

//---------------------------------------------------------------------------
// Рисуем кнопку ANY KEY в окне

void drawAnyKeyButton() {
  print(WINDOW_X+14, WINDOW_Y+4, "[ ANY KEY ]");
}

//---------------------------------------------------------------------------

void drawEscButton() {
  print(WINDOW_X+12, WINDOW_Y+4, "[ ar2 - stop ]");
}

//---------------------------------------------------------------------------
// Рисуем текст в окне

void drawWindowText(uchar ox, uchar oy, const char* text) {
  printn(WINDOW_X+ox, WINDOW_Y+oy, 40-ox, text);
}

//---------------------------------------------------------------------------
// Рисуем текст в окне

void drawWindowProgress(uchar ox, uchar oy, uchar n, char chr) {
  if(n==0) return;
  printcn(WINDOW_X+ox, WINDOW_Y+oy, n, chr);
}

//---------------------------------------------------------------------------

void drawCmdLine() {
  drawInput(panelA.cmdLineOff, ROWS_CNT+6, 62-panelA.cmdLineOff);
}

//---------------------------------------------------------------------------
// Рисуем поле ввода

void drawInput(uchar x, uchar y, uchar max) {
  register ushort /*c,*/ c1/*, old*/;
  uint cmdline_offset;
  uint cmdline_pos;
 
  cmdline_pos = strlen(cmdline);
  --max;
  if(cmdline_pos < max) cmdline_offset = 0; else cmdline_offset = cmdline_pos-max;
  c1 = cmdline_pos - cmdline_offset;
  cmdline[cmdline_pos] = 0;
  ++c1;
  if(c1 > max) c1 = max;
  ++c1;
  ++max;
  printm(x, y, max, cmdline + cmdline_offset);

  // Устаналиваем аппаратный курсор
  showTextCursor(x+cmdline_pos-cmdline_offset, y);
}

//---------------------------------------------------------------------------

void drawWindowInput(uchar x, uchar y, uchar max) {
  drawInput(WINDOW_X+x, WINDOW_Y+y, max);
}

//---------------------------------------------------------------------------

char getch1() {
  return getch();
}

//---------------------------------------------------------------------------

char bioskey1() {
	return bioskey();
}

void print(uchar x, uchar y,const char* text)
{
	print2(charAddr(x, y), text);
}

void printm(uchar x, uchar y, uchar len,const char* text)
{
	print2m(charAddr(x, y), len, text);
}

void i2s(char* buf, uint v, uint n, uchar spc)
{
	buf += n;
	*buf = 0;
	while (1) {
    uchar mod = v % 10;
		v /= 10;
		--buf;
		*buf = "0123456789"[mod];
		--n;
		if (n == 0) return;
		if (v == 0) break;
	}
	while (1) {
		--buf;
		*buf = spc;
		if (--n == 0) break;
	}
}

uchar* charAddr(uchar x, uchar y) {
	return radio86rkVideoMem + y * radio86rkVideoBpl + x;
}

//void div32_16(ulong* a, ushort b) {
	//uint tmp;

	//((ushort*)a)[1] = ((ushort*)a)[1] / b;

	//((uchar*)&tmp)[1] = op_div16_mod;
	//((uchar*)&tmp)[0] = ((uchar*)a)[1];
	//((uchar*)a)[1] = tmp / b;

	//((uchar*)&tmp)[1] = op_div16_mod;
	//((uchar*)&tmp)[0] = ((uchar*)a)[0];
	//((uchar*)a)[0] = tmp / b;
//  *a /= b;
//}

void i2s32(char* buf, ulong* src, uint n, uchar spc) {
	ulong v = *src;
  ulong v1;
	//set32(&v, src);

	buf += n;
	*buf = 0;

	while (1) {
    v1 = v;
		//div32_16(&v, 10);
    v /= (short)10;
		--buf;
		*buf = "0123456789ABCDEF"[/*op_div16_mod*/v1 % (short)10];
		if (--n == 0) return;
		if (((ushort*)&v)[0] == 0 && ((ushort*)&v)[1] == 0) break;
	}

	while (1) {
		--buf;
		*buf = spc;
		if (--n == 0) break;
	}
}


//char cmp32_32(ulong* a, ulong* b)
//{
//  if (*a == *b)
//    return 0;
//  return (*a > *b) ? 1 : -1;
//}

//void sub32_32(ulong* a, ulong* b)
//{
//  *a -= *b;
//}

//void sub32_16(ulong* a, ushort b)
//{
//	*a -= b;
//}

void printn(uchar x, uchar y, uchar len, const char* text)
{
	print2n(charAddr(x, y), len, text);
}

void printcn(uchar x, uchar y, uchar len, char c)
{
	print2cn(charAddr(x, y), len, c);
}

void gotoxy(char x, char y) {
	putch(0x1B);
	putch('Y');
	putch(y + 0x20);
	putch(x + 0x20);
}
