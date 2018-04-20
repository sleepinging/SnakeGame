/***************************/ 
//
//Coded by jiaxingxueyuan twt 
//
//
/***************************/ 

#include <stdio.h>  
#include <stdlib.h>  
#include <windows.h>  
#include <conio.h>  
#include <ctype.h>
#include "Snake.h"

#define true 1  
#define false 0
#define RED FOREGROUND_RED
#define BLUE FOREGROUND_BLUE
#define GREEN FOREGROUND_GREEN
#define WHITE RED|BLUE|GREEN
#define YELLOW RED|GREEN

int zoomhigh=25;
int zoomwidth=50;
int movespeed=1;
Snake s;
int mutex=1;
int snakeuimux=1;//蛇锁 
int colormux=1;//颜色锁 
int infomux=1;//信息锁 
int foodmux=1;
int posmux=1;
int drawmux=1;
HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取控制台句柄
void replykey(char key);
void drawpoint(const char* p,int x,int y);
void updatesnake(Snake &s);
void showheadinfo(Snake &s);

void keyhandler(INPUT_RECORD keyrec){//处理按键消息 
	int key=0;
	if ((key=keyrec.Event.KeyEvent.wVirtualKeyCode) != 0  
	    && keyrec.Event.KeyEvent.bKeyDown == true){//当按下键时              
	    if (keyrec.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED){//Shift键为按下状态    
	        //printf("Shift+%c",(char)key);  
	    }  
	    if (keyrec.Event.KeyEvent.dwControlKeyState & CAPSLOCK_ON){//大写锁定为打开状态  
	    	replykey(tolower(key));
	        //printf("%c",toupper(key));  
	    }  
	    else{//大写锁定关闭状态    
	    	replykey(tolower(key));
	        //printf("%c",tolower(key));  
	    }  
	}  
}

void listenkey(){//监听按键 
	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);//获得标准输入设备句柄  
    INPUT_RECORD keyrec;//定义输入事件结构体  
    DWORD res;//定义返回记录  
    for (;!isdead;){    
        ReadConsoleInput(handle_in, &keyrec, 1, &res);//读取输入事件  
        if (keyrec.EventType == KEY_EVENT){//如果当前事件是键盘事件        
            keyhandler(keyrec);
        }  
    }  	
}

COORD wherepos(){//获取光标的位置(x,y)     
	int x=0,y=0;
    CONSOLE_SCREEN_BUFFER_INFO pBuffer;  
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &pBuffer);  
    x=pBuffer.dwCursorPosition.X+1; 
	y=pBuffer.dwCursorPosition.Y+1;
	COORD p;
	p.X=x,p.Y=y;
	return p;
} 
void changecolor(WORD color){
	Lock(colormux);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
	Unlock(colormux);
}
void setpos(int x,int y){//设置光标位置 
	Lock(posmux);
    COORD c;  
    c.X=x-1,c.Y=y-1;  
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),c);
    Unlock(posmux);
}   
void updatescore(int score){//更新分数 
	Lock(infomux);
	setpos(zoomwidth+1+1,1);
	printf("\tScore:%d",score);
	Unlock(infomux);
}
void updatelevel(int level){//更新级别 
	Lock(infomux);
	setpos(zoomwidth+1+1,2);
	printf("\tLevel:%d",level);
	Unlock(infomux);
}
void updateinfo(const char* info){//显示信息 
	Lock(infomux);
	setpos(zoomwidth+1+1,7);
	for(int i=0;i<7;++i)
		printf("\t");
	setpos(zoomwidth+1+1,7);
	printf("\tInfo:%s",info);
	Unlock(infomux);
}
void updategamemod(int mod){
	setpos(zoomwidth+1+1,6);
	switch(mod){
		case 1:
			printf("\tGame Model:Normal Game");
			break;
		case 2:
			printf("\tGame Model:Debug");
			break;
		case 3:
			printf("\tGame Model:Auto");
			break;
	}
}
void updatelength(int len){//显示蛇的长度 
	Lock(infomux);
	setpos(zoomwidth+1+1,3);
	printf("\tLength:%d",len);
	Unlock(infomux);
}
void updatefood(){
	drawpoint(" ",food.X,food.Y);//删除原来的食物
	updatesnake(s);
}
void showgui(int width,int high){//显示基本界面 
	/*蛇活动范围*/
	changecolor(RED);
	++width,++high;
	for(int i=0;i<width;++i){
		printf(WALLCHAR);
	}
	printf("\n");
	for(int i=0;i<high-2;++i){
		printf(WALLCHAR);
		for(int i=0;i<width-2;++i){
			printf(" ");
		}
		printf("%s\n",WALLCHAR);
	}
	for(int i=0;i<width;++i){
		printf(WALLCHAR);
	}
	printf("\n");
	changecolor(WHITE);
	/*蛇活动范围*/
	
	/*显示分数信息等*/
	updatescore(Score);
	updatelevel(Level);
	updatelength(2);
	/*显示分数信息等*/
}
void drawpoint(const char* p,int x,int y){
	//Lock(drawmux);
	setpos(x,y);
	printf("%s",p);
	//Unlock(drawmux);
}
void drawsnakehead(Snake &s){//画蛇头 	
	changecolor(YELLOW);
	COORD p=s.pnode[0];
	switch(s.direction){//画头部 
		case 0://UP
			drawpoint(SNAKEHEADUPCHAR,p.X,p.Y);
			break;
		case 1://LEFT
			drawpoint(SNAKEHEADLEFTCHAR,p.X,p.Y);
			break;
		case 2://DOWN
			drawpoint(SNAKEHEADDOWNCHAR,p.X,p.Y);
			break;
		case 3://RIGHT
			drawpoint(SNAKEHEADRIGHTCHAR,p.X,p.Y);
			break;
	}
	changecolor(WHITE);
}
void showsnake(Snake &s){//显示蛇 
	//Lock(snakeuimux);
	COORD p;
	int i=0;
	drawsnakehead(s);
	++i;
	for(;i<s.length;++i){//画身体 
		p=*(s.pnode+i);
		drawpoint(SNAKEBODYCHAR,p.X,p.Y);
	}
	//Unlock(snakeuimux);
}
void deshowfood(COORD food){//消除食物 
	drawpoint(" ",food.X,food.Y);
	updatesnake(s);
}
void deshowsnake(Snake &s){//去除显示的蛇 
	//Lock(snakeuimux);
	COORD p=lasttail;
	drawpoint(" ",p.X,p.Y);
	for(int i=1;i<s.length;++i){
		p=*(s.pnode+i);
		drawpoint(" ",p.X,p.Y);
	}
	//Unlock(snakeuimux);
}
void replykey(char key){
	switch(key){
		case 'w':
			changedirection(s,0);
			break;
		case 'a':
			changedirection(s,1);
			break;
		case 's':
			changedirection(s,2);
			break;
		case 'd':
			changedirection(s,3);
			break;
	}
	updatesnake(s);
}
void showfood(COORD food){
	Lock(infomux);
	changecolor(GREEN);
	drawpoint(FOODCHAR,food.X,food.Y);
	changecolor(WHITE);
	setpos(zoomwidth+1+1,4);
	printf("\tNext Food:(%.3d,%.3d)",food.X,food.Y);
	Unlock(infomux);
}
void removepos(){
	setpos(zoomwidth+1+1,zoomhigh+1);
}
void updatesnake(Snake &s){
	Lock(infomux);
	//Unlock(snakeuimux);
	//deshowsnake(s);
	drawsnakehead(s);
	COORD p=s.pnode[1];
	drawpoint(SNAKEBODYCHAR,p.X,p.Y);
	p=lasttail;
	if(!checkisself(p,s)){
		drawpoint(" ",p.X,p.Y);
	}
	//showsnake(s);
	//showfood(food);
	if(debugmod)showheadinfo(s);
	removepos();
	//Unlock(snakeuimux);
	Unlock(infomux);
}
void waitforakey(char key){
	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);//获得标准输入设备句柄  
    INPUT_RECORD keyrec;//定义输入事件结构体  
    DWORD res;//定义返回记录  
    for (;;){    
        ReadConsoleInput(handle_in, &keyrec, 1, &res);//读取输入事件  
        if (keyrec.EventType == KEY_EVENT){//如果当前事件是键盘事件        
            if (keyrec.Event.KeyEvent.wVirtualKeyCode ==key  
	    		&& keyrec.Event.KeyEvent.bKeyDown == true){//当按下键时
	    			return;
  			}
        }  
    }  	
}
void init(int mod){
	isdead=0;
	showgui(zoomwidth,zoomhigh);
	COORD lt={1,1},rb={zoomwidth+1,zoomhigh+1};
	initsnake(s,zoomwidth*zoomhigh,lt,rb);
	generatefood(s);
	showfood(food);
	showsnake(s);
    updategamemod(mod);
	updateinfo("Press s to start,w/a/s/d to control");
	waitforakey('S');
	updateinfo("Game playing...");
}
void showdeadinfo(int r){
	switch(r){
		case 1:
			updateinfo("Dead by Hit Self!");
			break;
		case -1:
			updateinfo("Dead by Hit Top!");
			break;
		case -2:
			updateinfo("Dead by Hit Left!");
			break;
		case -3:
			updateinfo("Dead by Hit Bottom!");
			break;
		case -4:
			updateinfo("Dead by Hit Right!");
			break;
		default:
			updateinfo("Dead by Hit Wall!");
			break;
	}
}
void showheadinfo(Snake &s){
	setpos(zoomwidth+1+1,5);
	printf("\tSnake Head:(%.3d,%.3d)",s.pnode[0].X,s.pnode[0].Y);
}
DWORD WINAPI start(LPVOID){
	int r=0;
	for(;!isdead;){
		r=movesnakeastep(s);
		if(!r){
			updatesnake(s);
		}else{
			isdead=1;
			showdeadinfo(r);
		}		
		Sleep(1000/Speed);
	}
	return 0;
}
void cleanup(){
	freesanke(s);
}
DWORD WINAPI listenthread(LPVOID)  
{
    //printf("ID:%d\n", GetCurrentThreadId()); 
	listenkey(); 
    return 0;  
} 
void test(){
	changecolor(GREEN);
	drawpoint("O",food.X,food.Y);
	changecolor(WHITE);
	
	movesnakeastep(s);
	updatesnake(s);
	Sleep(1000);
	
	movesnakeastep(s);
	updatesnake(s);
	Sleep(1000);
	
	movesnakeastep(s);
	updatesnake(s);
}
int getfooddir(COORD sh,COORD food,int rundir){//获取食物方向 
	switch(rundir){
		case 0://UP
			if(sh.Y>food.Y)return -1;//不需要改变方向 
			if(sh.X<food.X)return RIGHT;
			if(sh.X>food.X)return LEFT;
			break;
		case 1://LEFT
			if(sh.X>food.X)return -1;
			if(sh.Y<food.Y)return DOWN;
			if(sh.Y>food.Y)return UP;
			break;
		case 2://DOWN
			if(sh.Y<food.Y)return -1;
			if(sh.X<food.X)return RIGHT;
			if(sh.X>food.X)return LEFT;
			break;
		case 3://RIGHT
			if(sh.X<food.X)return -1;
			if(sh.Y<food.Y)return DOWN;
			if(sh.Y>food.Y)return UP;
			break;
	}
	return -2;//相反方向,掉头 
}
DWORD WINAPI autorun(LPVOID){
	if(!autumod)return 0;
	int d=-1;
	for(;!isdead;){
		d=getfooddir(s.pnode[0],food,s.rundir);
		if(d==-2){
			changedirection(s,(s.rundir+1)%4);//默认左转
		}else if(d==-1){
			//changedirection(s,d);
		}else{
			changedirection(s,d);
		}
		Sleep(1000/Speed/2);
	}
	return 0;
}
int menu(){
	int mod=0;
	changecolor(YELLOW);
	printf("1.Start(normal game)\n");
	printf("2.Debug(You will not die and could set score,level and speed)\n");
	printf("3.Autorun(You can just see it run)\n");
	char c=getchar();
	fflush(stdin);
	system("cls");
	mod=(int)c;
	switch(c){
		case '1':
		    break;
		case '2':
			debugmod=1;
			printf("Enter your default score(default 0)\n");
			scanf(" %d",&Score);
			printf("Enter your default level(default 0)\n");
			scanf(" %d",&Level);
			printf("Enter your default speed(default 1)\n");
			scanf(" %d",&Speed);
			fflush(stdin);
			break;
		case '3':
			debugmod=1;
			autumod=1;
			printf("Do you want to run fastly default?\n");
			printf("1.Yes\n");
			printf("2.No\n");
			c=getchar();
			fflush(stdin);
			if(c=='1'){
				Speed=10;
				Level=60;
			}
			break;
		default:
			return 0;
	}
	system("cls");
	printf("Do you want to active sound?\n");
	printf("1.Yes\n");
	printf("2.No\n");
	needmusic=getchar()=='1';
	fflush(stdin);
	system("cls");
	changecolor(WHITE);
	return mod-'1'+1;	
} 
int main(){
	int mod=0;
	while((mod=menu())==0);
    init(mod);
    //test();
    CreateThread(NULL, 0, listenthread,NULL, 0, NULL);
    CreateThread(NULL, 0, start,NULL, 0, NULL);
    CreateThread(NULL, 0, autorun,NULL, 0, NULL);
    for(;;)Sleep(1000);
    getchar();
    cleanup();
    return 0;  
}
