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

	// to store walls, don't show lamps behind walls
	float* fDepthBuffer = nullptr;

	olcSprite* spriteWall;
		
	struct sObject {
		float x, y; 
		float vx, vy;
		bool bRemove;
		olcSprite* sprite;
	};
	list<sObject> listObjects;

	olcSprite* spriteLamp;
	olcSprite* spriteFireball;

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

		fDepthBuffer = new float[ScreenWidth()];

		// right click for properties then (true)
		spriteWall = new olcSprite(L"src/fps_wall1.spr");
		spriteLamp = new olcSprite(L"src/fps_lamp1.spr");
		spriteFireball = new olcSprite(L"src/fps_fireball1.spr");

		listObjects = {
			{ 8.5f, 8.5f, 0.0f, 0.0f, false, spriteLamp },
			{ 7.5f, 7.5f, 0.0f, 0.0f, false, spriteLamp },
			{ 10.5f, 3.5f, 0.0f, 0.0f, false, spriteLamp },
		};

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

		if (m_keys[VK_SPACE].bReleased) {
			sObject o;
			o.x = fPlayerX;
			o.y = fPlayerY;

			float fNoise = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.1f;
			o.vx = sinf(fPlayerA + fNoise) * 8.0f;
			o.vy = cosf(fPlayerA + fNoise) * 8.0f;

			o.sprite = spriteFireball;
			o.bRemove = false;
			listObjects.push_back(o);
		}

		for (int x = 0; x < ScreenWidth(); x++) {
			// we will send out 120 rays
			// for each column, calculate projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)ScreenWidth()) * fFOV;

			float fDistToWall = 0;
			float fStepSize = 0.01f;
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

			fDepthBuffer[x] = fDistToWall;

			for (int y = 0; y < ScreenHeight(); y++) {
				if (y <= nCeiling) {
					Draw(x, y, L' ');
				}
				else if (y > nCeiling && y <= nFloor) {
					if (fDistToWall < fDepth) {
						float fSampleY = ((float)y - (float)nCeiling) / ((float)nFloor - (float)nCeiling);
						Draw(x, y, spriteWall->SampleGlyph(fSampleX, fSampleY), spriteWall->SampleColour(fSampleX, fSampleY));
					}
					else {
						Draw(x, y, PIXEL_SOLID, 0);
					}
				}
				else // Floor
				{
					// dark green floor
					Draw(x, y, PIXEL_SOLID, FG_DARK_GREEN);
				}
			}
		}

		// Draw objects
		for (auto& object : listObjects) {
			// update object physics
			object.x += object.vx * fElapsedTime;
			object.y += object.vy * fElapsedTime;

			// Check if object is inside wall - set flag for removal
			if (map.c_str()[(int)object.x * nMapWidth + (int)object.y] == '#')
				object.bRemove = true;

			// can it be seen?
			float fVecX = object.x - fPlayerX;
			float fVecY = object.y - fPlayerY;
			float fDistFromPlayer = sqrtf(fVecX * fVecX + fVecY * fVecY);

			// check if in FOV?
			float fEyeX = sinf(fPlayerA);
			float fEyeY = cosf(fPlayerA);
			float fObjectAngle = atan2f(fEyeY, fEyeX) - atan2f(fVecY, fVecX);
			if (fObjectAngle < -3.14159f)
				fObjectAngle += 2.0f * 3.14159f;
			if (fObjectAngle > 3.14159f)
				fObjectAngle -= 2.0f * 3.14159f;

			bool bInFov = fabs(fObjectAngle) < fFOV / 2.0f;

			if (bInFov && fDistFromPlayer >= 0.5f && fDistFromPlayer < fDepth) {
				// object will have variable height like walls
				float fObjectCeiling = (float)(ScreenHeight() / 2.0) - ScreenHeight() / ((float)fDistFromPlayer);
				float fObjectFloor = ScreenHeight() - fObjectCeiling;
				float fObjectHeight = fObjectFloor - fObjectCeiling;

				float fObjectAspectRatio = (float)object.sprite->nHeight / (float)object.sprite->nWidth;
				float fObjectWidth = fObjectHeight / fObjectAspectRatio;

				float fMiddleOfObject = (0.5f * (fObjectAngle / (fFOV / 2.0f)) + 0.5f) * (float)ScreenWidth();

				// Draw Lamp
				for (float lx = 0; lx < fObjectWidth; lx++)
				{
					for (float ly = 0; ly < fObjectHeight; ly++)
					{
						float fSampleX = lx / fObjectWidth;
						float fSampleY = ly / fObjectHeight;
						wchar_t c = object.sprite->SampleGlyph(fSampleX, fSampleY);
						int nObjectColumn = (int)(fMiddleOfObject + lx - (fObjectWidth / 2.0f));
						if (nObjectColumn >= 0 && nObjectColumn < ScreenWidth()) {
							if (c != L' ' && fDepthBuffer[nObjectColumn] >= fDistFromPlayer) {
								Draw(nObjectColumn, fObjectCeiling + ly, c, object.sprite->SampleColour(fSampleX, fSampleY));
								fDepthBuffer[nObjectColumn] = fDistFromPlayer;
							}
						}
					}
				}
			}

		}

		// Remove dead objects from object list
		listObjects.remove_if([](sObject& o) {return o.bRemove; });

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

	if (!game.ConstructConsole(300, 160, 4, 4)) {
		std::wcerr << L"Console construction failed!" << endl;
		return -1;
	}

	game.Start();
	return 0;
}