// File: CLife.cpp
// Author: Cole Watson
// Project: CS215 Project 3 Fall 2016
// Description of file contents: 

#include "CLife.h"
#include <time.h>

CBitmap bgImage;
static CBitmap images [MAXTYPES];

using namespace std;

/*1*/ CLife::CLife ()
{
	// This function will: Creates the rows and columns on the C-life grid

	// srand (time (NULL));
	numRows = numCols = 1;
	grid = new CLifeSquare * [numRows+2];
	for (int r = 0; r < numRows+2; r++)
		grid[r] = new CLifeSquare [numCols+2];
	seed = 0;
	CLifeStatus = NEW;
}

/*2*/ CLife::CLife (const CLife & other)
{
	// This function will:  do nothing because it is empty

}

/*3*/CLife::~CLife ()
{
	// This function will: Is the delete function

	for (int r = 0; r < numRows + 2; r++)
		delete[] grid[r];
	delete[] grid;

}

/*4*/ CLife & CLife::operator = (const CLife & other)
{
	// This function will: this is the & operator

	return *this;
}

/*5*/ void CLife::LoadResources ()
{
	// This function will: will load all of the image/sound files i used in the project and associate them with the enumerated type

	int res = bgImage.LoadBitmap(CString("BACKGROUND_BMP"));
	res = images[BORDER].LoadBitmap(CString("EMPTY_BMP"));
	res = images[EMPTY].LoadBitmap(CString("EMPTY_BMP"));		
	res = images[ADDING].LoadBitmap(CString("F_ADDING_BMP"));
	res = images[PERSON].LoadBitmap(CString("FRIENDLY_BMP"));
	res = images[DELETING].LoadBitmap(CString("F_DELETING_BMP"));
	res = images[NANO].LoadBitmap(CString("NANO_BMP"));
	res = images[FANANO].LoadBitmap(CString("F_A_NANO_BMP"));
	res = images[FNANO].LoadBitmap(CString("F_NANO_BMP"));
	res = images[PLAYER].LoadBitmap(CString("PLAYER_BMP"));
	res = images[ANTIBODY].LoadBitmap(CString("ANTIBODY_BMP"));

}

/*6*/ void CLife::Init (int R, int C, CFrameWnd * windowP)
{
	// This function will: this gets called to make the initial grid based on the CLife grid

	for (int r = 0; r < numRows + 2; r++)
		delete[] grid[r];
	delete[] grid;
	generationsMap.clear();
	numRows = R < DEF_ROWS ? DEF_ROWS : R;
	numCols = C < DEF_COLS ? DEF_COLS : C;
	grid = new CLifeSquare *[numRows + 2];
	for (int r = 0; r < numRows + 2; r++)
		grid[r] = new CLifeSquare[numCols + 2];
	for (int r = 1; r <= numRows; r++)
		for (int c = 1; c <= numCols; c++)
			grid[r][c].what = EMPTY;
	populationSize = 0;
	generations = 0;	
	finished = 0;
	evaluating = true;
	CLifeStatus = NEW;
}

/*7*/ void CLife::Instructions (CFrameWnd * windowP)
{
	// This function will: This function contains the message for the instructions

     CString message;
	 message += "Press NEW then click and use the arrow keys to move the pointer, press SPACE to place bacteria. When finished, click on START. The goal is to earn points by having your colony last as long as possible, collect pills to create super-bacteria. Antibodies will destroy your bacteria. GOOD LUCK!  \n";
     
	CString title = "Instructions for Battle Bacteria.";
	windowP->MessageBox (message, title);
}

/*8*/ void CLife::Display (CFrameWnd * windowP)
{
	// This function will:  This function handles all the display/visuals

	CPaintDC dc (windowP);
	CRect rect;
	windowP->GetClientRect (&rect);
	CDC memDC;
	int res = memDC.CreateCompatibleDC(&dc);
	memDC.SelectObject(&bgImage);
	dc.TransparentBlt (0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, 1920, 1080, SRCCOPY); 
	DeleteDC (memDC);
	SetUp (rect);
	DisplayGameRect (&dc);
	dc.SetBkMode(TRANSPARENT);	
	CFont font;
	CString fontName = "Comic Sans";
	font.CreatePointFont(rect.Height() / 5, fontName, &dc);
	CFont* def_font = dc.SelectObject(&font);
	COLORREF def_color = dc.SetTextColor (RGB (0, 0, 0));
	DisplayDataRect (&dc);
	DisplayButtonRect (&dc);
	DisplayStartButtonRect(&dc);
	dc.SelectObject(def_font);
	dc.SetTextColor(def_color);
}

/*9*/ void CLife::Click (int y, int x, CFrameWnd * windowP)
{
	// This function will: gets called when the mouse is clicked and it registers where it is. Then it calls other functions based on where the click was.


	if (gameRect.PtInRect(CPoint(x, y)) && quiet == false)
	{
		for(int r = 0; r <= numRows;r++)
			for (int c = 0; c <= numCols; c++)
			{
				if (grid[r][c].where.PtInRect(CPoint(x, y)))
				{
					if (grid[r][c].what == EMPTY)
					{
						grid[PlayerRow][PlayerCol].what = EMPTY;
						windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
						PlayerRow = r;
						PlayerCol = c;
						grid[PlayerRow][PlayerCol].what = PLAYER;
						windowP->InvalidateRect(dataRect);
						windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
						CLifeStatus = READY;



					}
					if(grid[r][c].what == PERSON)
					{
						grid[PlayerRow][PlayerCol].what = EMPTY;
						windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
						PlayerRow = r;
						PlayerCol = c;
						Delete(r, c, windowP);
						grid[PlayerRow][PlayerCol].what = PLAYER;
						windowP->InvalidateRect(dataRect);
						windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
						CLifeStatus = READY;
					}
				}
			}
	}

	if (newRect.PtInRect(CPoint(x, y)))
	{
		seed = time(NULL) % 10000;
		windowP->KillTimer(1);
		Init (numRows, numCols, windowP);
		InitRandom(windowP);
		CLifeStatus = READY;
		windowP->Invalidate(true);	
	}

	if (startRect.PtInRect(CPoint(x, y)))
	{
		quiet = true;
		numSeconds = 5;
		windowP->SetTimer(1, 400, NULL);
		CLifeStatus = READY;
		UpdateTime(windowP);
		// pauseButton = False;
	}
	
}

/*10*/ void CLife::Move (char direction, CFrameWnd * windowP)
{
	// This function will: This will get called when the WASD or SPACE is pressed. it is used to place the bacteria.
	if (!quiet)
	{
		CRect oldWhere;
		switch (direction)
		{
		case 'a': // moving left
			if (grid[PlayerRow][PlayerCol - 1].what != BORDER && grid[PlayerRow][PlayerCol - 1].what != PERSON && grid[PlayerRow][PlayerCol - 1].what != NANO && grid[PlayerRow][PlayerCol - 1].what != ANTIBODY)
			{
				if (!pressedSpace)
				{
					grid[PlayerRow][PlayerCol].what = EMPTY;
				}
				else
				{
					grid[PlayerRow][PlayerCol].what = PERSON;
					populationSize++;

				}
				windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
				windowP->InvalidateRect(dataRect);
				PlayerCol -= 1;
				grid[PlayerRow][PlayerCol].what = PLAYER;
				windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
				pressedSpace = false;
			}
			break;
		case 'w': // moving up
			if (grid[PlayerRow - 1][PlayerCol].what != BORDER && grid[PlayerRow - 1][PlayerCol].what != PERSON && grid[PlayerRow - 1][PlayerCol].what != NANO && grid[PlayerRow - 1][PlayerCol].what != ANTIBODY)
			{
				if (!pressedSpace)
				{
					grid[PlayerRow][PlayerCol].what = EMPTY;
				}
				else
				{
					grid[PlayerRow][PlayerCol].what = PERSON;
					populationSize++;
				}
				windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
				windowP->InvalidateRect(dataRect);
				PlayerRow -= 1;
				grid[PlayerRow][PlayerCol].what = PLAYER;
				windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
				pressedSpace = false;
			}
			break;
		case 'd': // moving right
			if (grid[PlayerRow][PlayerCol + 1].what != BORDER && grid[PlayerRow][PlayerCol + 1].what != PERSON && grid[PlayerRow][PlayerCol + 1].what != NANO && grid[PlayerRow][PlayerCol + 1].what != ANTIBODY)
			{
				if (!pressedSpace)
				{
					grid[PlayerRow][PlayerCol].what = EMPTY;
				}
				else
				{
					grid[PlayerRow][PlayerCol].what = PERSON;
					populationSize++;
				}
				windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
				windowP->InvalidateRect(dataRect);
				PlayerCol += 1;
				grid[PlayerRow][PlayerCol].what = PLAYER;
				windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
				pressedSpace = false;
			}
			break;
		case 's': // moving down
			if (grid[PlayerRow + 1][PlayerCol].what != BORDER && grid[PlayerRow + 1][PlayerCol].what != PERSON && grid[PlayerRow + 1][PlayerCol].what != NANO && grid[PlayerRow + 1][PlayerCol].what != ANTIBODY)
			{
				if (!pressedSpace)
				{
					grid[PlayerRow][PlayerCol].what = EMPTY;
				}
				else
				{
					grid[PlayerRow][PlayerCol].what = PERSON;
					populationSize++;
				}
				windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
				windowP->InvalidateRect(dataRect);
				PlayerRow += 1;
				grid[PlayerRow][PlayerCol].what = PLAYER;
				windowP->InvalidateRect(grid[PlayerRow][PlayerCol].where);
				pressedSpace = false;
			}
			break;
		case VK_SPACE://adds a guy
			if (grid[PlayerRow][PlayerCol].what != BORDER && grid[PlayerRow][PlayerCol].what != PERSON)
			{
				Add(PlayerRow, PlayerCol, PERSON, windowP);
				windowP->InvalidateRect(dataRect);
				pressedSpace = true;
			}

			break;



		}
		CRect newWhere;
		windowP->InvalidateRect(oldWhere | newWhere);
	}
}

/*11*/ void CLife::UpdateTime (CFrameWnd * windowP)
{
	// This function will: this function is the timer function and allows it to be set to repeativly iterate through Next

	if (!finished)
		Next (windowP);
	else
	{
		windowP->KillTimer(1);
		CLifeStatus = FINISHED;
	}
}

/*12*/ void CLife::Message (CFrameWnd * windowP)
{
	// This function will: this will pop up at the end of the game and show the score and tell you if your community created a stable pattern or the populationSize hit 0
	CString str;
	str.Format(L"%i", score);
	CString message = "Your score is " + str + "\n\n";
	CString title = "Game Over";
	quiet = false;
	started = false;
	switch (finished)
	{
		case -1: message += "The community has vanished.";
			break;
		default: message += "The community has entered a stable pattern.";
	}
	windowP->KillTimer(1);
	windowP->MessageBox (message, title);
}

/*13*/ bool CLife::Done(CFrameWnd * windowP)
{
	// This function will: When the game is finished Done is called allowing the message to appear.

	return finished != 0;
}

/*14*/ CLife::CLifeSquare::CLifeSquare ()
{
	// This function will: This sets the outside to be the boarder of it.

	what = BORDER;
}

/*15*/ void CLife::CLifeSquare::Display (CDC * deviceContextP)
{
	// This function will: sets up the display on the screen for squares and images

	deviceContextP->Rectangle(where);
	CDC memDC;
	int res = memDC.CreateCompatibleDC(deviceContextP);
	memDC.SelectObject(&images[what]);
	res = deviceContextP->TransparentBlt (where.left, where.top, where.Width(), where.Height(), &memDC, 0, 0, 100, 100, SRCCOPY); 
	DeleteDC (memDC);	
}

/*16*/ int CLife::CLifeSquare::Reset(CFrameWnd * windowP)
{
	// This function will: this is called at the end of each next to check if its adding it adds the person, if deleting then it deletes the person, etc.
	if (what == FANANO)
	{
		what = FNANO;
		windowP->InvalidateRect(where);
		return 1;
	}

	if (what == ADDING)
	{
		what = PERSON;
		windowP->InvalidateRect (where);
		return 1;
	}
	if (what == DELETING)
	{
		what = EMPTY;
		windowP->InvalidateRect (where);
		return -1;
	}
	return 0;
}

/*17*/ void CLife::SetUp (CRect rect)
{
	// This function will:  This is the basic set up for the display and dimensions of the boxes

	int tbBorder = rect.Height() / 12;
	int lrBorder = rect.Width() / 12;
	gameRect = CRect (rect.Width() / 3, tbBorder, rect.Width()-lrBorder, rect.Height()-tbBorder);
	dataRect = CRect (lrBorder, rect.Height() / 3, rect.Width() / 4, 2 * rect.Height() / 3);
	buttonRect = CRect (lrBorder, 3 * rect.Height() / 4, rect.Width() / 4, rect.Height() - tbBorder);
	int thirdW = buttonRect.Width() / 3;
	newRect = CRect (buttonRect.left, buttonRect.top, buttonRect.left + thirdW, buttonRect.bottom);
	startRect = CRect(buttonRect.left + 2 * thirdW, buttonRect.top, buttonRect.left + 4 * thirdW, buttonRect.bottom);

	int sqHeight = gameRect.Height() / numRows;
	int sqWidth = gameRect.Width() / numCols;
	int top = gameRect.top + (gameRect.Height() - sqHeight * numRows) / 2;
	int left = gameRect.left + (gameRect.Width() - sqWidth * numCols) / 2;
	int size = sqHeight < sqWidth ? sqHeight : sqWidth;
	for (int r = 1; r <= numRows; r++)
		for (int c = 1; c <= numCols; c++)
		{
			grid[r][c].where = CRect(left + (c-1)*sqWidth + 1, top + (r-1)*sqHeight + 1, left + (c)*sqWidth, top + (r)*sqHeight);
		}
}

/*18*/ void CLife::DisplayGameRect (CDC * deviceContextP)
{
	// This function will: actually displays the grid based on CLIFE

	for (int r = 1; r <= numRows; r++)
		for (int c = 1; c <= numCols; c++)
			grid [r][c].Display (deviceContextP);
}

/*19*/ void CLife::DisplayDataRect (CDC * deviceContextP)
{
	// This function will: shows the top box which contains population, score and generations
	
	CPoint corner = CPoint(dataRect.Height()/4, dataRect.Height()/4);  	
	deviceContextP->RoundRect(dataRect, corner);
	CRect genRect = CRect (dataRect.left, dataRect.top, dataRect.right, dataRect.top + dataRect.Height() / 2);
	CRect popRect = CRect (dataRect.left, dataRect.top + dataRect.Height() / 2, dataRect.right, dataRect.bottom);
	CRect scoreRect = CRect(dataRect.left, dataRect.top - dataRect.Height() / 10, dataRect.right, dataRect.bottom);
	char buffer [50];
	sprintf_s (buffer, "Generations: %d", generations);
	deviceContextP->DrawText(CString (buffer), genRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		sprintf_s(buffer, "Population: %d", populationSize);
		deviceContextP->DrawText(CString(buffer), popRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	if (started)
	{
		sprintf_s(buffer, "Score: %d", score);
		deviceContextP->DrawText(CString(buffer), scoreRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}

}
/*20*/ void CLife::DisplayButtonRect (CDC * deviceContextP)
{
	// This function will:  displays the new button which when pressed calls InitRandom
	
	CPoint corner = CPoint(buttonRect.Height()/4, buttonRect.Height()/4);  
	deviceContextP->RoundRect(newRect, corner);
	deviceContextP->DrawText(CString ("New"), newRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}

/*21.1*/ void CLife::DisplayStartButtonRect(CDC * deviceContextP)
{
	// This function will: start the game is the start button which when clicked uses the timer to iterate through Next

	CPoint corner = CPoint(startRect.Height() / 4, startRect.Height() / 4);
	deviceContextP->RoundRect(startRect, corner);
	deviceContextP->DrawText(CString("Start"), startRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}


/*21*/ void CLife::InitRandom(CFrameWnd * windowP)
{
	// This function will:  randomly generates a board of antibodies and nano pills
	score = 0;
	started = true;
	srand (seed);
	for (int p = 0; p < 7; p++)
	{
		int r = 1 + rand () % numRows;
		int c = 1 + rand () % numCols;
		Add (r, c, NANO, windowP);
	}
	srand(seed + 1);
	for (int p = 0; p < 25; p++)
	{
		int r = 1 + rand() % numRows;
		int c = 1 + rand() % numCols;
		Delete(r, c, windowP);
		Add(r, c, ANTIBODY, windowP);
	}
}

/*22*/ void CLife::Next(CFrameWnd * windowP)
{
	// This function will: next goes through every generation. it calculates population size, generation and calculates score.
	roundScore = 0;
	if (evaluating)
		for (int r = 1; r <= numRows; r++)
			for (int c = 1; c <= numCols; c++)
				Evaluate(r, c, windowP);
	else
	{
		for (int r = 1; r <= numRows; r++)
			for (int c = 1; c <= numCols; c++)
				populationSize += grid[r][c].Reset(windowP);
		if (populationSize == 0)
			finished = -1;	
		string mapStr = MakeString();
		map<string, int>::iterator itr = generationsMap.find(mapStr);
		if (itr == generationsMap.end())
		{
			generations++;
			generationsMap[mapStr] = generations;
		}
		else
			finished = itr->second;
		windowP->InvalidateRect (dataRect);
	}
	score += populationSize + (roundScore *  generations);
	evaluating = !evaluating;
}

/*23*/ bool CLife::Add(int row, int col, square_types what, CFrameWnd * windowP)
{
	// This function will: this function is called on a certain square and adds a person, antibody or nano based on perameters

	if (row < 1 || row > numRows)
		return false;
	if (col < 1 || col > numCols)
		return false;
	if (grid[row][col].what != EMPTY && grid[row][col].what != NANO)
		return false;
	if (what == PERSON)
	{
		//if (grid[row][col].what == ANTIBODY)
		//	return false;
		grid[row][col].what = what;
		windowP->InvalidateRect(grid[row][col].where);
		populationSize++;
		roundScore++;
		return true;
	}
	else if (what == ANTIBODY)
	{
		grid[row][col].what = what;
		windowP->InvalidateRect(grid[row][col].where);
		return true;
	}
	else if (what == NANO) // the little pill things
	{
		grid[row][col].what = what;
		windowP->InvalidateRect(grid[row][col].where);
		roundScore += 15;
		return true;
	}
	return false;
}

/*24*/ bool CLife::Delete(int row, int col, CFrameWnd * windowP)
{
	// This function will: Deletes the bacteria if it dies and is called on a certain square

	if (row < 1 || row > numRows)
		return false;
	if (col < 1 || col > numCols)
		return false;
	if (grid[row][col].what <= EMPTY)
		return false;
	grid[row][col].what = EMPTY;
	windowP->InvalidateRect (grid[row][col].where);
	populationSize--;
	return true;
}

/*25*/ void CLife::Evaluate(int row, int col, CFrameWnd * windowP)
{
	// This function will: is called on a certain square and it checks all of its neighbors and adds up the neighbor count. When the game starts it also cleans up the player which is used to place bacteria 
	int neighbors = 0;
	if (grid[row - 1][col - 1].what >= PERSON)
		neighbors++;

	if (grid[row][col - 1].what >= PERSON)
		neighbors++;

	if (grid[row + 1][col - 1].what >= PERSON)
		neighbors++;

	if (grid[row - 1][col].what >= PERSON)
		neighbors++;

	if (grid[row + 1][col].what >= PERSON)
		neighbors++;

	if (grid[row - 1][col + 1].what >= PERSON)
		neighbors++;

	if (grid[row][col + 1].what >= PERSON)
		neighbors++;

	if (grid[row + 1][col + 1].what >= PERSON)
		neighbors++;

	if (grid[row][col].what == NANO && neighbors == 3)
	{
		grid[row][col].what = FANANO;
		windowP->InvalidateRect(grid[row][col].where);
	}

	if (grid[row][col].what == EMPTY && neighbors == 3)
	{
		grid[row][col].what = ADDING;
		windowP->InvalidateRect(grid[row][col].where);
	}

	if (grid[row][col].what == PERSON && neighbors >= 4)
	{
		grid[row][col].what = DELETING;
		windowP->InvalidateRect(grid[row][col].where);
	}

	if (grid[row][col].what == PERSON && neighbors <= 1)
	{
		grid[row][col].what = DELETING;
		windowP->InvalidateRect(grid[row][col].where);
	}

	if (grid[row][col].what == FNANO && neighbors == 0)
	{
		grid[row][col].what = DELETING;
		windowP->InvalidateRect(grid[row][col].where);
	}
	if (grid[row][col].what == PLAYER)
	{
		grid[row][col].what = EMPTY;
		windowP->InvalidateRect(grid[row][col].where);
	}
}

/*26*/ string CLife::MakeString() const
{
	// This function will: makes the grid into a string which makes GenerationsMap
	string str;
	for (int r = 0; r < numRows; r++)
	{
		for (int c = 0; c < numCols; c++)
		{
			if (grid[r][c].what == PERSON)
				str += "P";
			if (grid[r][c].what == EMPTY)
				str += "E";
			if (grid[r][c].what == NANO)
				str += "N";
			if (grid[r][c].what == FNANO)
				str += "F";

		}

	}
	return str;
}
