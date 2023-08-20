#include <fstream>
#include <string>
#include <algorithm>
#include <queue>
#include <random>

using namespace std;

std::random_device rd;
std::mt19937 gen(rd());


struct vec3d
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
};

struct customChar
{
	vector<float> lines;
};

//each element of the following arrays consists of multiple integers
//where each 4 integers represent the x1,y1,x2,y2 in this order

//This represents all the letters needed to write "score =" represeneted as lines
customChar scoreLetters[] =
{
	{{2,1,4,1,1,2,1,2,2,3,3,3,4,4,4,4,1,5,3,5}},//s
	{{2,1,4,1,1,2,1,4,2,5,4,5}},//c
	{{2,1,3,1,1,2,1,4,2,5,3,5,4,2,4,4}},//o
	{{1,1,1,5,1,1,3,1,3,1,3,2,1,3,2,3,3,4,3,5 }},//r
	{{1,1,3,1,1,1,1,5,1,3,3,3,1,5,3,5}},//e
	{{1,3,4,3,1,5,4,5}}//=
};

//This represents all the letters needed to write "high" represeneted as lines
customChar highScoreLetters[] =
{
	//{{1,1,1,5,1,3,3,3,3,1,3,5}},
	{{1,0,1,5,1,2,3,2,3,2,3,5}},//h
	{{1,0,1,0,1,2,1,5}},//i
	{{1,1,3,1,3,1,3,5,3,5,1,5,1,3,3,3,1,1,1,3}},//g
	{{1,0,1,5,1,2,3,2,3,2,3,5}},//h
	{ {2,1,4,1,1,2,1,2,2,3,3,3,4,4,4,4,1,5,3,5}},//s
	{{2,1,4,1,1,2,1,4,2,5,4,5}},//c
	{{2,1,3,1,1,2,1,4,2,5,3,5,4,2,4,4}},//o
	{{1,1,1,5,1,1,3,1,3,1,3,2,1,3,2,3,3,4,3,5 }},//r
	{{1,1,3,1,1,1,1,5,1,3,3,3,1,5,3,5}},//e
	{{1,3,4,3,1,5,4,5}}//=
};

//Letters for "ammo = "
customChar ammoLetters[] =
{
	{{2,1,2,1,1,2,1,5,3,2,3,5,1,3,3,3}},//A
	{{1,2,1,5,2,1,2,1,3,2,3,2,4,1,4,1,5,2,5,5}},//m
	{{1,2,1,5,2,1,2,1,3,2,3,2,4,1,4,1,5,2,5,5}},//m
	{{2,1,3,1,1,2,1,4,2,5,3,5,4,2,4,4}},//o
	{{1,3,4,3,1,5,4,5}}//=
};

//This represents all the numbers from 0-9 represeneted as lines
customChar neededNumbers[] =
{
	{{1,1,1,5,1,5,3,5,1,1,3,1,3,1,3,5}},
	{{1,1,2,1,2,1,2,5,1,5,3,5}},
	{{1,1,3,1,3,1,3,3,1,3,3,3,1,3,1,5,1,5,3,5}},
	{{1,1,3,1,3,1,3,5,3,3,1,3,3,5,1,5}},
	{{1,1,1,3,1,3,3,3,3,1,3,5}},
	{{1,1,3,1,1,1,1,3,1,3,3,3,3,3,3,5,3,5,1,5}},
	{{1,1,3,1,1,1,1,5,1,5,3,5,1,3,3,3,3,3,3,5}},
	{{1,1,3,1,3,1,3,5}},
	{{1,1,1,5,1,5,3,5,1,1,3,1,3,1,3,5,1,3,3,3}},
	{{1,1,3,1,1,1,1,3,1,3,3,3,3,1,3,5}}
};

struct vec2d
{
	float u = 0;
	float v = 0;
	float w = 0;
};

struct triangle
{
	vec3d p[3];
	vec2d t[3];

	wchar_t sym;
	short col;
};

struct mesh
{
	vector<triangle> tris;

	bool destroyable = true;
	bool destroyed = false;

};
struct mat4x4
{
	float m[4][4] = { 0 };
};

vec3d Vector_Add(vec3d& v1, vec3d& v2)
{
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3d Vector_Sub(vec3d& v1, vec3d& v2)
{
	return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

vec3d Vector_Mul(vec3d& v1, float k)
{
	return { v1.x * k, v1.y * k, v1.z * k };
}

vec3d Vector_Div(vec3d& v1, float k)
{
	return { v1.x / k, v1.y / k, v1.z / k };
}

float Vector_DotProduct(vec3d& v1, vec3d& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float Vector_Length(vec3d& v)
{
	return sqrtf(Vector_DotProduct(v, v));
}

vec3d Vector_Normalise(vec3d& v)
{
	float l = Vector_Length(v);
	return { v.x / l, v.y / l, v.z / l };
}

vec3d Vector_CrossProduct(vec3d& v1, vec3d& v2)
{
	vec3d v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}

vec3d Vector_IntersectPlane(vec3d& plane_p, vec3d& plane_n, vec3d& lineStart, vec3d& lineEnd, float& t)
{
	plane_n = Vector_Normalise(plane_n);
	float plane_d = -Vector_DotProduct(plane_n, plane_p);
	float ad = Vector_DotProduct(lineStart, plane_n);
	float bd = Vector_DotProduct(lineEnd, plane_n);
	t = (-plane_d - ad) / (bd - ad);
	vec3d lineStartToEnd = Vector_Sub(lineEnd, lineStart);
	vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
	return Vector_Add(lineStart, lineToIntersect);
}

vec3d Vector_MultiplyVector(vec3d& v1, vec3d& v2)
{
	vec3d v;
	v = { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
	return v;
}

vec3d Matrix_MultiplyVector(mat4x4& m, vec3d& i)
{
	vec3d v;
	v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
	v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
	v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
	v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
	return v;
}

mat4x4 Matrix_MakeIdentity()
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeRotationX(float fAngleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[1][2] = sinf(fAngleRad);
	matrix.m[2][1] = -sinf(fAngleRad);
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeRotationY(float fAngleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][2] = sinf(fAngleRad);
	matrix.m[2][0] = -sinf(fAngleRad);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeRotationZ(float fAngleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][1] = sinf(fAngleRad);
	matrix.m[1][0] = -sinf(fAngleRad);
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeTranslation(float x, float y, float z)
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}

mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	mat4x4 matrix;
	matrix.m[0][0] = fAspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = fFar / (fFar - fNear);
	matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}

mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2)
{
	mat4x4 matrix;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
	return matrix;
}

mat4x4 Matrix_PointAt(vec3d& pos, vec3d& target, vec3d& up)
{
	// Calculate new forward direction
	vec3d newForward = Vector_Sub(target, pos);
	newForward = Vector_Normalise(newForward);

	// Calculate new Up direction
	vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
	vec3d newUp = Vector_Sub(up, a);
	newUp = Vector_Normalise(newUp);

	// New Right direction is easy, its just cross product
	vec3d newRight = Vector_CrossProduct(newUp, newForward);

	// Construct Dimensioning and Translation Matrix	
	mat4x4 matrix;
	matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
	return matrix;

}

mat4x4 Matrix_RotationPlayer(float fAngleRad, float pivotX, float pivotY, float pivotZ)
{
	mat4x4 matrix;

	// Calculate sine and cosine of the angle
	float cosAngle = cosf(fAngleRad);
	float sinAngle = sinf(fAngleRad);

	// Rotation matrix components
	matrix.m[0][0] = cosAngle;
	matrix.m[0][2] = sinAngle;
	matrix.m[2][0] = -sinAngle;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosAngle;
	matrix.m[3][3] = 1.0f;

	// Translation components
	matrix.m[0][3] = pivotX - (pivotX * cosAngle) + (pivotZ * sinAngle);
	matrix.m[1][3] = pivotY;
	matrix.m[2][3] = pivotZ - (pivotX * sinAngle) - (pivotZ * cosAngle);

	return matrix;
}

mat4x4 Matrix_QuickInverse(mat4x4& m) // Only for Rotation/Translation Matrices
{
	mat4x4 matrix;
	matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}
struct collisionBox {
	float mnx = 1e9, mny = 1e9, mnz = 1e9, mxx = -1e9, mxy = -1e9, mxz = -1e9;
};

bool Compare(triangle a, triangle b)
{
	float midpointa = (a.p[0].z + a.p[1].z + a.p[2].z) / 3;
	float midpointb = (b.p[0].z + b.p[1].z + b.p[2].z) / 3;
	if (midpointa > midpointb)
		return true;
	else
		return false;
}

int random(float low, float high)
{
	std::uniform_int_distribution<> dist(low, high);
	return dist(gen);
}

mesh MeshCreate(string file_path, float scale)
{
	mesh temp;
	triangle temp_triangle;
	ifstream file(file_path);

	if (file.is_open()) {
		string line, num{};
		int c{};
		float x, y, z, current_num;
		vector<vec3d> points;

		while (getline(file, line)) {

			if (line[0] == 'v')
			{
				c = 0;
				for (int i = 2; i < line.length(); i++)
				{
					while (line[i] != ' ' && i < line.length())
					{
						num += line[i];
						i++;
					}

					current_num = stof(num);
					num = "";

					if (c == 0)
						x = current_num;
					if (c == 1)
						y = current_num;
					if (c == 2)
						z = current_num;
					c++;
				}
				points.push_back({ x * scale, y * scale, z * scale });
				c = 0;
			}

			else if (line[0] == 'f')
			{
				c = 0;
				for (int i = 2; i < line.length(); i++)
				{
					num = "";
					while (line[i] != ' ' && i < line.length())
					{
						num += line[i];
						i++;
					}
					int pos = stoi(num);
					num = "";

					if (c == 0)
					{
						temp_triangle.p[0].x = points[pos - 1].x;
						temp_triangle.p[0].y = points[pos - 1].y;
						temp_triangle.p[0].z = points[pos - 1].z;
					}

					if (c == 1)
					{
						temp_triangle.p[1].x = points[pos - 1].x;
						temp_triangle.p[1].y = points[pos - 1].y;
						temp_triangle.p[1].z = points[pos - 1].z;
					}

					if (c == 2)
					{
						temp_triangle.p[2].x = points[pos - 1].x;
						temp_triangle.p[2].y = points[pos - 1].y;
						temp_triangle.p[2].z = points[pos - 1].z;
					}
					c++;


				}
				temp.tris.push_back(temp_triangle);
			}

		}

		// Close the file
		file.close();
	}
	else {
		cout << "Failed to open the file." << endl;
	}

	return temp;
}

void MeshMove(mesh& object_mesh, vec3d& velocity)
{
	for (auto& tri : object_mesh.tris)
	{
		tri.p[0] = Vector_Add(tri.p[0], velocity);
		tri.p[1] = Vector_Add(tri.p[1], velocity);
		tri.p[2] = Vector_Add(tri.p[2], velocity);

	}
}

//Gives The Object an Inital Spawn Position and a Direction To Move In
void SpawnObject(mesh object_mesh, float x_offset, float y_offset, float z_offset, vec3d direction, deque<pair<mesh, vec3d>>& objects)
{
	vec3d vObjectTranslate = { x_offset, y_offset, z_offset };
	MeshMove(object_mesh, vObjectTranslate);
	objects.push_back({ object_mesh, direction });
}

int GetHighScore(int score)
{
	fstream file;
	string line;
	int highScore{};
	file.open("HighScore.txt", ios::in);


	if (file.is_open())
	{
		while (getline(file, line))
		{
			highScore = stoi(line);

		}
		file.close();

		if (score > highScore)
		{
			file.open("HighScore.txt", ios::out);

			file << score;

			file.close();

			return score;

		}

		return highScore;
	}
	else
	{
		file.close();  // Close the failed open attempt
		file.open("HighScore.txt", ios::out);  // Create the file
		file << score;  // Write "0" to the empty file
		file.close();
	}

}

int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle& in_tri, triangle& out_tri1, triangle& out_tri2)
{
	// Make sure plane normal is indeed normal
	plane_n = Vector_Normalise(plane_n);

	// Return signed shortest distance from point to plane, plane normal must be normalised
	auto dist = [&](vec3d& p)
	{
		vec3d n = Vector_Normalise(p);
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
	};

	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	vec3d* inside_points[3];  int nInsidePointCount = 0;
	vec3d* outside_points[3]; int nOutsidePointCount = 0;
	vec2d* inside_tex[3]; int nInsideTexCount = 0;
	vec2d* outside_tex[3]; int nOutsideTexCount = 0;


	// Get signed distance of each point in triangle to plane
	float d0 = dist(in_tri.p[0]);
	float d1 = dist(in_tri.p[1]);
	float d2 = dist(in_tri.p[2]);

	if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; inside_tex[nInsideTexCount++] = &in_tri.t[0]; }
	else {
		outside_points[nOutsidePointCount++] = &in_tri.p[0]; outside_tex[nOutsideTexCount++] = &in_tri.t[0];
	}
	if (d1 >= 0) {
		inside_points[nInsidePointCount++] = &in_tri.p[1]; inside_tex[nInsideTexCount++] = &in_tri.t[1];
	}
	else {
		outside_points[nOutsidePointCount++] = &in_tri.p[1];  outside_tex[nOutsideTexCount++] = &in_tri.t[1];
	}
	if (d2 >= 0) {
		inside_points[nInsidePointCount++] = &in_tri.p[2]; inside_tex[nInsideTexCount++] = &in_tri.t[2];
	}
	else {
		outside_points[nOutsidePointCount++] = &in_tri.p[2];  outside_tex[nOutsideTexCount++] = &in_tri.t[2];
	}

	// Now classify triangle points, and break the input triangle into 
	// smaller output triangles if required. There are four possible
	// outcomes...

	if (nInsidePointCount == 0)
	{
		// All points lie on the outside of plane, so clip whole triangle
		// It ceases to exist

		return 0; // No returned triangles are valid
	}

	if (nInsidePointCount == 3)
	{
		// All points lie on the inside of plane, so do nothing
		// and allow the triangle to simply pass through
		out_tri1 = in_tri;

		return 1; // Just the one returned original triangle is valid
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		// Triangle should be clipped. As two points lie outside
		// the plane, the triangle simply becomes a smaller triangle

		// Copy appearance info to new triangle
		out_tri1.col = in_tri.col;
		out_tri1.sym = in_tri.sym;

		// The inside point is valid, so keep that...
		out_tri1.p[0] = *inside_points[0];
		out_tri1.t[0] = *inside_tex[0];

		// but the two new points are at the locations where the 
		// original sides of the triangle (lines) intersect with the plane
		float t;
		out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		out_tri1.t[1].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1.t[1].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1.t[1].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

		out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], t);
		out_tri1.t[2].u = t * (outside_tex[1]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1.t[2].v = t * (outside_tex[1]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1.t[2].w = t * (outside_tex[1]->w - inside_tex[0]->w) + inside_tex[0]->w;

		return 1; // Return the newly formed single triangle
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		// Triangle should be clipped. As two points lie inside the plane,
		// the clipped triangle becomes a "quad". Fortunately, we can
		// represent a quad with two new triangles

		// Copy appearance info to new triangles
		out_tri1.col = in_tri.col;
		out_tri1.sym = in_tri.sym;

		out_tri2.col = in_tri.col;
		out_tri2.sym = in_tri.sym;

		// The first triangle consists of the two inside points and a new
		// point determined by the location where one side of the triangle
		// intersects with the plane
		out_tri1.p[0] = *inside_points[0];
		out_tri1.p[1] = *inside_points[1];
		out_tri1.t[0] = *inside_tex[0];
		out_tri1.t[1] = *inside_tex[1];

		float t;
		out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		out_tri1.t[2].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1.t[2].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1.t[2].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

		// The second triangle is composed of one of he inside points, a
		// new point determined by the intersection of the other side of the 
		// triangle and the plane, and the newly created point above
		out_tri2.p[0] = *inside_points[1];
		out_tri2.t[0] = *inside_tex[1];
		out_tri2.p[1] = out_tri1.p[2];
		out_tri2.t[1] = out_tri1.t[2];
		out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
		out_tri2.t[2].u = t * (outside_tex[0]->u - inside_tex[1]->u) + inside_tex[1]->u;
		out_tri2.t[2].v = t * (outside_tex[0]->v - inside_tex[1]->v) + inside_tex[1]->v;
		out_tri2.t[2].w = t * (outside_tex[0]->w - inside_tex[1]->w) + inside_tex[1]->w;
		return 2; // Return two newly formed triangles which form a quad
	}
}

CHAR_INFO GetColour(float lum)
{
	short bg_col, fg_col;
	wchar_t sym;
	int pixel_bw = (int)(13.0f * lum);
	switch (pixel_bw)
	{
	case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

	case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
	case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
	case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
	case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

	case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
	case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
	case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
	case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

	case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
	case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
	case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
	case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
	default:
		bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
	}

	CHAR_INFO c;
	c.Attributes = bg_col | fg_col;
	c.Char.UnicodeChar = sym;
	return c;
}

collisionBox getCollisionOfMesh(mesh& m) {
	collisionBox ret;
	for (auto& tri : m.tris) {
		triangle triTransformed;
		triTransformed.p[0] = tri.p[0];
		triTransformed.p[1] = tri.p[1];
		triTransformed.p[2] = tri.p[2];
		ret.mnx = min(ret.mnx, triTransformed.p[0].x);
		ret.mnx = min(ret.mnx, triTransformed.p[1].x);
		ret.mnx = min(ret.mnx, triTransformed.p[2].x);
		ret.mny = min(ret.mny, triTransformed.p[0].y);
		ret.mny = min(ret.mny, triTransformed.p[1].y);
		ret.mny = min(ret.mny, triTransformed.p[2].y);
		ret.mnz = min(ret.mnz, triTransformed.p[0].z);
		ret.mnz = min(ret.mnz, triTransformed.p[1].z);
		ret.mnz = min(ret.mnz, triTransformed.p[2].z);
		ret.mxx = max(ret.mxx, triTransformed.p[0].x);
		ret.mxx = max(ret.mxx, triTransformed.p[1].x);
		ret.mxx = max(ret.mxx, triTransformed.p[2].x);
		ret.mxy = max(ret.mxy, triTransformed.p[0].y);
		ret.mxy = max(ret.mxy, triTransformed.p[1].y);
		ret.mxy = max(ret.mxy, triTransformed.p[2].y);
		ret.mxz = max(ret.mxz, triTransformed.p[0].z);
		ret.mxz = max(ret.mxz, triTransformed.p[1].z);
		ret.mxz = max(ret.mxz, triTransformed.p[2].z);
	}
	return ret;
}

bool bIsCollided(collisionBox& b1, collisionBox& b2) {
	return b1.mnx <= b2.mxx && b2.mnx <= b1.mxx &&
		b1.mny <= b2.mxy && b2.mny <= b1.mxy &&
		b1.mnz <= b2.mxz && b2.mnz <= b1.mxz;
}

