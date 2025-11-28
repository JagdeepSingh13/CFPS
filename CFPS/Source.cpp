#include<iostream>
#include<Windows.h>
#include<chrono>
#include<vector>
#include<utility>
#include<algorithm>
#include<olcConsoleGameEngine.h>
using namespace std;

class Ultimate_FPS : public olcConsoleGameEngine {
public:
	Ultimate_FPS() {
		m_sAppName = L"ULtimate FPS";
	}
private:
	int nMapWidth = 32;
	int nMapHeight = 32;

	float fPlayerX = 8.0f;
	float fPlayerY = 8.0f;
	float fPlayerA = -3.14159f / 2.0f;
	float fFOV = 3.14159f / 4.0f;
	float fDepth = 16.0f;
	float fSpeed = 5.0f;
	std::wstring map;

	olcSprite* spriteWall;

protected:
	virtual bool OnUserCreate() {
		map += L"#########.......#########.......";
		map += L"#...............#...............";
		map += L"#.......#########.......########";
		map += L"#..............##..............#";
		map += L"#......##......##......##......#";
		map += L"#......##..............##......#";
		map += L"#..............##..............#";
		map += L"###............####............#";
		map += L"##.............###.............#";
		map += L"#............####............###";
		map += L"#..............................#";
		map += L"#..............##..............#";
		map += L"#..............##..............#";
		map += L"#...........#####...........####";
		map += L"#..............................#";
		map += L"###..####....########....#######";
		map += L"####.####.......######..........";
		map += L"#...............#...............";
		map += L"#.......#########.......##..####";
		map += L"#..............##..............#";
		map += L"#......##......##.......#......#";
		map += L"#......##......##......##......#";
		map += L"#..............##..............#";
		map += L"###............####............#";
		map += L"##.............###.............#";
		map += L"#............####............###";
		map += L"#..............................#";
		map += L"#..............................#";
		map += L"#..............##..............#";
		map += L"#...........##..............####";
		map += L"#..............##..............#";
		map += L"################################";

		spriteWall = new olcSprite(L"../src/fps_wall1.spr");

		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime) {
#ifdef _DEBUG
		if (m_bufScreen == nullptr) {
			MessageBoxA(nullptr, "m_bufScreen is NULL in OnUserUpdate()", "Debug", MB_ICONERROR);
			return false;
		}
#endif

		// Handle CCW Rotation
		if (m_keys[L'A'].bHeld)
			fPlayerA -= (fSpeed * 0.5f) * fElapsedTime;

		// Handle CW Rotation
		if (m_keys[L'D'].bHeld)
			fPlayerA += (fSpeed * 0.5f) * fElapsedTime;

		// Handle Forwards movement & collision
		if (m_keys[L'W'].bHeld)
		{
			fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
			if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
				fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
			}
		}

		// Handle backwards movement & collision
		if (m_keys[L'S'].bHeld)
		{
			fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
				fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
			}
		}

		// Handle Strafe Right movement & collision
		if (m_keys[L'E'].bHeld)
		{
			fPlayerX += cosf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY -= sinf(fPlayerA) * fSpeed * fElapsedTime;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX -= cosf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY += sinf(fPlayerA) * fSpeed * fElapsedTime;
			}
		}

		// Handle Strafe Left movement & collision
		if (m_keys[L'Q'].bHeld)
		{
			fPlayerX -= cosf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY += sinf(fPlayerA) * fSpeed * fElapsedTime;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX += cosf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY -= sinf(fPlayerA) * fSpeed * fElapsedTime;
			}
		}

		for (int x = 0; x < ScreenWidth(); x++) {
			// we will send out 120 rays
			// for each column, calculate projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)ScreenWidth()) * fFOV;

			float fDistToWall = 0;
			float fStepSize = 0.1f;
			bool bHitWall = false;
			bool bBoundary = false;

			float fSampleX = 0.0f;

			// unit vector of ray in player space
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistToWall < fDepth) {
				fDistToWall += fStepSize;

				// move in direction of unit vector
				int nTestX = (int)(fPlayerX + fEyeX * fDistToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistToWall);

				// test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = true;
					fDistToWall = fDepth;
				}
				else {
					// ray is inbounds so check if cell is a wall block
					if (map.c_str()[nTestY * nMapWidth + nTestX] == '#') {
						bHitWall = true;
						
						// consider cell is 1*1
						float fBlockMidX = (float)nTestX + 0.5f;
						float fBlockMidY = (float)nTestY + 0.5f;

						// point where collision has occured
						float fTestPointX = fPlayerX + fEyeX * fDistToWall;
						float fTestPointY = fPlayerY + fEyeY * fDistToWall;

						float fTestAngle = atan2f((fTestPointY - fBlockMidY), (fTestPointX - fBlockMidX));

						// rotate by pi/4 so each edge is one quadrant
						if (fTestAngle >= -3.14159f * 0.25f && fTestAngle < 3.14159f * 0.25f)
							fSampleX = fTestPointY - (float)nTestY;
						if (fTestAngle >= 3.14159f * 0.25f && fTestAngle < 3.14159f * 0.75f)
							fSampleX = fTestPointX - (float)nTestX;
						if (fTestAngle < -3.14159f * 0.25f && fTestAngle >= -3.14159f * 0.75f)
							fSampleX = fTestPointX - (float)nTestX;
						if (fTestAngle >= 3.14159f * 0.75f || fTestAngle < -3.14159f * 0.75f)
							fSampleX = fTestPointY - (float)nTestY;
					}
				}
			}
			// calculate dist to ceiling and floor, to give illusion of further away
			// as dist to wall inc., ceiling inc.
			int nCeiling = (float)(ScreenHeight() / 2.0) - ScreenHeight() / ((float)fDistToWall);
			int nFloor = ScreenHeight() - nCeiling;

			short nShade = ' ';
			// using extended ascii
			if (fDistToWall <= fDepth / 4.0f)			nShade = 0x2588;	// Very close	
			else if (fDistToWall < fDepth / 3.0f)		nShade = 0x2593;
			else if (fDistToWall < fDepth / 2.0f)		nShade = 0x2592;
			else if (fDistToWall < fDepth)				nShade = 0x2591;
			else									    nShade = ' ';		// Too far away

			if (bBoundary)	nShade = ' ';

			for (int y = 0; y < ScreenHeight(); y++) {
				if (y <= nCeiling) {
					Draw(x, y, L' ');
				}
				else if (y > nCeiling && y <= nFloor) {
					Draw(x, y, nShade);
				}
				else // Floor
				{
					// Shade floor based on distance
					float b = 1.0f - (((float)y - ScreenHeight() / 2.0f) / ((float)ScreenHeight() / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					Draw(x, y, nShade);
				}
			}
		}

		// Display Map & Player
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
				Draw(nx + 1, ny + 1, map[ny * nMapWidth + nx]);
		Draw(1 + (int)fPlayerX, 1 + (int)fPlayerY, L'P');

		return true;
	}
};

int main() {
	Ultimate_FPS game;

	if (!game.ConstructConsole(240, 140, 4, 4)) {
		std::wcerr << L"Console construction failed!" << endl;
		return -1;
	}

	game.Start();
	return 0;
}