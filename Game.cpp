
#include <windows.h>		
#include <stdio.h>


#include <gl\gl.h>			
#include <gl\glu.h>			

#include <vector>


#include "Window.h"		
#include "Class.h"			
#include "Fmod.h"			
#include "Texture.h"		


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")	
#pragma comment(lib, "fmodvc.lib")


GL_Window*	OGL_window;
Keys*		OGL_keys;


TextureTga	playertex[3];				
TextureTga  computertex[3];
TextureTga	ammunitiontex[4];
TextureTga	awardtex[3];
TextureTga	othertex[4];

GLuint	base;						

bool start;							
bool startp;						
bool end;							

bool keybombp;						
bool keyfirep;						
int myPlaneNum;						
unsigned int killed = 0;

DWORD lastTickCount=0;				
DWORD tickCount=0;					
int	 timer=0;						
DWORD	endtime;					

DWORD starttime;					

FSOUND_SAMPLE *sound_1;				
FSOUND_SAMPLE *sound_2;				
FSOUND_SAMPLE *sound_3;				
FMUSIC_MODULE *sound_4;				

float groudMove=0;					
float startQuadOffset;				

PlayerPlane myPlane;						
std::vector<ComputerPlane> computers;
Award	awards[MAX_AWARD];					
Ammunition ammunitions[MAX_AMMUNITION];		

extern int numActiveComputer;


void setTimer()
{
	lastTickCount=tickCount;
	tickCount=GetTickCount();
	timer=tickCount-lastTickCount;	
}


void GameInit(void)
{
	int i;
	for(i=0; i<MAX_AMMUNITION; i++){				
		ammunitions[i].setLife(0);					
		ammunitions[i].setExplosible(false);		
	}
	for(i=0; i<MAX_AWARD; i++){						
		awards[i].setLife(0);						
	}
  /*
	for (i=0; i<MAX_COMPUTER; i++)					
  {
		computers[i].compinit();
  }
  */
  ComputerPlane com;
  com.compinit();
  computers.push_back(com);
  com.compinit();
  computers.push_back(com);
  com.compinit();
  computers.push_back(com);
	myPlane.initPlane(0,-230,100,2);				
	myPlaneNum=MAX_PLAYER;							
	starttime=0;									
	startQuadOffset=0;								
}


GLvoid InitFMOD(GLvoid)
{
	if (FSOUND_Init(44100, 32, 0))					
	{
		sound_1=FSOUND_Sample_Load(5, "Data/fire.wav", FSOUND_LOOP_NORMAL, 0);
		sound_2=FSOUND_Sample_Load(FSOUND_FREE, "Data/hitTheTarget.wav", FSOUND_2D, 0);
		sound_3=FSOUND_Sample_Load(FSOUND_FREE, "Data/eat.wav", FSOUND_2D, 0);
		sound_4=FMUSIC_LoadSong("Data/bg.mid");
	}
}


GLvoid FreeFMOD(GLvoid)
{
	if(sound_1) FSOUND_Sample_Free(sound_1);
	if(sound_2) FSOUND_Sample_Free(sound_2);
	if(sound_3) FSOUND_Sample_Free(sound_3);
	if(sound_4) FMUSIC_FreeSong(sound_4);
}


void BuildFontGL(GLvoid)												
{
	HFONT	newFont;													
	HFONT	oldFont;													

	base = glGenLists(256);												

	newFont = CreateFont(	-18,										
							0,											
							0,											
							0,											
							FW_THIN,									
							FALSE,										
							FALSE,										
							FALSE,										
							ANSI_CHARSET,								
							OUT_TT_PRECIS,								
							CLIP_DEFAULT_PRECIS,						
							ANTIALIASED_QUALITY,						
							FF_DONTCARE|DEFAULT_PITCH,					
							"Tahoma");									

	oldFont = (HFONT)SelectObject(OGL_window->hDC, newFont); 			
	wglUseFontBitmaps(OGL_window->hDC, 0, 256, base);					
	SelectObject(OGL_window->hDC, oldFont);								
	DeleteObject(newFont);												
}

GLvoid KillFontGL(GLvoid)												
{
	glDeleteLists(base, 256);											
}

GLvoid glPrint(const char *fmt, ...)									
{
	char		text[256];												
	va_list		ap;														

	if (fmt == NULL)													
		return;															

	va_start(ap, fmt);													
		vsprintf(text, fmt, ap);										
	va_end(ap);															

	glPushAttrib(GL_LIST_BIT);											
	glListBase(base);													
	glCallLists((int)strlen(text), GL_UNSIGNED_BYTE, text);				
	glPopAttrib();														
}


void LoadTexture(void)
{
	BuildTexture("Data/Playercenter.tga", &playertex[0]);
	BuildTexture("Data/Playerleft.tga", &playertex[1]);
	BuildTexture("Data/Playerright.tga", &playertex[2]);

	BuildTexture("Data/Computer1.tga", &computertex[0]);
	BuildTexture("Data/Computer2.tga", &computertex[1]);
	BuildTexture("Data/Computer3.tga", &computertex[2]);

	BuildTexture("Data/Award1.tga", &awardtex[0]);
	BuildTexture("Data/Award2.tga", &awardtex[1]);
	BuildTexture("Data/Award3.tga", &awardtex[2]);

	BuildTexture("Data/Ball.tga", &ammunitiontex[0]);
  BuildTexture("Data/Ball_3.tga", &ammunitiontex[2]);
	BuildTexture("Data/Heavyball.tga", &ammunitiontex[1]);
	BuildTexture("Data/Missile.tga", &ammunitiontex[3]);

	BuildTexture("Data/Groud1.tga", &othertex[0]);
	
  BuildTexture("Data/Map3.tga", &othertex[1]);
	BuildTexture("Data/Planeexplode.tga", &othertex[2]);
	BuildTexture("Data/Ballexplode.tga", &othertex[3]);
}


BOOL Initialize(GL_Window* window, Keys* keys)	
{
	
	OGL_window	= window;
	OGL_keys	= keys;

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);								
	glClearDepth(1.0f);													
	glDepthFunc(GL_LEQUAL);												
	glEnable(GL_DEPTH_TEST);											
	glShadeModel(GL_SMOOTH);											
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);					
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	LoadTexture();														
	BuildFontGL();														
	

	return TRUE;														
}


void Background(void)
{
	glEnable(GL_TEXTURE_2D);
	
	glPushMatrix();
	glTranslatef(250.0f, 0.0f, 10.0f);
	glBindTexture(GL_TEXTURE_2D,othertex[0].texID);
	glBegin(GL_QUADS);
		glTexCoord2i(0,  0);glVertex2i(  0, -300);
		glTexCoord2i(4,  0);glVertex2i(150, -300);
		glTexCoord2i(4, 10);glVertex2i(150,  300);
		glTexCoord2i(0, 10);glVertex2i(  0,  300);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-250.0f, 0.0f, 10.0f);
	glBegin(GL_QUADS);
		glTexCoord2i(0, 0);glVertex2i(-150, -300);
		glTexCoord2i(4, 0);glVertex2i(   0, -300);
		glTexCoord2i(4,10);glVertex2i(   0,  300);
		glTexCoord2i(0,10);glVertex2i(-150,  300);
	glEnd();
	glPopMatrix();

	
	glPushMatrix();
	glTranslatef(0.0f, 0.0f,-50.0f);
  
	glBindTexture(GL_TEXTURE_2D, othertex[1].texID);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f+groudMove);  glVertex2i(-250,-300);
		glTexCoord2f(1.0f, 0.0f+groudMove);  glVertex2i( 250,-300);
		glTexCoord2f(1.0f, 1.0f+groudMove);  glVertex2i( 250, 300);
		glTexCoord2f(0.0f, 1.0f+groudMove);  glVertex2i(-250, 300);
	glEnd();
	glPopMatrix();

	groudMove+=0.0001f*timer;
	glDisable(GL_TEXTURE_2D);
}


void Opening(void)
{
	glRasterPos2f(-200.0f, 100.0f);
	glPrint("Press F5 to start a new game!");
	glRasterPos2f(-200.0f, 0.0f);
	glPrint(" Direction : W,S,A,D ");
	glRasterPos2f(-200.0f, -20.0f);
	glPrint(" Fire : J ; Missile: K");
	glRasterPos2f(-200.0f, -100.0f);
	int winscore=WIN_SCORE;
	glPrint("Mission : Earn %d Score! ", winscore);
}


void Running(void)
{
	int i;
	
	if(starttime<4000){
		starttime += timer;
		startp=true;						
		if(starttime<2000)
			startQuadOffset += 0.16f*timer;
		else
			startQuadOffset -= 0.16f*timer;
		
		glPushMatrix();
		glTranslatef(0.0f,-300.0f,-20.0f);
		glTranslatef(0.0f, startQuadOffset, 0.0f);
		glBindTexture(GL_TEXTURE_2D, othertex[0].texID);
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 0);glVertex2i(-250,-320);
			glTexCoord2i(2, 0);glVertex2i( 250,-320);
			glTexCoord2i(2, 2);glVertex2i( 250,   0);
			glTexCoord2i(0, 2);glVertex2i(-250,   0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
		
		glPushMatrix();
		glTranslatef(0.0f, 300.0f,-20.0f);
		glTranslatef(0.0f,-startQuadOffset, 0.0f);
		glBindTexture(GL_TEXTURE_2D, othertex[0].texID);
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 0);glVertex2i(-250,   0);
			glTexCoord2i(2, 0);glVertex2i( 250,   0);
			glTexCoord2i(2, 2);glVertex2i( 250, 320);
			glTexCoord2i(0, 2);glVertex2i(-250, 320);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
		return;
	}
	startp=false;

  /*L MAX 12 LEVEL */
  if (killed >= (computers.size() << 3) && computers.size() <= 11 )
  {
    ComputerPlane com;
    com.compinit();
    computers.push_back(com);
    killed = 0;
  }

	glEnable(GL_BLEND);				
	
  std::vector<ComputerPlane>::iterator it = computers.begin();
  for (; it != computers.end(); ++it)
  {
    ComputerPlane & com = *it;
    com.move();
    com.fire();
    com.draw();
    com.blast();
    com.update();
  }
	
	for(i=0; i<MAX_AMMUNITION; i++){
		ammunitions[i].move ();
		ammunitions[i].draw ();
		ammunitions[i].blast();
	}
	
	for(i=0; i<MAX_AWARD; i++){
		awards[i].move();
		awards[i].draw();
		awards[i].eat();
	}

	
	
	if(OGL_keys->keyDown['W']==TRUE)
		myPlane.moveUp();
	else if(OGL_keys->keyDown['S']==TRUE)
		myPlane.moveDown ();
	if(OGL_keys->keyDown['A']==TRUE)
		myPlane.moveLeft ();
	else if(OGL_keys->keyDown['D']==TRUE)
		myPlane.moveRight ();
	else
		myPlane.stay();

  /*L: Fire always  */
  myPlane.fire();

	/* Fire
	if(OGL_keys->keyDown['J']==TRUE){
		myPlane.fire();
		if(!keyfirep){
			FSOUND_PlaySound(5,sound_1);
			keyfirep=true;
		}
	}
	if(OGL_keys->keyDown['J']==FALSE || myPlaneNum<=0){
		keyfirep=false;
		FSOUND_StopSound(5);	
	}
  */
	
	if(OGL_keys->keyDown['K']==TRUE && !keybombp){
		myPlane.fireBomb();
		keybombp=true;
	}
	if(OGL_keys->keyDown['K']==FALSE)
		keybombp=false;

	myPlane.draw();			
	myPlane.hitcomPlane();	
	myPlane.blast();		
	myPlane.update();		

	glDisable(GL_BLEND);	

	
	glPushMatrix();
	glRasterPos3f(140.0f,-285.0f,10.0);
	glPrint("score:%u", myPlane.getScore());
	glPopMatrix();

  glPushMatrix();
  glRasterPos3f(140.0f, -245.0f, 5.0);
  glPrint("LEVEL:%u", computers.size());
  glPopMatrix();

#ifdef TEST
  glPushMatrix();
  glRasterPos3f(10.0f, -245.0f, 5.0);
  glPrint("X:%f, Y:%f", myPlane.getX(), myPlane.getY());
  glPopMatrix();
#endif

#if 0
	
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(240.0f,-265.0f, 0.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, playertex[0].texID);
	for(i=1; i<=myPlaneNum; i++){
		glTranslatef(-20.0f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 0); glVertex2i(-10,-10);
			glTexCoord2i(1, 0); glVertex2i( 10,-10);
			glTexCoord2i(1, 1); glVertex2i( 10, 10);
			glTexCoord2i(0, 1); glVertex2i(-10, 10);
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();

	
	glPushMatrix();
	glTranslatef(-230.0f,-265.0f,0.0);
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ammunitiontex[3].texID);
	for(i=1; i<=myPlane.getBombNum(); i++){
		glTranslatef(20.0f, 0.0f, 0.0);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 0); glVertex2i(-10,-10);
			glTexCoord2i(1, 0); glVertex2i( 10,-10);
			glTexCoord2i(1, 1); glVertex2i( 10, 10);
			glTexCoord2i(0, 1); glVertex2i(-10, 10);
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
#endif

	
	
	glPushMatrix();
	glTranslatef(-240.0f,-290.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, playertex[0].texID);
	glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2i(-10,-10);
		glTexCoord2i(1, 0); glVertex2i( 10,-10);
		glTexCoord2i(1, 1); glVertex2i( 10, 10);
		glTexCoord2i(0, 1); glVertex2i(-10, 10);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(-230.0f,-290.0f, 0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
		glVertex2f(0.0f,-5.0f);
		glVertex2f(myPlane.getLife(),-5.0f);
		glVertex2f(myPlane.getLife(), 5.0f);
		glVertex2f(0.0f, 5.0f);
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
}


void Ending(void)
{
	FSOUND_StopSound(5);						
	FMUSIC_StopSong(sound_4);
	keyfirep=false;								
	if(myPlane.getScore()<WIN_SCORE){			
		if(GetTickCount()-endtime<2000){		
			startp=true;						
			glRasterPos3f(-200.0f, 100.0f,-20.0f);
			glPrint("Final Score:  %d!", myPlane.getScore());
			glRasterPos3f(-200.0f,-100.0f,-20.0f);
			glPrint("You Lost!! Try Again!!");
		}else{
			start=false;
			startp=false;
			end=false;
		}
	}else{										
		if(GetTickCount()-endtime<2000){		
			startp=true;						
			glRasterPos3f(-200.0f, 100.0f,-20.0f);
			glPrint("Congratulations!!");
			glRasterPos3f(-200.0f,-100.0f,-20.0f);
			glPrint("YOU WIN!!");
		}else{
			start=false;
			startp=false;
			end=false;
		}
	}
}


void DrawSceneGL(void)
{
	setTimer();			
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();	

	Background();		

	if(!start){			
		Opening();		
	}else{				
		if(!end){		
			Running();	
		}else{			
			Ending();	
		}
	}

	glFlush();
}


void Update(void)
{
	if (OGL_keys->keyDown[VK_ESCAPE] == TRUE)						
	{
		TerminateApplication (OGL_window);							
	}
	if (OGL_keys->keyDown[VK_F1] == TRUE)							
	{
		PostMessage (OGL_window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);	
	}

	
	if(OGL_keys->keyDown[VK_F5] == TRUE&&!startp){

		start=!start;
		startp=true;
		if(start){
			FMUSIC_PlaySong(sound_4);
			GameInit();											
		}else{
			FMUSIC_StopSong(sound_4);
		}
	}
	if(OGL_keys->keyDown[VK_F5] == FALSE)
		startp=false;
}


void Deinitialize(void)
{
	KillFontGL();
	FreeFMOD();
}
