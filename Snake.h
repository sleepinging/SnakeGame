#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include "music.h"
#include <math.h>

#define UP 0;
#define LEFT 1;
#define DOWN 2;
#define RIGHT 3;

#define HITTOP -1
#define HITLEFT -2
#define HITBOTTMO -3
#define HITRIGHT -4
#define HITSELF 1
#define ABS(a) (a>0?a:-a)

#define WALLCHAR "#"
#define SNAKEBODYCHAR "+"
#define SNAKEHEADUPCHAR "^"
#define SNAKEHEADLEFTCHAR "<"
#define SNAKEHEADDOWNCHAR "v"
#define SNAKEHEADRIGHTCHAR ">"
#define FOODCHAR "*"

typedef struct Snake{
	short direction;//蛇头方向 
	short rundir;//前进方向 
	int length;
	int maxlen;
	COORD *pnode;
	COORD lefttop;//最左上角的墙坐标 
	COORD rigthbottom;//最右下角的墙坐标 
}Snake,snake;

COORD food={9,2};
int isdead=0;
COORD lasttail;//上次的尾巴位置 
int Score=0;
int Level=0;
int Speed=1;
int debugmod=0;//调试模式(不死) 
int autumod=0;//自动模式 
int movemux=1;
int needmusic=0;
COORD gettail(Snake &s);
void eatfood(Snake &s,COORD foodpos);
void deshowfood(COORD food);
void updatescore(int score);
void updatelevel(int level);
void updatelength(int len);
void showfood(COORD food);

void Lock(int &s){//加锁
	while(s==0);
	--s;
}
void Unlock(int &s){//解锁
	++s;
}

void initsnake(Snake &s,int maxlen,COORD lefttop,COORD rigthbottom){//初始化蛇 
	s.direction=RIGHT;
	s.rundir=RIGHT;
	s.maxlen=maxlen;
	s.length=2;
	s.pnode=NULL;
	s.pnode=(COORD*)calloc(maxlen,sizeof(COORD));
	COORD hp;
	hp.X=lefttop.X+2,hp.Y=lefttop.Y+1;
	*s.pnode=hp;
	hp.X=lefttop.X+1;
	*(s.pnode+1)=hp;
	s.lefttop=lefttop;
	s.rigthbottom=rigthbottom;
	srand((unsigned)time(NULL));
}
void freesanke(Snake &s){
	free(s.pnode);
}
int checkisself(COORD cp,Snake &s){//检测这一格是不是蛇身体
	COORD p;
	for(int i=0;i<s.length;++i){
		p=*(s.pnode+i);
		if(p.X==cp.X&&p.Y==cp.Y){
			return 1;
		}
	}
	return 0;
}
int moveleft(Snake& s);
int movedown(Snake &s);
int moveright(Snake &s);
int moveup(Snake &s){//向上移动 
	if(s.rundir==2){
		return movedown(s);
	}
	s.direction=UP;
	s.rundir=UP;
	COORD up=*s.pnode;//头部上面一格 
	--up.Y;
	if(up.Y<=s.lefttop.Y&&!debugmod){//撞到顶部 
		return HITTOP;
	}
	if(checkisself(up,s)&&!debugmod){
		return HITSELF;
	}
	if(up.X==food.X&&up.Y==food.Y){//下一格是食物 
		eatfood(s,food);
	}else{		
		for(int i=s.length;i>0;--i){
			*(s.pnode+i)=*(s.pnode+i-1);
		}
		*s.pnode=up;	
	}
	return 0;
}
int moveleft(Snake &s){//向左移动
	if(s.rundir==3){
		return moveright(s);
	}
	s.direction=LEFT;
	s.rundir=LEFT;
	COORD up=*s.pnode;//头部左面一格 
	--up.X;
	if(up.X<=s.lefttop.X&&!debugmod){//撞到左部 
		return HITLEFT;
	}
	if(checkisself(up,s)&&!debugmod){
		return HITSELF;
	}
	if(up.X==food.X&&up.Y==food.Y){//下一格是食物 
		eatfood(s,food);
	}else{		
		for(int i=s.length;i>0;--i){
			*(s.pnode+i)=*(s.pnode+i-1);
		}
		*s.pnode=up;	
	}
	return 0;
}
int movedown(Snake &s){//向下移动
	if(s.rundir==0){
		return moveup(s);
	}
	s.direction=DOWN;
	s.rundir=DOWN;
	COORD up=*s.pnode;//头部下面一格 
	++up.Y;
	if(up.Y>=s.rigthbottom.Y&&!debugmod){//撞到底部 
		return HITBOTTMO;
	}
	if(checkisself(up,s)&&!debugmod){
		return HITSELF;
	}
	if(up.X==food.X&&up.Y==food.Y){//下一格是食物 
		eatfood(s,food);
	}else{		
		for(int i=s.length;i>0;--i){
			*(s.pnode+i)=*(s.pnode+i-1);
		}
		*s.pnode=up;	
	}
	return 0;
}
int moveright(Snake &s){//向右移动
	if(s.rundir==1){
		return moveleft(s);
	}
	s.direction=RIGHT;
	s.rundir=RIGHT;
	COORD up=*s.pnode;//头部右面一格 
	++up.X;
	if(up.X>=s.rigthbottom.X&&!debugmod){//撞到右部 
		return HITRIGHT;
	}
	if(checkisself(up,s)&&!debugmod){
		return HITSELF;
	}
	if(up.X==food.X&&up.Y==food.Y){//下一格是食物 
		eatfood(s,food);
	}else{		
		for(int i=s.length;i>0;--i){
			*(s.pnode+i)=*(s.pnode+i-1);
		}
		*s.pnode=up;	
	}
	return 0;
}
void changedirection(Snake &s,int direction){
	int t=s.rundir-direction;
	if(t==2||t==-2)return;
	s.direction=direction;
}
int movesnakeastep(Snake &s){
	Lock(movemux);
	int r=0;
	lasttail=s.pnode[s.length-1];
	switch(s.direction){
		case 0://UP
			r=moveup(s);
			break;
		case 1://LEFT
			r=moveleft(s);
			break;
		case 2://DOWN
			r=movedown(s);
			break;
		case 3://RIGHT
			r=moveright(s);
			break;
	}
	Unlock(movemux);
	return r;
}
void addlenth(Snake &s,COORD addpos){//默认头部增加 
	if(s.length>=s.maxlen)return;
	++s.length;
	for(int i=s.length;i>0;--i){
		s.pnode[i]=s.pnode[i-1];
	}
	s.pnode[0]=addpos;
	updatelength(s.length);
}
int suiji(int a,int b){//生成[a,b]内随机数 
	//srand((unsigned)time(NULL));
	return (rand()%(b-a+1)+a);
}
COORD generatefood(Snake &s){
	do{	
		int x=2,y=2;
		if(debugmod){
			x=suiji(3,s.rigthbottom.X-2);
			y=suiji(3,s.rigthbottom.Y-2);
		}else{
			x=suiji(2,s.rigthbottom.X-1);
			y=suiji(2,s.rigthbottom.Y-1);
		}		
		food.X=x,food.Y=y;
	}while(checkisself(food,s));
	//printf("%d,%d",x,y);
}
void addscore(Snake &s){	
	Score+=(int)sqrt(Level)+1;
	updatescore(Score);
	if(Level>500)return;
	Level=(int)(sqrt(Score)*2);
	updatelevel(Level);
	Speed=Level+1;
}
void removepos();
void showpoint(COORD p){
	removepos();
	printf("(%d,%d)",p.X,p.Y);
}
void eatfood(Snake &s,COORD foodpos){
	if(needmusic)playeatmusic();
	addlenth(s,foodpos);
	deshowfood(food);
	generatefood(s);
	if(debugmod)showfood(food);
	addscore(s);
}
