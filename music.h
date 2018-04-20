#include<stdio.h>
#include<windows.h>
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")

DWORD WINAPI _playmusic(LPVOID wavpath)
{
	//PlaySound((char*)wavpath,0,SND_FILENAME|SND_NOWAIT);
	char *c=NULL;
	char *c1="open ";
	char *c2=" alias a1 wait";
	size_t l=strlen(c1)+strlen((char*)wavpath)+strlen(c2)+1;
	c=(char*)calloc(l,sizeof(char));
	if(!c)return 0;
	strcat(c,c1);
	strcat(c,(char*)wavpath);
	strcat(c,c2);
	mciSendString(c,NULL,0,NULL);
	mciSendString("play a1",0,0,0);
	free(c);
	return 0;
}

int playmusic(const char* wavpath)
{
	CreateThread(NULL, 0, _playmusic,(LPVOID)wavpath, 0, NULL);
	return 0;
}

void playeatmusic(){
	playmusic("eat.wav");
}
