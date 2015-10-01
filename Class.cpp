#include <windows.h>
#include <math.h>
#include <gl\gl.h>

#include <vector>

#include "Class.h"
#include "fmod.h"
#include "texture.h"

#define TEST

//����������:
extern TextureTga	playertex[3];				// �������飬������������
extern TextureTga  computertex[3];
extern TextureTga	ammunitiontex[4];
extern TextureTga	awardtex[3];
extern TextureTga	othertex[4];

extern unsigned int killed;

extern PlayerPlane myPlane;					// �ҷ��ɻ�
extern std::vector<ComputerPlane> computers;
extern Ammunition ammunitions[MAX_AMMUNITION];		// �ӵ�����
extern Award awards[MAX_AWARD];			// ��Ʒ����
int ammunitionIndex=0;
int awardIndex=0;
int numActiveComputer = 3;

extern FSOUND_SAMPLE *sound_1;			// �����ӵ�������ָ��
extern FSOUND_SAMPLE *sound_2;			// ����е�����ָ��
extern FSOUND_SAMPLE *sound_3;			// ������Ʒ������ָ��
extern FSOUND_SAMPLE *sound_4;			// ��Ϸ�ı�Ӱ����ָ��

extern int myPlaneNum;					// �ҷ�Ŀǰ���µķɻ���Ŀ
extern bool end;						// ������Ϸ�����Ĳ���ֵ
extern DWORD endtime;					// ������������ʼʱ��
extern int timer;						// ��һ֡������ʱ��

inline void insert_ammunition(const Ammunition & ammunition)
{
  ammunitions[ammunitionIndex++]=ammunition;
	if(ammunitionIndex==MAX_AMMUNITION)
  {
		ammunitionIndex=0;
  }
}

// �����(x1,y1)���(x2,y2)�ľ���
float distance(float x1,float y1,float x2,float y2)
{
	return (float)sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

// ��ҷɻ��ඨ��
// ��ҷɻ���ʼ��
void PlayerPlane::initPlane(float a,float b,int l,float s)
{
	x=a;
	y=b;
	life=l;
	speed=s;

	blastNum=0;
	blastTime=0;

	bombNum=START_BOMB_NUMBER;
	planeTexture=0;
	fireTime=0;
	ammunitionKind=0;
	score=0;
}
PlayerPlane::PlayerPlane(float a,float b,int l,float s)
{
	initPlane(a,b,l,s);
  fireInterval = 100;
  fireLevel = 0;
}

// ���·ɻ�
void PlayerPlane::update()
{
	if (life<=0&&blastNum==8){		// ��ҷɻ���ը���
		myPlaneNum--;
		if (myPlaneNum!=0){			// ��һ���ʣ��ɻ�
			int temp=score;
			initPlane(0,-230,100,2);
			score=temp;
		}else{
			// ��Ϸ����(ʧ��)
			endtime=GetTickCount();
			end=true;
		}
	}else if(score>=WIN_SCORE){		// ��Ϸ����(ʤ��)
		endtime=GetTickCount();
		end=true;
	}
}

// �����ӵ�
void PlayerPlane::fire()
{
	if(life>0){
		fireTime+=timer;
		if(fireTime>getFireInterval())
    {
      if (0 == fireLevel)
      {
			  Ammunition t1(x-12,y+1,90,5,100.0f,ammunitionKind);	
			  Ammunition t2(x+12,y+1,90,5,100.0f,ammunitionKind);
			  t1.setOwner(1);
			  t2.setOwner(1);
			  insert_ammunition(t1);
        insert_ammunition(t2);
      }
      else if (1 == fireLevel)
      {
			  Ammunition t1(x-12,y+1,90,5,100.0f,1);	
			  Ammunition t2(x+12,y+1,90,5,100.0f,1);
			  t1.setOwner(1);
			  t2.setOwner(1);
			  insert_ammunition(t1);
        insert_ammunition(t2);
      }
      else if (2 == fireLevel)
      {
			  Ammunition t1(x-12,y+1,90,5,100.0f,1);	
			  Ammunition t2(x+12,y+1,90,5,100.0f,1);
			  t1.setOwner(1);
			  t2.setOwner(1);
			  insert_ammunition(t1);
        insert_ammunition(t2);

        Ammunition t3(x+12, y+1, 75, 5, 150.0f, 1);
        t3.setOwner(1);
        insert_ammunition(t3);

        Ammunition t4(x-12, y+1, 105, 5, 150.0f, 1);
        t4.setOwner(1);
        insert_ammunition(t4);
      }
      else
      {
        Ammunition t(x+12, y+1, 70, 6, 150.0f, 1);
        t.setOwner(1);
        insert_ammunition(t);

        t.setX(x+6);
        t.setY(y+2);
        t.setDirection(80);
        insert_ammunition(t);

        t.setDirection(90);
        t.setX(x);
        t.setY(y+3);
        insert_ammunition(t);

        t.setX(x-6);
        t.setY(y+2);
        t.setDirection(100);
        insert_ammunition(t);

        t.setX(x-12);
        t.setDirection(110);
        insert_ammunition(t);
      }

			fireTime-=getFireInterval();
		}
	}
}

// ����ը��
void PlayerPlane::fireBomb()
{
	if(life>0 && bombNum>0){
		Ammunition t1(x,y+30,90,4,100.0f,3);
		t1.setOwner(1);
		ammunitions[ammunitionIndex++]=t1;
		if(ammunitionIndex==MAX_AMMUNITION)
			ammunitionIndex=0;
		bombNum--;
	}
}

// ��Ⲣ���Ʒɻ���ը
void PlayerPlane::blast()
{
	if(life<=0){
		blastTime+=timer;
		if(blastNum<8){
			// ���Ʊ�ըͼ��
			glPushMatrix();
			glTranslatef(x, y, 0.0f);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, othertex[2].texID);
			glBegin(GL_QUADS);
				glTexCoord2d(blastNum*0.125f,0);	glVertex2f(-50.0f,-50.0f);
				glTexCoord2d((blastNum+1)*0.125f,0);glVertex2f( 50.0f,-50.0f);
				glTexCoord2d((blastNum+1)*0.125f,1);glVertex2f( 50.0f, 50.0f);
				glTexCoord2d(blastNum*0.125f,1);	glVertex2f(-50.0f, 50.0f);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			glPopMatrix();
			// ���±�ը��ز���
			if(blastTime>30){
				if(blastNum==0)
					FSOUND_PlaySound(FSOUND_FREE,sound_2);
				blastNum++;
				blastTime=0;
			}
		}
	}
}

// ���ɻ��Ƿ���ײ
void PlayerPlane::hitcomPlane()
{
	if(life>0)
  {
    std::vector<ComputerPlane>::iterator it = computers.begin();
    for (; it != computers.end(); ++it)
    {
      ComputerPlane & com = *it;
      if (distance(x,y,com.getX(),com.getY()) < 20 && com.getLife() > 0)
      {
        life -= 100;
        com.setLife(0);
        com.setExplosible(true);
        return;
      }
    }
	}
}

// �ɻ������ƶ�
void PlayerPlane::moveUp()
{
	if(y<280 && life>0){
		y+=speed*timer/20;
		planeTexture=0;
	}
}

// �ɻ������ƶ�
void PlayerPlane::moveDown()
{
	if(y>-230 && life>0){
		y-=speed*timer/20;
		planeTexture=0;
	}
}

// �ɻ������ƶ�
void PlayerPlane::moveRight()
{
	if(life>0){
		planeTexture=2;
		if(x<230.0){
			x+=speed*timer/20;
		}
	}
}

// �ɻ������ƶ�
void PlayerPlane::moveLeft()
{
	if(life>0){
		planeTexture=1;
		if(x>-230.0){
			x-=speed*timer/20;
		}
	}
}

// �ɻ�ԭ�ز���
void PlayerPlane::stay()
{
	if(life>0)
		planeTexture=0;
}

// ���Ʒɻ�
void PlayerPlane::draw()
{
	if(life>0){
		glPushMatrix();
		glTranslatef(x, y, 0.0f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, playertex[planeTexture].texID);
		// ���÷ɻ���ɫ
		if(life>50)
			glColor3f(1.0f, 1.0f, 1.0f);
		else
			glColor3f(0.7f, 0.7f, 0.7f);
		glBegin(GL_QUADS);
			glTexCoord2i(0.0f, 0.0f);glVertex2i(-20,-20);
			glTexCoord2i(1.0f, 0.0f);glVertex2i( 20,-20);
			glTexCoord2i(1.0f, 1.0f);glVertex2i( 20, 20);
			glTexCoord2i(0.0f, 1.0f);glVertex2i(-20, 20);
		glEnd();
		glColor3f(1.0f, 1.0f, 1.0f);
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}
}

// ���Էɻ����Ա����
ComputerPlane::ComputerPlane(float a,float b,int l,float d,float s,int k):BaseObject(a,b,l,s,d,k)
{
	fireTime=0;
}


// ���Էɻ��������Ϊ0��1��2���֣����ڲ�ͬ�����࣬�����в�ͬ������ֵ���ӵ����Եȡ�
void ComputerPlane::setKind(int k)
{
	if(k==0){
		kind=0;
		speed=2;
		life=150;
		rewardScore=100;
    fireInteral = 1000;
	}else if(k==1){
		kind=1;
		speed=3;
		life=100;
		rewardScore=200;
    fireInteral = 1000;
	}else if(k==2){
		kind=2;
		speed=1;
		life=300;
		rewardScore=500;
    fireInteral = 1200;
	}
}

// ���Էɻ���ʼ��
void ComputerPlane::compinit()
{
	x=(float)(rand()%500-250);
	y=(float)(rand()%100+300);
	direction=(float)(rand()%90-135);
	blastNum=0;
	explosible=false;
	// ��ʼ�����Էɻ�����kind

#ifdef TEST_ENEMY
  setKind(2);
#else
	int temp=rand()%8;
	if(temp<=4)
		setKind(0);
	else if(temp==5 || temp==6)
		setKind(1);
	else if(temp==7)
		setKind(2);
#endif
}

// ���µ��Էɻ�
void ComputerPlane::update()
{
	if(life<=0 && blastNum==8 && explosible || life<=0 && !explosible){
		compinit();
		blastNum=0;
	}
}

// ���Ƶ��Էɻ�
void ComputerPlane::draw()
{
	if(life>0){
		glPushMatrix();
		glTranslatef(x, y, 0.0f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, computertex[kind].texID);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 0);glVertex2i(-25,-25);
			glTexCoord2i(1, 0);glVertex2i( 25,-25);
			glTexCoord2i(1, 1);glVertex2i( 25, 25);
			glTexCoord2i(0, 1);glVertex2i(-25, 25);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}
}

// ���Էɻ��ƶ�
void ComputerPlane::move()
{
	if(x>-275 && x<275 && y>-325){
		x=x+speed*cos(direction/180*PI)*timer/20;
		y=y+speed*sin(direction/180*PI)*timer/20;
	}else{
		life=0;
		explosible=false;
	}
}

// ���Էɻ��ƶ��Լ��ӵ�����
void ComputerPlane::fire()
{
	if(life>0){
		fireTime+=timer;
		// ����ٶ����
		if((fireTime>fireInteral+rand()%5000) && y>-100)
    {
			if(kind==0)
      {
				Ammunition t1(x,y-25,-90,3,100.0f,0);
				t1.setOwner(3);
        insert_ammunition(t1);
				fireTime=0;
			}
      else if(kind==1)
      { // Tracking
				int temp=atan((myPlane.getY()-(y-25))/(myPlane.getX()-x))/PI*180;
				if(myPlane.getX()<x)
        {
					temp+=180;
        }
				Ammunition t1(x,y-25,temp,3,100.0f,0);
				t1.setOwner(3);
        insert_ammunition(t1);
				fireTime=0;
			}
      else if(kind==2)
      { // Multi
				Ammunition t1(x,y-25,-90,2,100.0f,2);
				t1.setOwner(3);
        insert_ammunition(t1);

        t1.setDirection(-45);
        insert_ammunition(t1);
        t1.setDirection(-135);
        insert_ammunition(t1);
				fireTime=0;
			}
		}
	}
}

// ���Էɻ���ը
void ComputerPlane::blast()
{
	if(life<=0 && explosible && blastNum<8){
		glPushMatrix();
		glTranslatef(x, y, 0.0f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,othertex[2].texID);
		glBegin(GL_QUADS);
			glTexCoord2f(blastNum*0.125f, 0.0f);		glVertex2f(-50.0f,-50.0f);
			glTexCoord2f(blastNum*0.125f+0.125f,0.0f);	glVertex2f( 50.0f,-50.0f);
			glTexCoord2f(blastNum*0.125f+0.125f,1.0f);	glVertex2f( 50.0f, 50.0f);
			glTexCoord2f(blastNum*0.125f, 1.0f);		glVertex2f(-50.0f, 50.0f);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();

		// ��ը��������
		blastTime+=timer;
		if(blastTime>30){
			if(blastNum==0)
				FSOUND_PlaySound(FSOUND_FREE,sound_2);
			blastNum++;
			blastTime=0;
		}

		if(blastNum==8){	//��ը���
			// ���ձ�ը�˵ĵ��Էɻ�����ӷ�
			myPlane.setScore(myPlane.getScore()+rewardScore);
			
#ifdef HARD_LEVEL
			// ����2�ĸ߼��ɻ��������Ա�
			if(kind==2)
      {
				for(int i=0;i<20;i++)
        {
					Ammunition t1(x,y-25,18*i,3,100.0f,0);
					t1.setOwner(3);
					ammunitions[ammunitionIndex++]=t1;
					if(ammunitionIndex==MAX_AMMUNITION)
						ammunitionIndex=0;
				}
			}
#endif
			// �ɻ���ը��������Ʒ
			leftaward();
		}
	}
}

// ���Էɻ���ը��������Ʒ
void ComputerPlane::leftaward()
{
#ifdef TEST
  int temp =rand() % 5;
#else
	int temp=rand()%100;
#endif

	if(temp==0 || temp==99 || temp==50){			// �߼��ӵ�����
		Award t1(x,y,1,100,0);
		awards[awardIndex++]=t1;
		if(awardIndex==MAX_AWARD)
			awardIndex=0;
	}else if(temp==1){		// ��������
		Award t1(x,y,1,100,1);
		awards[awardIndex++]=t1;
		if(awardIndex==MAX_AWARD)
			awardIndex=0;
	}else if(temp==2){		// ը������
		Award t1(x,y,1,100,2);
		awards[awardIndex++]=t1;
		if(awardIndex==MAX_AWARD)
			awardIndex=0;
	}
}

void ComputerPlane::damaged(int damage)
{
	life-=damage;
	if(life<=0)
  {
		explosible=true;
    killed++;
  }
}


// Ammunition��ĳ�Ա����
// �����������֣��ֱ���kind��������0��1��2��3���֡����У�
//	0Ϊ��ͨ�ӵ���1Ϊ�ҷ��߼��ӵ���2Ϊ�з��߼��ӵ���3Ϊ�ҷ���ը��

// �ӵ���ʼ��
Ammunition::Ammunition(float a,float b,float d,float s,int l,int k):BaseObject(a,b,l,s,d,k)
{
	explosible=false;
	displacement=0;
	owner=0;
	if(kind==0)				//�����ӵ�������
		explodeLevel=20;	
	else if(kind==1)
		explodeLevel=50;
	else if(kind==2)
		explodeLevel=50;
	else if(kind==3)
		explodeLevel=100;

  switch (kind)
  {
  case 0:
    explodeLevel=20;
    break;
  case 1:
    explodeLevel=50;
    break;
  case 2:
    explodeLevel=50;
    break;
  case 3:
    explodeLevel=100;
    break;
  case 4:
    explodeLevel=500;
    break;
  default:
    explodeLevel=10;
    break;
  }

#ifdef TEST
  // Tracking
  if (kind == 2)
  {
    float deltay = myPlane.getY() - y;
    float deltax = myPlane.getX() - x;
    float angle;
    if( deltax == 0 )
    {
      if( myPlane.getY() >= y ) 
      {
        deltax = 0.0000001;
      }
      else
      {
         deltax = -0.0000001;
      }
    }
    if( deltay == 0 )
    {
      if( myPlane.getX() >= x ) 
      {
        deltay = 0.0000001;
      }
      else
      {
         deltay = -0.0000001;
      }
    }


    if( deltax>0 && deltay>0 )
    {
      angle = atan(fabs(deltay/deltax)); 
    }
    else if( deltax<0 && deltay<0 )
    {
      angle = PI+atan(fabs(deltay/deltax));
    }
    else if( deltax<0 && deltay<0 )    
    {
      angle = PI-atan(fabs(deltay/deltax));
    }
    else
    {
      angle = 2*PI-atan(fabs(deltay/deltax));
    }

    direction = angle * 180 / PI;
  }
#endif
}

// �ж��ӵ��Ƿ���зɻ�
void Ammunition::hitTheTarget()
{
	if(owner==1)
  {		// ��ҷ������ӵ�
    std::vector<ComputerPlane>::iterator it = computers.begin();
    for (; it != computers.end(); ++it)
    {
      ComputerPlane & com = *it;
      if (distance(com.getX(),com.getY(),x,y)<=25 && com.getLife() > 0)
      {
        life = 0;
        explosible = true;
        com.damaged(explodeLevel);
        return;
      }
    }
	}else if(owner==3){		// ���Է������ӵ�
#ifndef TEST
		if(distance(myPlane.getX(),myPlane.getY(),x,y)<=15 && myPlane.getLife()>0){			//�ӵ��ڵл���Χ��
			life=0;
			explosible=true;			
			myPlane.setLife(myPlane.getLife()-explodeLevel);
		}
#endif
	}
}

// �ӵ��ƶ�
void Ammunition::move()
{
	if(life>0)
  {
    if(kind!=3)
    {		// ����ը��
			if(displacement<AMMUNITION_RANGE)
      {	// �������
				x=x+speed*cos(direction/180*PI)*timer/20;	
				y=y+speed*sin(direction/180*PI)*timer/20;
				displacement+=speed*timer/20;
				hitTheTarget();				// ����Ƿ�����
			}
      else
      {
				life=0;
				explosible=false;
			}
		}
    else
    {							// ը��
			if(displacement<BOMB_RANGE)
      {
				x=x+speed*cos(direction/180*PI)*timer/20;	
				y=y+speed*sin(direction/180*PI)*timer/20;
				displacement+=speed*timer/20;
			}
      else
      {
				life=0;
				explosible=true;
			}
		}
	}
}

// �ӵ���ը
void Ammunition::blast()
{
	if(life<=0 && explosible && blastNum<8){		// �ӵ���Ч��������ը��δ��ը��
		glPushMatrix();
		glTranslatef(x, y, 0.0f);
		glEnable(GL_TEXTURE_2D);
		if(kind!=3){
			glBindTexture(GL_TEXTURE_2D, othertex[3].texID);
			glBegin(GL_QUADS);
				glTexCoord2d(blastNum*0.125f, 0.0f);		glVertex2f(-10.0f,-10.0f);
				glTexCoord2d(blastNum*0.125f+0.125f, 0.0f);	glVertex2f( 10.0f,-10.0f);
				glTexCoord2d(blastNum*0.125f+0.125f, 1.0f);	glVertex2f( 10.0f, 10.0f);
				glTexCoord2d(blastNum*0.125f, 1.0f);		glVertex2f(-10.0f, 10.0f);
			glEnd();
		}else{
			float temp=0.667*BOMB_BLAST_RANGE;
			glBindTexture(GL_TEXTURE_2D, othertex[2].texID);
			glBegin(GL_QUADS);
				glTexCoord2d(blastNum*0.125f, 0.0f);		glVertex2f(-temp,-temp);
				glTexCoord2d(blastNum*0.125f+0.125f, 0.0f);	glVertex2f( temp,-temp);
				glTexCoord2d(blastNum*0.125f+0.125f, 1.0f);	glVertex2f( temp, temp);
				glTexCoord2d(blastNum*0.125f, 1.0f);		glVertex2f(-temp, temp);
			glEnd();
		}
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();

		// ��ը��������
 		blastTime+=timer;
		if(blastTime>20){
			blastNum++;
			blastTime-=20;
		}
		
		// ը����ը�����Χ��Ŀ��
		if(kind==3 && blastNum==4){
			std::vector<ComputerPlane>::iterator it = computers.begin();
      for (; it != computers.end(); ++it)
      {
        ComputerPlane & com = *it;
        if (distance(com.getX(),com.getY(),x,y)<=BOMB_BLAST_RANGE && com.getLife() > 0)
        {
          com.damaged(explodeLevel);
        }
			}
			// �����Χ���ӵ�
			for(int i=0;i<MAX_AMMUNITION;i++){
				if(distance(ammunitions[i].getX(),ammunitions[i].getY(),x,y)<=BOMB_BLAST_RANGE
					&& ammunitions[i].getLife()>0
					&& ammunitions[i].getOwner()!=owner)
				{
					ammunitions[i].setLife(0);
					ammunitions[i].setExplosible(true);
				}
			}
		}
	}
}

// �����ӵ�
void Ammunition::draw()
{
	if(life>0)
	{
		glPushMatrix();
		glTranslatef(x, y, 0.0f);
		glEnable(GL_TEXTURE_2D);
		switch(kind)
		{
		case 0:			// ������ͨ���ӵ�
			{
				glBindTexture(GL_TEXTURE_2D, ammunitiontex[0].texID);
				glBegin(GL_QUADS);
					glTexCoord2i(0,0);glVertex2i(-3,-3);
					glTexCoord2i(1,0);glVertex2i( 3,-3);
					glTexCoord2i(1,1);glVertex2i( 3, 3);
					glTexCoord2i(0,1);glVertex2i(-3, 3);
				glEnd();
				break;
			}
		case 1:			// ������ҵĸ߼��ӵ�
			{
				glBindTexture(GL_TEXTURE_2D, ammunitiontex[2].texID);
				glBegin(GL_QUADS);
					glTexCoord2i(0, 0);glVertex2i(-1,-3);
					glTexCoord2i(1, 0);glVertex2i( 1,-3);
					glTexCoord2i(1, 1);glVertex2i( 1, 3);
					glTexCoord2i(0, 1);glVertex2i(-1, 3);
				glEnd();
				break;
			}
		case 2:			// ���Ƶ��Եĸ߼��ӵ�
			{
				glBindTexture(GL_TEXTURE_2D, ammunitiontex[0].texID);
				glColor3f(0.0f, 1.0f, 1.0f);
				glBegin(GL_QUADS);
					glTexCoord2i(0, 0);glVertex2i(-3,-3);
					glTexCoord2i(1, 0);glVertex2i( 3,-3);
					glTexCoord2i(1, 1);glVertex2i( 3, 3);
					glTexCoord2i(0, 1);glVertex2i(-3, 3);
				glEnd();
				glColor3f(1.0f, 1.0f, 1.0f);
				break;
			}
		case 3:			// ����ը��
			{
				glBindTexture(GL_TEXTURE_2D, ammunitiontex[3].texID);
				glBegin(GL_QUADS);
					glTexCoord2i(0, 0);glVertex2i(-15,-15);
					glTexCoord2i(1, 0);glVertex2i( 15,-15);
					glTexCoord2i(1, 1);glVertex2i( 15, 15);
					glTexCoord2i(0, 1);glVertex2i(-15, 15);
				glEnd();
				break;
			}
		default:
			{
				glBindTexture(GL_TEXTURE_2D, ammunitiontex[0].texID);
				glBegin(GL_QUADS);
					glTexCoord2i(0,0);glVertex2i(-3,-3);
					glTexCoord2i(1,0);glVertex2i( 3,-3);
					glTexCoord2i(1,1);glVertex2i( 3, 3);
					glTexCoord2i(0,1);glVertex2i(-3, 3);
				glEnd();
				break;
			}
			break;
		}
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}
}

// ��Ʒ��Ա����
// ��kind�������࣬��Ϊ0 �߼��ӵ�������1 ը��������2 ��������

// ��ʼ��
Award::Award(float a,float b,float s,int l,int k):BaseObject(a,b,l,s,-1,k)
{
}


// ��Ʒ����
void Award::draw()
{
	if(life>0){
		glPushMatrix();
		glTranslatef(x, y, 0.0f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, awardtex[kind].texID);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);glVertex2f(-10.0f,-10.0f);
			glTexCoord2f(1.0f,0.0f);glVertex2f( 10.0f,-10.0f);
			glTexCoord2f(1.0f,1.0f);glVertex2f( 10.0f, 10.0f);
			glTexCoord2f(0.0f,1.0f);glVertex2f(-10.0f, 10.0f);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}
}

// ��Ʒ�ƶ�
void Award::move()
{
	if(life>0){
		if(x>-240 && x<240 && y>-310 && y<300)
			y-=speed*timer/20;
		else
			life=0;
	}
}

// �ж������Ƿ���ҷɻ���
void Award::eat()
{
	if(life>0 && distance(x,y, myPlane.getX(),myPlane.getY())<30)
	{
		switch(kind)
		{
		case 0:			// �߼��ӵ�����

			{
#if 0
				if(myPlane.getAmmunitionKind()==1){
					for(int i=0;i<80;i++){
						Ammunition t1(myPlane.getX(),myPlane.getY(),9*i,3,300.0f,4);
						t1.setOwner(1);
						ammunitions[ammunitionIndex++]=t1;
						if(ammunitionIndex==MAX_AMMUNITION)
							ammunitionIndex=0;
					}
				}
				myPlane.setAmmunitionKind(1);
#endif

        if (myPlane.getFireLevel() < 3)
        {
          myPlane.levelUp();
        }
        else
        {
          myPlane.setScore(myPlane.getScore() + 1000);
        }
				life=0;
				break;
			}

		case 1:			// ��������
			{
				if(myPlane.getLife()<50)
					myPlane.setLife(myPlane.getLife()+50);
				else if(myPlane.getLife()<100)
					myPlane.setLife(100);
				life=0;
				break;
			}
		case 2:			// ը������
			{
				//myPlane.setBombNum(myPlane.getBombNum()+1);
				for(int i=0;i<80;i++){
					Ammunition t1(myPlane.getX(),myPlane.getY(),9*i,3,300.0f,4);
					t1.setOwner(1);
					ammunitions[ammunitionIndex++]=t1;
					if(ammunitionIndex==MAX_AMMUNITION)
						ammunitionIndex=0;
				}
				life=0;
				break;
			}
		default:
			break;
		}
		FSOUND_PlaySound(FSOUND_FREE,sound_3);
	}
}