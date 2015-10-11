#ifndef	CLASS_H
#define CLASS_H



#define	PI					3.1415927	

#define MAX_AMMUNITION		1000	
#define MAX_COMPUTER		20			
#define MAX_AWARD			 40			
#define MAX_PLAYER			1			

#define AMMUNITION_RANGE	750			
#define BOMB_RANGE			   80			  
#define	BOMB_BLAST_RANGE	150			

#define START_BOMB_NUMBER	 0			
#define WIN_SCORE			500000		



class BaseObject{
public:
	BaseObject(float a=0,float b=0,int l=0,float s=0,float d=0,int k=0){
		x=a;
		y=b;
		life=l;
		speed=s;
		direction=d;
		kind=k;
		blastNum=0;
		blastTime=0;
	}

	float	getX(){return x;}
	float	getY(){return y;}
	int		getLife(){return life;}
	float	getSpeed(){return speed;}
	int		getKind(){return kind;}

	void	setX(float a){x=a;}
	void	setY(float b){y=b;}
	void	setLife(int l){life=l;}
	void	setSpeed(float s){speed=s;}

protected:
	float x;				
	float y;				
	float speed;			
	int life;				

	float direction;		
	int kind;				

	int blastNum;			
	int blastTime;			
};



class PlayerPlane:public BaseObject{
public:
	PlayerPlane(float a=0,float b=0,int l=0,float s=0);		
	void initPlane(float a,float b,int l,float s);			
	
	int getAmmunitionKind() {return ammunitionKind;}
	int getBombNum() {return bombNum;}
	int getScore() {return score;}

	void setAmmunitionKind(int bk) {ammunitionKind=bk;}
	void setBombNum(int bn) {bombNum=bn;}
	void setScore(int s) {score=s;}

  void setFireInterval(int i) {fireInterval=i;}
  int getFireInterval() {return fireInterval;}

  void levelUp() { fireLevel++; }
  void setFireLevel(int f) { fireLevel = f;}
  int getFireLevel() { return fireLevel;}

	void moveUp();			
	void moveDown();		
	void moveRight();		
	void moveLeft();		
	void stay();			

	void draw();			
	void blast();			
	void fire();			
	void fireBomb();		
	void hitcomPlane();		
	void update();			

private:
	int fireTime;			
	int bombNum;			
	int planeTexture;		
	int ammunitionKind;		
	int score;				
  int fireInterval; /*L default 100 */
  int fireLevel;
};


class ComputerPlane:public BaseObject{
public:
	ComputerPlane(float a=0,float b=0,int l=0,float d=0,float s=0,int k=0);	

	bool getExplosible(){ return explosible;}
	void setExplosible(bool b) {explosible=b;}
	void setKind(int k);		

	void draw();				
	void move();				
	void fire();				
	void blast();				
	void compinit();			
	void update();				
	void leftaward();			
	void damaged(int damage);	

private:
	int fireTime;				
	bool explosible;			
	int rewardScore;			
  int fireInteral;
};


/*L new kind=4, explodeLevel=100 */
class Ammunition:public BaseObject{
public:
	Ammunition(
    float x_pos=0,
    float y_pos=0,
    float direction=0,
    float speed=0,
    int lift=0,
    int kind=0);		

	void setOwner(int w) {owner=w;}
	int getOwner() {return owner;}
	void setExplosible(bool bn) {explosible=bn;}
	int getExplodeLevel(void){return explodeLevel;}

  void setY(float y_) {y=y_;};
  void setX(float x_) {x=x_;};
  void setDirection(float d_) {direction=d_;};

	void move();			
	void draw();			
	void blast();			
	void hitTheTarget();	

private:
	bool explosible;		
	float displacement;		
	int owner;				
	int explodeLevel;		
};


class Award:public BaseObject{
public:
	Award(float a=0,float b=0,float s=0,int l=0,int k=0);						

	void draw();			
	void move();			
	void eat();				
};

#endif