#include<iostream>
#include<Windows.h>
#include<chrono>
#include<vector>
#include<utility>
#include<algorithm>
using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
// angle player is looking
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

bool bJustShot = false;
float fShotTimer = 0.0f;
// float fShotCooldown = 0.25f;

void shootRay(std::wstring& map) {
	float fRayX = sinf(fPlayerA);
	float fRayY = cosf(fPlayerA);

	float dist = 0.0f;
	while (dist < fDepth) {
		dist += 0.1f;
		
		int nTestX = (int)(fPlayerX + fRayX * dist);
		int nTestY = (int)(fPlayerY + fRayY * dist);

		// test if ray is out of bounds
		if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
			return;
		}

		if (map[nTestY * nMapWidth + nTestX] == '#') {
			// break the wall
			map[nTestY * nMapWidth + nTestX] = '.';
			return;
		}
	}
}

void drawMuzzleFlash(wchar_t* screen) {
	int cx = nScreenWidth / 2;
	int cy = nScreenHeight - 6;

	wchar_t flare = 0x2588;

	// 5×5 block (except corners)
	for (int y = -2; y <= 2; y++) {
		for (int x = -2; x <= 2; x++) {
			if (abs(x) + abs(y) < 4)   // diamond shape
				screen[(cy + y) * nScreenWidth + (cx + x)] = flare;
		}
	}
}

int main() {
	// create screen buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	std::wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#...........##.#";
	map += L"#...........##.#";
	map += L"#...........##.#";
	map += L"#...........##.#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#........#######";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	// game loop
	while (1) {
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTIme = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTIme.count();

		// controls
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (0.8f)* fElapsedTime;
		
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (0.8f)* fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

			// collision detection
			// inside of map is used to get the pos of player
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}

		fShotTimer += fElapsedTime;
		bJustShot = false;
		if ((GetAsyncKeyState((unsigned short)'Q') & 0x8000)) {
			fShotTimer = 0.0f;
			bJustShot = true;
			shootRay(map);
		}

		for (int x = 0; x < nScreenWidth; x++) {
			// we will send out 120 rays
			// for each column, calculate projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fDistToWall = 0;
			bool bHitWall = false;
			bool bBoundary = false;

			// unit vector of ray in player space
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistToWall < fDepth) {
				fDistToWall += 0.1f;

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
					if (map[nTestY * nMapWidth + nTestX] == '#') {
						bHitWall = true;

						vector<pair<float, float> > p;   // dist, angle (dot product)
						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {
								// Angle of corner to eye
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}
						}

						// Sort Pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

						float fBound = 0.01;
						// take cos -1 of dot to get angle b/ww two rays
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						// if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}
			// calculate dist to ceiling and floor, to give illusion of further away
			// as dist to wall inc., ceiling inc.
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';
			// using extended ascii
			if (fDistToWall <= fDepth / 4.0f)			nShade = 0x2588;	// Very close	
			else if (fDistToWall < fDepth / 3.0f)		nShade = 0x2593;
			else if (fDistToWall < fDepth / 2.0f)		nShade = 0x2592;
			else if (fDistToWall < fDepth)				nShade = 0x2591;
			else									    nShade = ' ';		// Too far away

			if (bBoundary)	nShade = ' ';

			for (int y = 0; y < nScreenHeight; y++) {
				if (y <= nCeiling) {
					screen[y * nScreenWidth + x] = ' ';
				}
				else if (y>nCeiling && y<= nFloor) {
					screen[y * nScreenWidth + x] = nShade;
				}
				else // Floor
				{
					// Shade floor based on distance
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}
		}

		if (bJustShot) {
			drawMuzzleFlash(screen);
		}

		// Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		// display map
		for (int nx = 0; nx < nMapWidth; nx++) {
			for (int ny = 0; ny < nMapWidth; ny++) {
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		}
		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

		// setting last char of screen to escape char
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}

	return 0;
}