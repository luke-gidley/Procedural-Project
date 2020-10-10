#pragma once
#include "EasyGraphics.h"



using namespace std;
class DrawingTool : public EasyGraphics
{

public:
	struct tile;
	DrawingTool();
	~DrawingTool();

	virtual void onDraw();
	virtual void onKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	bool playing;
	bool isRepeat;
	char press;
	int x_pos, y_pos, dx, dy;
	int level;
	int tileEntered = 0;
	bool moveOrFollow;
	bool failed;
	bool cheatClicked;
	bool populated;
	const int startX = 200;
	const int startY = 175;
	
	int REDRAW_TIMER_ID = 1;
	void onTimer(UINT nIDEvent);

	void insertionSort();

	void move();
	void initMap(int currentSize);
	void collisionDetection();
	void gameLoop();
	void drawMap(int currentTile);
	void finalTile();

	void nextLevelText();
	void failedText();

	void makePath(int level);
	void followPath();
	void addPath(int &currentTile, int& tempCurrentTile);
	bool repeatCheck(int &currentTile, int& tempCurrentTile);

	bool pathCheck(int tileEntered);
	void cheat();
	int scoreBoard();

	struct tile 
	{
		bool entered = false;
		int arrayColumn;
		int arrayRow;
		string tileName;
	};
		

	

	struct score
	{
		int scoreTotal;
		string name;
		string tempScore;
	};

	vector<tile> map;
	vector<tile> path;
	vector<score> scores;
		
	score newScore;

};



