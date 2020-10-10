#include "DrawingTool.h"
#include <conio.h>
#include <stdio.h>
#include <time.h> 
#include <fstream>




using namespace std;
DrawingTool::DrawingTool()
{
	setImmediateDrawMode(false);
	dx = (125 + (75/2));  //initalizes where the player will be before the game starts
	dy = (100 + (75/2));
	playing = true;
	srand(time(NULL));
	level = 1;
	moveOrFollow = false;
	tileEntered = 0;
	populated = false;
	
	
	
}

DrawingTool::~DrawingTool()
{

}

//whenever anything moves the map needs to be refreshed so this is done by this method.
void DrawingTool::onDraw()
{
	int size = 10;
	clearScreen(WHITE);
	setPenColour(BLACK, 5);
	setBackColour(GREY);
	//drawRectangle(265, 90, 320, 320, true);
	if (level <= 4)
		size = 4;
	else if (level > 4 && level < 10)
		size = 5;
	initMap(size);						// this makes the vector that the drawMap inerprets, it will only make the map if a new level has started.
	collisionDetection();//this finds out what tile is entered and changes its entered value to true so that the interpreter will know wether to draw it or not.
	int currentTile = 0;
	drawMap(currentTile);//this draws the map.	
	
	scoreBoard();
	
	if (cheatClicked)
	{
		cheat();
	}

	setPenColour(BLACK, 5);
	drawCircle(dx, dy, 50/2, true);

	if (!moveOrFollow) {
		nextLevelText();
		failed = false;
		cheatClicked = false;
	}
	if (failed) {
		failedText();
	}

	drawText("High Scores", 550, 50);

	EasyGraphics::onDraw();
}


void DrawingTool::onKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	press = nChar; //this stores the most recent keypress.
	if (press == 'C') {
		cheatClicked = true;
		press = ' ';
	}

	// this is what happens when you fail, first it will display how many levels you completed then ask for a name input.
	if (failed) {
		failedText();
		if (press >= 'A' && press <= 'Z')	// takes the name input.
		{
			if (newScore.name.size() < 5)
				newScore.name += press;
			onDraw();
		}
		if (press == VK_BACK) {				// lets you erase name input.
			if (newScore.name.size() > 0)
				newScore.name.erase(newScore.name.size() - 1, 1);
			onDraw();
		}
		if (press == VK_RETURN)				// takes the name input and puts it into the vector, sorts it, and writes it to the file.
		{
			populated = false;
			newScore.scoreTotal = level - 2;
			newScore.tempScore = to_string(level);
			scores.push_back(newScore);
			insertionSort();
			reverse(scores.begin(), scores.end());
			for (int i = 0; i < scores.size(); i++)
			{
				if(i >= 10)
				scores.erase(scores.begin() + i);	//deletes anything over 10 elements.
			}
			ofstream scoreAdd;						
			scoreAdd.open("scores.txt", trunc);		// opens the file and deletes everything inside.
			if (scoreAdd.fail())
				drawText("Error opening file", 100, 100);
			for (int i = 0; i < scores.size(); i++)		//adds all of the sorted scores to the scoreboard ready to be displayed.
			{
				scores[i].tempScore = to_string(scores[i].scoreTotal);	
				scoreAdd << scores[i].name + scores[i].tempScore + "\n";
			}
			scoreAdd.close();

			//sets the game up to start again.
			dx = (125 + (75 / 2));  
			dy = (100 + (75 / 2));
			playing = true;
			level = 1;
			moveOrFollow = false;
			tileEntered = 0;
			populated = false;
			newScore.name.clear();
			onDraw();
		}
		press = ' ';
		

	}
	gameLoop();	//this starts the game loop
}


//after a key is pressed the decision on what will happen to it happens here, including wether to display the cheat, if the player gets to move or if it is the path setter.
//tested by stepping through it too see where it would take me from each click, not much testing requried as it is just a choice maker with limited inputs.
void DrawingTool::gameLoop()
{
	
	
	onDraw();

	if (!moveOrFollow) {
		moveOrFollow = true;
		
		makePath(level);
		followPath();
		level++;
		dx = startX + (75 * path[0].arrayRow - (75 / 2)); //sets the start point.
		dy = startY + (75 * path[0].arrayColumn - (75 / 2));
		press = ' ';
		tileEntered = 0;
		//failed = false;
		onDraw();
		


		}
	
	else if (moveOrFollow)
	{
		if (!failed) {
			failed = pathCheck(tileEntered);
			setTimer(REDRAW_TIMER_ID, 60);
		}
		
		}
		
}

//This function creates the map and adds the whole thing to a vector. the map is made up of tiles which are each structures, with it needing to know where its i and j are.
//this has been tested by using diffrent level attributes to get the map to be the right size, along with using breakpoints the check that the vector is properly populated.
void DrawingTool::initMap(int currentSize)
{
	if (!isRepeat)
	{
		map.clear(); //clears the map each time so that it doesnt keep adding to it.
		for (int i = 0; i < currentSize; i++)
			for (int j = 0; j < currentSize; j++)
			{
				{
					tile tile;	//makes the new tile and assigns the values too it.
					tile.tileName = "blue"; 
					tile.arrayRow = i;
					tile.arrayColumn = j;
					tile.entered = false;
					map.push_back(tile); //adds it to the map vector.
				}
			}
	}
	isRepeat = true; //this stops the map being remade each time it is called as it is part of the onDraw() function. Reason for this is there are many times where it needs to be remade, but adding a skip makes it 
					 //easier to work with.
}


//this method runs through the whole map and checks which tile has been entered, then changes that ones entered value to true so that the interpreter knows to display the entered version of that block.
//This method was tested by giving it maps of differing sizes and seeing if when a tile was entered that the entered value changed.
void DrawingTool::collisionDetection()
{
	for (int i = 0; i < map.size(); i++)
	{
		int x_axis = 125 + (75 * map[i].arrayRow);
		int y_axis = 100 + (75 * map[i].arrayColumn);
		if (dx > x_axis && dx < (x_axis + 75) && dy > y_axis && dy < (y_axis + 75))
		{
			map[i].entered = true;
			
		}

		
	}
}

//This function makes the path, depending on the level it will have a diffrent length.
// It was tested by putting in different level counts and checking how long the path given out was.
void DrawingTool::makePath(int level)
{
	tileEntered = 0;
	int currentTile = rand() % map.size(); //picks a number between 0 and the size of the map to be the start point.

	path.clear(); //path needs to be cleared every time a new one is created.
	bool repeat = false;
	int tempCurrentTile;
	path.push_back(map[currentTile]); // adds the start tile to the vector.
	

	for (int i = 0; i < (level + 4); i++) //4 instead of 3 because the first tile is the start tile. sometimes this will loop after the map is complete as there will have been some repeats.
	{
		do
		{
			if (path.size() < level + 4) // stops anything after the whole of the map being added
			{
				addPath(currentTile, tempCurrentTile);
				repeat = repeatCheck(currentTile, tempCurrentTile);
			}
			else
			{
				repeat = false;
			}
		} while (repeat);
	}
}

//this draws the actual map from the onDraw function.
//it has been tested by putting in maps of various sizes and looking for the output.
void DrawingTool::drawMap(int currentTile) 
{
	for (auto i = 0; i < sqrt(map.size()); i++)
	{

		for (auto j = 0; j < sqrt(map.size()); j++)
		{
			x_pos = 125 + (75 * map[currentTile].arrayRow);
			y_pos = 100 + (75 * map[currentTile].arrayColumn);

			auto bmp = "bmpsplit\\" + map[currentTile].tileName + ".bmp"; //lets the map know which bmp file to pick up
			wstring wstringbmp(bmp.begin(), bmp.end());

			if (map[currentTile].entered == false)
			{
				drawBitmap(wstringbmp.c_str(), x_pos, y_pos, 74, 74);
			}
			currentTile++; // goes to the next tile.
		}
	}
}

//tells the game how to follow the path.
//tested by looking at what path needs to be followed then drawing my own version, then watching what path is followed by the avatar and compairing.
void DrawingTool::followPath()
{
	dx = startX + (75 * path[0].arrayRow - (75 / 2)); //sets the start point.
	dy = startY + (75 * path[0].arrayColumn - (75 / 2));

	for (int j = 0; j < map.size() - 1; j++) {
		map[j].entered = false;
	}
	for (int pathTile = 0; pathTile < path.size() - 1; pathTile++) //for each item, the next one will be compared to it to work out where it needs to go.
	{
		for (int i = 0; i < 75; i++)
		{
			if (path[pathTile + 1].arrayRow > path[pathTile].arrayRow)
			{
				dx += 1;
			}
			else if (path[pathTile + 1].arrayRow < path[pathTile].arrayRow)
			{
				dx -= 1;
			}
			else if (path[pathTile + 1].arrayColumn > path[pathTile].arrayColumn)
			{
				dy += 1;
			}
			else if (path[pathTile + 1].arrayColumn < path[pathTile].arrayColumn)
			{
				dy -= 1;			
			}
			Sleep(5); // slows down movement slightly.
			onDraw();
		}

	}
	for (int j = 0; j < map.size(); j++) {
		map[j].entered = false;
	}
}


//this adds a new tile to the path.
//tested by looking at what the random numbers give then checking where the next tile should be. it makes sure that it cant leave the grid and if it tries it is put into the other direction.
void DrawingTool::addPath(int& currentTile, int &tempCurrentTile)
{
	
	int nextTileColumn = map[currentTile].arrayColumn;
	int nextTileRow = map[currentTile].arrayRow;
	int axis = rand() % 2;
	int direction = rand() % 2;
	if (axis == 0)// if axis is zero it goes left or right (term column says what column it is in so if column is 0 it is far left and the max is far right)
	{
		if (direction == 0)
		{
			nextTileColumn++;
		}
		else {
			nextTileColumn--;
		}
	}
	else
	{
		if (direction == 0)
		{
			nextTileRow++;
		}
		else {
			nextTileRow--;
		}

	}
		
	if (nextTileColumn == -1) // this stops it going outside the grid.
		nextTileColumn += 2;
	else if (nextTileColumn == sqrt(map.size()))
		nextTileColumn -= 2;

	if (nextTileRow == -1)
		nextTileRow += 2;
	else if (nextTileRow == sqrt(map.size()))
		nextTileRow -= 2;
	
	tempCurrentTile = currentTile;// makes a temp copy of the index of what will be the previous tile.
	for (int i = 0; i < map.size(); i++) // finds the tile it is going towards, adds it to the path.
	{
		if (map[i].arrayColumn == nextTileColumn && map[i].arrayRow == nextTileRow)
		{
			currentTile = i;
			path.push_back(map[currentTile]);
			break;
		}
	}
	
}


//this function checks if the path is going back on itself or if there is a duplicate entry directly after. while duplicates are rare they have come up once or twice so the protection is needed.
//tested by putting in paths with duplicates and backwards pathing and checking if all were caught, also heavily tested with the path generation.
bool DrawingTool::repeatCheck(int &currentTile, int &tempCurrentTile)
{
	bool repeat = false;// false by default.

	
	if (path.size() < 3) // this check is a duplicate check for the first two tiles.
	{
		if (path.size() == 2 && path[path.size() - 1].arrayColumn == path[path.size() - 2].arrayColumn && path[path.size() - 1].arrayRow == path[path.size() - 2].arrayRow)
		{
			path.erase(path.begin() + path.size() - 1);
			currentTile = tempCurrentTile; // this sets the index back to the previous tile.
		}
		repeat = true;
	}
	if (path.size() > 2) { //this checks if the map is going to go back on itself, if it is, it is removed the path.
		if (path[path.size() - 1].arrayColumn == path[path.size() - 3].arrayColumn && path[path.size() - 1].arrayRow == path[path.size() - 3].arrayRow)
		{
			path.erase(path.begin() + path.size() - 1);
			currentTile = tempCurrentTile;
			repeat = true;
		}//duplicate check.
		else if (path[path.size() - 1].arrayColumn == path[path.size() - 2].arrayColumn && path[path.size() - 1].arrayRow == path[path.size() - 2].arrayRow)
		{
			path.erase(path.begin() + path.size() - 1);
			currentTile = tempCurrentTile;
			repeat = true;
		}
	}

	return repeat;
}



//this checks if you are in the correct square at the correct time.
//tested by following the path and intentionally leaving the square and checking the result.
bool DrawingTool::pathCheck(int tileEntered)
{
	int x_axis = 125 + (75 * path[tileEntered].arrayRow);
	int y_axis = 100 + (75 * path[tileEntered].arrayColumn);
	if (dx >= x_axis && dx <= (x_axis + 75) && dy >= y_axis && dy <= (y_axis + 75))
	{
		//inside the correct square
	}	
	else
	{
		press = ' ';	//outside the correct square, stops movement.
		failed = true;

		onDraw();
	}
	
	return failed;
}


//this function is for the players movement, when it is called the player will move into the next box in the direction that the player has clicked. 
//Tested by clicking the key and looking for the corresponding movement.
void DrawingTool::onTimer(UINT nIDEvent)
{

	if (nIDEvent == REDRAW_TIMER_ID) {
		for (int i = 0; i < 75; i++) //75 as thats the size of a square.
		{

			switch (press)	//decides which way to go depending on key press.
			{
			case VK_RIGHT:
				dx += 1; break;
			case VK_LEFT:
				dx -= 1; break;
			case VK_DOWN:
				dy += 1; break;
			case VK_UP:
				dy -= 1; break;
			default:
				press = ' '; break;
			}
			if (press == ' ')	//breaks out of the whole loop, makes the game quicker and name input quicker.
				break;
			Sleep(5);
			onDraw();
			if(tileEntered == (path.size() - 2))
			finalTile();
			}
		if (press != 'C' && (press == VK_RIGHT || press == VK_LEFT || press == VK_DOWN || press == VK_UP )) {
			tileEntered++;
			pathCheck(tileEntered);
		}
		
	}

}
	
//this function checks if you have entered the final tile in a path, if you have it stops movement and sets up for the next level.
//tested by entering the final tile in a path, both before you are supposed to and when you are, it is only called when the final tile should be entered.
void DrawingTool::finalTile()
{
	int x_axis = 125 + (75 * path[path.size() - 1].arrayRow);
	int y_axis = 100 + (75 * path[path.size() - 1].arrayColumn);
	if (dx == x_axis + 75 / 2 + 1 && dy == y_axis + 75 / 2 + 1)
	{
		tileEntered = 0;
		moveOrFollow = false;
		press = ' ';
		killTimer(REDRAW_TIMER_ID);
		isRepeat = false;
		onDraw();
	}

}

//this is the text that gets displayed when you finish a level.
//tested by seeing where it appears on the screen when you finish a level and adjusting until it was what I wanted.
void DrawingTool::nextLevelText()
{
	setBackColour(YELLOW);
	setPenColour(BLACK, 3);

	drawRectangle(100, 15, 400, 80, true);

	setTextColour(BLUE);
	setFont(16, L"Tahoma");
	drawText("Press any key to start the next level!", 145, 15);
	setTextColour(RED);
	setFont(20, L"Arial");
	if(level > 1)
		cheat();
}

//this is the text that gets displayed when you fail.
//tested by seeing where it appears on the screen when you fail and adjusting until it was what I wanted.
void DrawingTool::failedText() {
	cheat();
	char levels = level - 2;
	tileEntered = 0;
	string text = "You have completed " + to_string(levels) + " levels!";

	
	drawText(text.c_str(), 250, 500);

	setBackColour(YELLOW);
	setPenColour(BLACK, 3);

	drawRectangle(160, 150, 250, 80, true);
	
	drawText("Enter name here:", 175, 150);
	drawText(newScore.name.c_str(), 175, 180);
}


//this function displays the path from the start to the end, it is used at the end of a level and when C is clicked.
//tested by putting in a vector with the path i wanted and checked against what was displayed.
void DrawingTool::cheat() {
	int previousTileCentreR = startX + (75 * path[0].arrayRow) - 75/2;
	int previousTileCentreC = startY + (75 * path[0].arrayColumn) - 75 / 2;
	int currentTileCentreR = previousTileCentreR;
	int currentTileCentreC = previousTileCentreC;
	for (int i = 0; i < path.size() - 1; i++)	// loops for the length of the path, depending on which direction the next tile is in the adds to the 
	{
		if (path[i + 1].arrayRow > path[i].arrayRow)
		{
			currentTileCentreR += 75;
		}
		else if (path[i + 1].arrayRow < path[i].arrayRow)
		{
			currentTileCentreR -= 75;
		}
		else if (path[i + 1].arrayColumn > path[i].arrayColumn)
		{
			currentTileCentreC += 75;
		}
		else if (path[i + 1].arrayColumn < path[i].arrayColumn)
		{
			currentTileCentreC -= 75;
		}
		setPenColour(RED, 3);
		drawLine(previousTileCentreR, previousTileCentreC, currentTileCentreR, currentTileCentreC);
		previousTileCentreR = currentTileCentreR; //sets the start of the next line to the end of the previous.
		previousTileCentreC = currentTileCentreC;
	}
}

//this function sets up and displays the scoreboard, it reads it from scores.txt, then it takes each line and puts it into the scores vector to be sorted.

int DrawingTool::scoreBoard()
{
	int count = 0;
	int x = 550;
	int y = 100;
	string line;
	ifstream scoreIn("scores.txt");			//opens the file

	if (scoreIn.fail())
		drawText("Error opening file", 100, 100);
	else {
		if (populated == false) {
			scores.clear();
			while (std::getline(scoreIn, line) && count < 10) {		//reads through each line of the file until it either runs out of lines or it has read 10 lines.
				count++;

				score scoreData;
				scoreData.tempScore = " ";
				scoreData.name = "";
				for (int i = 0; i < line.length(); i++) {			//adds the score data to the score part of the struct and the name part to the name of the struct.
					if (line[i] >= '0' && line[i] <= '9') {
						scoreData.tempScore += line[i];
					}
					else
						scoreData.name += line[i];
				}
				scoreData.scoreTotal = stoi(scoreData.tempScore);   //since all input from files are strings this converts the score to an int to be sorted.
				scores.push_back(scoreData);
			}
			insertionSort();
			reverse(scores.begin(), scores.end());					//sort makes it smallest to largest to this makes it largest to smallest.
		}
			for (int i = 0; i < scores.size(); i++) {				//displays the score.
				setTextColour(RED);
				setFont(20, L"Arial");
				scores[i].tempScore = to_string(scores[i].scoreTotal);
				string currentLine = scores[i].name + " - " + scores[i].tempScore;

				drawText(currentLine.c_str(), x, y);
				y += 35;
			}

		
	}
	scoreIn.close();	//closes the file.
	populated = true;
	return count;
}

//insertion sort to sort the scoreboard.
//tested with varying numbers and checking the output against my own.
void DrawingTool::insertionSort()
{
	int i, key, j; 
	for (int k = 0; k < scores.size(); k++) {
		
		for (i = 1; i < scores.size(); i++)
		{
			key = scores[i].scoreTotal;
			j = i - 1;

			while (j >= 0 && scores[j].scoreTotal > key)
			{
				string tempName = scores[j + 1].name;	// this copies the name into a temp variable so it can be written with the move.
				scores[j + 1] = scores[j];
				scores[j].name = tempName;				// writes the name into the moved file.
				j = j - 1;
			}
			scores[j + 1].scoreTotal = key;
		}
	}
}

