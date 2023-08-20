#include "olcConsoleGameEngine.h"
#include "utilities.h"
#include <filesystem>
using namespace std;

class olcEngine3D : public olcConsoleGameEngine
{
public:
	olcEngine3D()
	{
		m_sAppName = L"Asteroid Shooter";
	}
	
private:
	filesystem::path bulletPath;

	mesh meshCube;
	mesh meshMeteor;
	mesh player;
	mesh boxMesh;
	mesh playerMesh;
	mat4x4 matProj;
	mat4x4 matIdentity = Matrix_MakeIdentity();

	vec3d vCamera{ 0.0f, 0.6f, 0.0f };
	vec3d vLookDir;
	vec3d vPrevPos{ 0.0f, 0.0f, 0.0f };

	float MAX_X = 50.0f;
	float MIN_X = -50.0f;
	float MAX_Y = 50.0f;
	float MIN_Y = -50.0f;
	float meteorTimer = 0.0f;
	float ammoBoxTimer = 0.0f;
	float meteorRespawnTimer{ 1.0f };
	float ammoBoxRespawnTimer{ 15.0f };

	int highScore;
	int score{};
	int HITSCORE = 10;
	int ammo{ 20 };

	deque<pair<mesh, vec3d>> bullets;
	deque<pair<mesh, vec3d>> meteors;
	deque<pair<mesh, vec3d>> ammoBoxes;

	vector<triangle> vecTrianglesToRaster;
	vector<triangle> vecTrisClipped;

	float fYaw;
	float fTheta;

	vector<int> GetDigits(int score)
	{
		vector<int> digits;
		do
		{
			int currentDigit = score % 10;
			score /= 10;
			digits.push_back(currentDigit);
		} while (score);

		return digits;
	}

	void DrawString(string str, float& x_offset, float y_offset)
	{
		int arr[4];
		if (str == "score =")
		{
			for (const auto& character : scoreLetters)
			{
				int count = 0;
				for (const auto& point : character.lines)
				{
					if (count % 4 == 0 && count != 0)
					{
						DrawLine(arr[0] + x_offset, arr[1] + y_offset, arr[2] + x_offset, arr[3] + y_offset);
					}
					arr[count % 4] = point;
					count++;
				}
				DrawLine(arr[0] + x_offset, arr[1] + y_offset, arr[2] + x_offset, arr[3] + y_offset);
				x_offset += 6;
			}
		}

		else if (str == "high score =")
		{
			for (const auto& character : highScoreLetters)
			{
				int count = 0;
				for (const auto& point : character.lines)
				{
					if (count % 4 == 0 && count != 0)
					{
						DrawLine(arr[0] + x_offset, arr[1] + y_offset, arr[2] + x_offset, arr[3] + y_offset);
					}
					arr[count % 4] = point;
					count++;
				}
				DrawLine(arr[0] + x_offset, arr[1] + y_offset, arr[2] + x_offset, arr[3] + y_offset);
				x_offset += 6;
			}
		}

		else if (str == "ammo =")
		{
			for (const auto& character : ammoLetters)
			{
				int count = 0;
				for (const auto& point : character.lines)
				{
					if (count % 4 == 0 && count != 0)
					{
						DrawLine(arr[0] + x_offset, arr[1] + y_offset, arr[2] + x_offset, arr[3] + y_offset);
					}
					arr[count % 4] = point;
					count++;
				}
				DrawLine(arr[0] + x_offset, arr[1] + y_offset, arr[2] + x_offset, arr[3] + y_offset);
				x_offset += 6;
			}
		}
	}

	void DrawNumber(int number, float x_offset, float y_offset)
	{
		int arr[4];
		vector<int> digits = GetDigits(number);

		for (int i = digits.size() - 1; i >= 0; i--)
		{
			int count = 0;
			for (const auto& point : neededNumbers[digits[i]].lines)
			{
				if (count % 4 == 0 && count != 0)
				{
					DrawLine(arr[0] + x_offset, arr[1] + y_offset, arr[2] + x_offset, arr[3] + y_offset);
				}
				arr[count % 4] = point;
				count++;
			}

			DrawLine(arr[0] + x_offset, arr[1] + y_offset, arr[2] + x_offset, arr[3] + y_offset);
			x_offset += 5;
		}
	}

	void DrawInfo(int score, int high_score, int ammo)
	{
		float xOffset{};
		int test{ 0 };
		int yOffset{ 0 };

		DrawString("high score =", xOffset, yOffset);
		DrawNumber(high_score, xOffset, yOffset);
		yOffset += 8;
		xOffset = 0;

		DrawString("score =", xOffset, yOffset);
		DrawNumber(score, xOffset, yOffset);
		yOffset += 8;
		xOffset = 0;

		DrawString("ammo =", xOffset, yOffset);
		DrawNumber(ammo, xOffset, yOffset);
	}

	void objectProject(mat4x4& matWorld, mat4x4& matView, mat4x4& matProj, mat4x4& matCameraRot, mesh& objectMesh,
		vector<triangle>& vecTrianglesToRaster, bool bPlayer, vec3d& vCamera, bool bLight)
	{
		if (!bPlayer)
		{

			for (auto& tri : objectMesh.tris)
			{
				triangle triTransformed, triViewed, triProjected;

				triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
				triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
				triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

				//Calculate Triangle Normal
				vec3d N, line1, line2;

				//Get Lines of Triangle
				line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
				line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

				//Cross product to get normal
				N = Vector_CrossProduct(line1, line2);

				//Normalize the nomral
				N = Vector_Normalise(N);

				vec3d  vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

				if (Vector_DotProduct(N, vCameraRay) < 0.0f)
				{
					//Light
					vec3d light_direction = { 0,1, -1 };
					light_direction = Vector_Normalise(light_direction);

					float dp;
					if (bLight)
						dp = max(0.3f, Vector_DotProduct(light_direction, N));
					else
						dp = 0.8;

					CHAR_INFO c = GetColour(dp);
					triTransformed.col = c.Attributes;
					triTransformed.sym = c.Char.UnicodeChar;

					triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
					triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
					triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
					triViewed.sym = triTransformed.sym;
					triViewed.col = triTransformed.col;

					int nClippedTriangles = 0;
					triangle clipped[2];
					nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.01 }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

					for (int n = 0; n < nClippedTriangles; n++)
					{
						triViewed = clipped[n];

						// Project triangles from 3D --> 2D
						triProjected.p[0] = Matrix_MultiplyVector(matProj, triViewed.p[0]);
						triProjected.p[1] = Matrix_MultiplyVector(matProj, triViewed.p[1]);
						triProjected.p[2] = Matrix_MultiplyVector(matProj, triViewed.p[2]);
						triProjected.col = clipped[n].col;
						triProjected.sym = clipped[n].sym;

						//Normalize The Projected Vector
						triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
						triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
						triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

						// Offset so you can see it
						vec3d vOffSetView = { 1,1,0 };
						triProjected.p[0] = Vector_Add(triProjected.p[0], vOffSetView);
						triProjected.p[1] = Vector_Add(triProjected.p[1], vOffSetView);
						triProjected.p[2] = Vector_Add(triProjected.p[2], vOffSetView);

						triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
						triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
						triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
						triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
						triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
						triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

						//Store triangle for sorting
						vecTrianglesToRaster.push_back(triProjected);
					}
				}
			}
		}

		else
		{
			for (auto& tri : objectMesh.tris)
			{
				triangle triTranslated, triProjected, triViewed;
				vec3d vPlayerCameraTranslate;
				vPlayerCameraTranslate = { vCamera.x , vCamera.y , vCamera.z };

				triTranslated.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
				triTranslated.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
				triTranslated.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

				triViewed.p[0] = Vector_Add(triTranslated.p[0], vPlayerCameraTranslate);
				triViewed.p[1] = Vector_Add(triTranslated.p[1], vPlayerCameraTranslate);
				triViewed.p[2] = Vector_Add(triTranslated.p[2], vPlayerCameraTranslate);

				vec3d N, line1, line2;

				//Get Lines of Triangle
				line1 = Vector_Sub(triViewed.p[1], triViewed.p[0]);
				line2 = Vector_Sub(triViewed.p[2], triViewed.p[0]);

				//Cross product to get normal
				N = Vector_CrossProduct(line1, line2);

				//Normalize the nomral
				N = Vector_Normalise(N);

				vec3d  vCameraRay = Vector_Sub(triViewed.p[0], vCamera);

				if (Vector_DotProduct(N, vCameraRay) < 0.0f)
				{
					vec3d light_direction = { 0.0f, -1.0f, -1.0f };
					light_direction = Matrix_MultiplyVector(matCameraRot, light_direction);
					light_direction = Vector_Normalise(light_direction);

					float dp;
					dp = max(0.3f, Vector_DotProduct(light_direction, N));

					CHAR_INFO c = GetColour(dp);
					triTranslated.col = c.Attributes;
					triTranslated.sym = c.Char.UnicodeChar;

					//Project The Vectors From 3D to 2D
					triProjected.p[0] = Matrix_MultiplyVector(matProj, triTranslated.p[0]);
					triProjected.p[1] = Matrix_MultiplyVector(matProj, triTranslated.p[1]);
					triProjected.p[2] = Matrix_MultiplyVector(matProj, triTranslated.p[2]);
					triProjected.col = triTranslated.col;
					triProjected.sym = triTranslated.sym;

					//Normalize The Projected Vector
					triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
					triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
					triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

					vec3d vOffSetView = { 1,1,0 };
					triProjected.p[0] = Vector_Add(triProjected.p[0], vOffSetView);
					triProjected.p[1] = Vector_Add(triProjected.p[1], vOffSetView);
					triProjected.p[2] = Vector_Add(triProjected.p[2], vOffSetView);

					triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

					//Put The Triangles to be Rendered
					vecTrianglesToRaster.push_back(triProjected);
				}
			}
		}
	}

	//This Function Takes Input From The Player And Does an Action Depending On The Input
	void DoPlayerActions(const float& fElapsedTime, vec3d& vRight, vec3d& vLeft, vec3d& vForward, mat4x4 matCameraRot)
	{
		//Moves The Player Up
		if (GetKey(VK_UP).bHeld && vCamera.y < MAX_Y) {
			vCamera.y += 8.0f * fElapsedTime;
			vec3d vMovUp = { 0.0f, 8.0f * fElapsedTime, 0.0f };
			MeshMove(player, vMovUp);
		}

		//Moves The Player Down
		if (GetKey(VK_DOWN).bHeld && vCamera.y > MIN_Y) {
			vCamera.y -= 8.0f * fElapsedTime;
			vec3d vMovDown = { 0.0f, -8.0f * fElapsedTime, 0.0f };
			MeshMove(player, vMovDown);
		}

		//Moves The Player Right
		if (GetKey(VK_RIGHT).bHeld)
		{
			vec3d vTest = Vector_Add(vCamera, vRight);
			if ((vTest.x < MAX_X && vTest.x > MIN_X))
			{
				vCamera = Vector_Add(vCamera, vRight);
				MeshMove(player, vRight);
			}
		}

		//Moves The Player Left
		if (GetKey(VK_LEFT).bHeld)
		{
			vec3d vTest = Vector_Add(vCamera, vLeft);
			if ((vTest.x < MAX_X && vTest.x > MIN_X))
			{
				vCamera = Vector_Add(vCamera, vLeft);
				MeshMove(player, vLeft);
			}
		}

		//Moves The Player Forward
		if (GetKey(L'W').bHeld)
		{
			vec3d vTest = Vector_Add(vCamera, vForward);
			if ((vTest.x < MAX_X && vTest.x > MIN_X))
			{
				vCamera = Vector_Add(vCamera, vForward);
				MeshMove(player, vForward);
			}
		}

		//Moves The Player Backwards
		if (GetKey(L'S').bHeld)
		{
			vec3d vBack = { -vForward.x, -vForward.y, -vForward.z };
			vec3d vTest = Vector_Add(vCamera, vBack);
			if ((vTest.x < MAX_X && vTest.x > MIN_X))
			{
				vCamera = Vector_Sub(vCamera, vForward);
				MeshMove(player, vBack);
			}
		}

		//Moves The Player Left
		if (GetKey(L'A').bHeld && !GetKey(L'D').bHeld) {
			vec3d vOld_LookDir = Vector_Sub(player.tris[4].p[0], vCamera);
			fYaw -= 1.5f * fElapsedTime;

			matCameraRot = Matrix_MakeRotationY(fYaw);
			vLookDir = Matrix_MultiplyVector(matCameraRot, vLookDir);
			vLookDir = Vector_Mul(vLookDir, sqrtf(vOld_LookDir.x * vOld_LookDir.x + vOld_LookDir.y * vOld_LookDir.y + vOld_LookDir.z * vOld_LookDir.z));

			vec3d RotDir = Vector_Sub(vLookDir, vOld_LookDir);

			vOld_LookDir = Vector_Normalise(vOld_LookDir);
			vLookDir = Vector_Normalise(vLookDir);

			MeshMove(player, RotDir);

		}

		//Moves The Player Right
		if (GetKey(L'D').bHeld && !GetKey(L'A').bHeld) {
			vec3d vOld_LookDir = Vector_Sub(player.tris[4].p[0], vCamera);
			fYaw += 1.5f * fElapsedTime;

			matCameraRot = Matrix_MakeRotationY(fYaw);
			vLookDir = Matrix_MultiplyVector(matCameraRot, vLookDir);
			vLookDir = Vector_Mul(vLookDir, sqrtf(vOld_LookDir.x * vOld_LookDir.x + vOld_LookDir.y * vOld_LookDir.y + vOld_LookDir.z * vOld_LookDir.z));

			vec3d RotDir = Vector_Sub(vLookDir, vOld_LookDir);

			vOld_LookDir = Vector_Normalise(vOld_LookDir);
			vLookDir = Vector_Normalise(vLookDir);

			MeshMove(player, RotDir);
		}

		//Shoots Bullets
		if (GetKey(VK_SPACE).bPressed)
		{
			if (ammo) {
				ammo--;
				vec3d vShootDir = { 0.0f, 0.0f, 1.0f };
				vShootDir = Matrix_MultiplyVector(matCameraRot, vShootDir);
				mesh bullet = MeshCreate(bulletPath.string(), 0.5f);

				vec3d  bulletDirection = vShootDir;

				vec3d vSpawnPoint = { vCamera.x, vCamera.y, vCamera.z };

				for (auto& triangle : bullet.tris)
				{
					triangle.p[0] = Vector_Add(triangle.p[0], vSpawnPoint);
					triangle.p[1] = Vector_Add(triangle.p[1], vSpawnPoint);
					triangle.p[2] = Vector_Add(triangle.p[2], vSpawnPoint);
				}
				bullets.push_back({ bullet,bulletDirection });
			}
		}
	}

	void GameReset()
	{
		vecTrianglesToRaster.clear();
		vCamera = { 0.0f, 0.6f, 0.0f };
		player = playerMesh;
		highScore = GetHighScore(score);
		score = 0;
		ammo = 20;
		ammoBoxTimer = 0;
		meteorTimer = 0;
		meteors.clear();
		bullets.clear();
		ammoBoxes.clear();
	}

	//Takes an Object and Moves It in It's Direction Vector
	//Also Gives us How Many Objects Need to be Popped off For Getting Too Far Away From The Player
	void MoveObject(float fElapsedTime, deque<pair<mesh, vec3d>>& objects, vec3d vCamera, int& numberPoppedObjects, float speed)
	{
		for (int i = 0; i < objects.size(); i++)
		{
			//Moves the Object Through the World
			vec3d vVelocity = Vector_Mul(objects[i].second, speed * fElapsedTime);
			mat4x4 matBulletTranslate = Matrix_MakeTranslation(vVelocity.x, vVelocity.y, vVelocity.z);

			//Calculate the Distance Between the Object and The Player
			float dist = sqrtf((vCamera.x - objects[i].first.tris[0].p[0].x) * (vCamera.x - objects[i].first.tris[0].p[0].x) +
				(vCamera.y - objects[i].first.tris[0].p[0].y) * (vCamera.y - objects[i].first.tris[0].p[0].y)
				+ (vCamera.z - objects[i].first.tris[0].p[0].z) * (vCamera.z - objects[i].first.tris[0].p[0].z));

			for (auto& tri : objects[i].first.tris)
			{
				tri.p[0] = Matrix_MultiplyVector(matBulletTranslate, tri.p[0]);
				tri.p[1] = Matrix_MultiplyVector(matBulletTranslate, tri.p[1]);
				tri.p[2] = Matrix_MultiplyVector(matBulletTranslate, tri.p[2]);
			}
			if (dist > 200.0f)
				++numberPoppedObjects;
		}
	}

public:
	bool OnUserCreate() override
	{
		filesystem::path executablePath = std::filesystem::path(std::filesystem::current_path());

		filesystem::path playerPath = executablePath / "assets//VideoShip.obj";
		filesystem::path meteorPath = executablePath / "assets//Asteroid.obj";
		filesystem::path meshBox = executablePath / "assets//box.obj";
		bulletPath = executablePath / "assets//shot2.obj";

		player = MeshCreate(playerPath.string(), 0.1f);
		meshMeteor = MeshCreate(meteorPath.string(), 1.0f);
		boxMesh = MeshCreate(meshBox.string(), 1.0f);

		//This Loop Makes The Player Centered In The Screen
		for (auto& tri : player.tris)
		{
			tri.p[0].z += 0.8;
			tri.p[1].z += 0.8;
			tri.p[2].z += 0.8;

			tri.p[0].y += 0.3;
			tri.p[1].y += 0.3;
			tri.p[2].y += 0.3;
		}

		playerMesh = player;

		matProj = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.01f, 1000.0f);

		highScore = GetHighScore(0);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		meteorTimer += fElapsedTime;
		ammoBoxTimer += fElapsedTime;

		//Vectors for Storing triangles
		vecTrianglesToRaster.clear();
		vecTrisClipped.clear();

		//Vectors For Moving The Player
		vec3d vUp = { 0.0f, -1.0f, 0.0f };
		vec3d vLeftDir = Vector_CrossProduct(vLookDir, vUp);
		vec3d vRightDir = Vector_CrossProduct(vUp, vLookDir);

		vec3d vForward = Vector_Mul(vLookDir, 8.0f * fElapsedTime);
		vec3d vRight = Vector_Mul(vRightDir, 8.0f * fElapsedTime);
		vec3d vLeft = Vector_Mul(vLeftDir, 8.0f * fElapsedTime);

		mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);

		DoPlayerActions(fElapsedTime, vRight, vLeft, vForward, matCameraRot);

		// Clear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		mat4x4 matWorld;
		matWorld = Matrix_MakeIdentity();

		vLookDir = { 0, 0, 1 };
		vec3d vTarget = { 0, 0, -1 };
		matCameraRot = Matrix_MakeRotationY(fYaw);

		vLookDir = Matrix_MultiplyVector(matCameraRot, vLookDir);
		vTarget = Vector_Add(vCamera, vLookDir);
		mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

		mat4x4 matView = Matrix_QuickInverse(matCamera);

		//Spawns Meteors
		if (meteorTimer > meteorRespawnTimer)
		{
			// the randomization works but i need to fix it so that they keep spawning in front of you
			float randX = random(vCamera.x - 5.0f, vCamera.x + 5.0f);
			float randY = random(vCamera.y - 5.0f, vCamera.y + 5.0f);
			float randZ = vCamera.z + 30.0f;

			SpawnObject(meshMeteor, randX, randY, randZ, { 0.0f, 0.0f, -1.0f }, meteors);
			meteorTimer = 0.0f;
		}

		//Spawns Ammo Boxs
		if (ammoBoxTimer > ammoBoxRespawnTimer)
		{
			if (ammoBoxes.size() == 0)
			{
				float randX = random(vCamera.x - 5.0f, vCamera.x + 5.0f);
				float randY = random(vCamera.y - 5.0f, vCamera.y + 5.0f);
				float randZ = vCamera.z + 50.0f;

				SpawnObject(boxMesh, randX, randY, randZ, { 0.0f,0.0f,0.0f }, ammoBoxes);
			}
			ammoBoxTimer = 0.0f;
		}

		//Moves The Bullets Spawned By The Player Every Frame
		int bulletPopCounter{};
		MoveObject(fElapsedTime, bullets, vCamera, bulletPopCounter, 100.0f);

		//Moves The Meteors Every Frame
		int meteorPopCounter{};
		MoveObject(fElapsedTime, meteors, vCamera, meteorPopCounter, 20.0f);

		//resets the game if the player is destroyed
		if (player.destroyed)
		{
			GameReset();
		}

		//Loops Over all the Meteors to Destroy Them if They hit a Bullet'
		//Also Checks if the Player Hit a Meteor and Destroys The Player if Hit
		for (auto& mesh : meteors)
		{
			if (mesh.first.destroyed)
				continue;

			//Checks for Collision Between the Player and The Meteors
			collisionBox firstMeshBox = getCollisionOfMesh(player);
			collisionBox secondMeshBox = getCollisionOfMesh(mesh.first);

			bool bCollision = bIsCollided(firstMeshBox, secondMeshBox);

			if (bCollision)
			{
				mesh.first.destroyed = true;
				player.destroyed = true;
			}

			//Checks for Collision Between the Meteors and the Bullets
			for (auto& physicsObject : bullets)
			{
				collisionBox firstMeshBox = getCollisionOfMesh(mesh.first);
				collisionBox secondMeshBox = getCollisionOfMesh(physicsObject.first);

				bool bCollision = bIsCollided(firstMeshBox, secondMeshBox);

				if (bCollision)
				{
					if (mesh.first.destroyable)
					{
						mesh.first.destroyed = true;
						score += HITSCORE;
						break;
					}
				}
			}
		}

		//Loops Over The Ammo Boxes and Checks if The Player Hits Them
		for (auto& mesh : ammoBoxes)
		{
			if (mesh.first.destroyed)
				continue;

			//Checks for Collision Between the Player and The Ammo Boxes
			collisionBox firstMeshBox = getCollisionOfMesh(player);
			collisionBox secondMeshBox = getCollisionOfMesh(mesh.first);

			bool bCollision = bIsCollided(firstMeshBox, secondMeshBox);

			if (bCollision)
			{
				mesh.first.destroyed = true;
				ammo += 25;

				//Removes Used Ammo Box
				ammoBoxes.pop_front();
			}
		}

		//For Removing The Bullets When They Get too Far From The Player
		for (int i = 0; i < bulletPopCounter; ++i)
			bullets.pop_front();

		//For Removing The Meteors When They Get too Far From The Player
		for (int i = 0; i < meteorPopCounter; ++i)
			meteors.pop_front();

		//Adds Bullets to The Draw Vector
		for (int i = 0; i < bullets.size(); ++i)
		{
			if (!bullets[i].first.destroyed)
				objectProject(matIdentity, matView, matProj, matCameraRot, bullets[i].first, vecTrianglesToRaster, false, vCamera, true);

		}

		//Adds Meteors to The Draw Vector
		for (int i = 0; i < meteors.size(); ++i)
		{
			if (!meteors[i].first.destroyed)
				objectProject(matIdentity, matView, matProj, matCameraRot, meteors[i].first, vecTrianglesToRaster, false, vCamera, true);

		}

		//Adds Ammo Boxes To The Draw Vector
		for (int i = 0; i < ammoBoxes.size(); ++i)
		{
			if (!ammoBoxes[i].first.destroyed)
				objectProject(matIdentity, matView, matProj, matCameraRot, ammoBoxes[i].first, vecTrianglesToRaster, false, vCamera, true);
		}

		//Adds the Player to The Draw Vector
		if (!player.destroyed)
			objectProject(matIdentity, matIdentity, matProj, matCameraRot, playerMesh, vecTrianglesToRaster, true, vCamera, true);

		//Sorting The Triangles So That They are Rendered Correctely
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), Compare);

		//Clipping Triangles
		for (auto& tri : vecTrianglesToRaster)
		{
			int newTriangles = 1;
			triangle clipped[2];
			deque <triangle> trisToCheck;
			trisToCheck.push_back(tri);
			int add = 0;

			for (int i = 0; i < 4; i++)
			{

				while (newTriangles > 0)
				{
					triangle triangleTest = trisToCheck.front();
					trisToCheck.pop_front();
					newTriangles--;

					//Care about the -1
					switch (i)
					{
					case 0: add = Triangle_ClipAgainstPlane({ 0.0f, 0.0f ,0.0f }, { 0.0f, 1.0f, 0.0f }, triangleTest, clipped[0], clipped[1]); break;

					case 1:	add = Triangle_ClipAgainstPlane({ 0.0f, 0.0f ,0.0f }, { 1.0f, 0.0f, 0.0f }, triangleTest, clipped[0], clipped[1]); break;

					case 2:	add = Triangle_ClipAgainstPlane({ 0.0f, (float)ScreenHeight() - 1 ,0.0f }, { 0.0f, -1.0f, 0.0f }, triangleTest, clipped[0], clipped[1]); break;

					case 3:	add = Triangle_ClipAgainstPlane({ (float)ScreenWidth() - 1 , 0.0f ,0.0f }, { -1.0f, 0.0f, 0.0f }, triangleTest, clipped[0], clipped[1]); break;

					}

					for (int j = 0; j < add; j++)
						trisToCheck.push_back(clipped[j]);
				}
				newTriangles = trisToCheck.size();
			}

			for (auto& tri : trisToCheck)
			{
				vecTrisClipped.push_back(tri);
			}
		}

		//Drawing The Triangle
		for (auto& triProjected : vecTrisClipped)
		{
			FillTriangle(triProjected.p[0].x, triProjected.p[0].y,
				triProjected.p[1].x, triProjected.p[1].y,
				triProjected.p[2].x, triProjected.p[2].y,
				triProjected.sym, triProjected.col);
		}

		DrawInfo(score, highScore, ammo);
		return true;
	}

	bool OnUserDestroy() override
	{
		GetHighScore(score);
		return true;
	}

};




int main()
{
	olcEngine3D demo;
	if (demo.ConstructConsole(256, 240, 3, 3))
		demo.Start();
	return 0;
}