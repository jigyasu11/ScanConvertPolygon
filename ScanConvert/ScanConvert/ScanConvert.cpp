#include<iostream>
#include<vector>
#include<stdio.h>
#define NDEBUG
#include<assert.h>
#include<math.h>
#include<list>
#include<algorithm>
#include<gl/glut.h>
#include<gl/gl.h>
#include<time.h>
using namespace std;

#define ImageW 400
#define ImageH 400
float framebuffer[ImageH][ImageW][3];
enum state_input{
	input,
	clip
};
state_input s = input;
typedef enum direction {in, out};
int left_x, left_y;
int right_x, right_y;
int min_x, min_y, max_x, max_y;
int pCount = 0;
struct vertex
{
	int x, y;
};
struct edge
{
	vertex vertex1, vertex2;
};
struct AEL_node
{
	int maxY;
	float currentX, xIncr;
};
class polygon
{
	private:	
		vertex v1,v2;
		edge e;
		AEL_node A;
		vector<AEL_node>AEL;                 //To store create an AEL

	public:
		vector<vector<float>>color;		     //To store color codes per polygon
		vector<vector<vector<edge>>>table;   //To store the AET of all the polygons: Dimension = no of polygons(fixed=15) x no of scan lines(fixed=ImageH) x [no of edges]
		vector<vector<vertex>>AET_poly;      //To store the input vertices of all the polygons: Dimension = no of polygons x no of vertices 
		static bool sortAEL(AEL_node n1, AEL_node n2); 
		void fill_AET(int x, int y);
		void fill_table(vector<vertex>AET, int polyNum);
		direction getDir(int point, int k);
		static vertex getIntersection(vertex v1, vertex v2, int k);
		void clipPoly(int polyNum);
		void polyRasterize(int polyNum);
		void drawit();
		void clearFramebuffer();
		void setFramebuffer(int x, int y, float R, float G, float B);
}p;


bool polygon :: sortAEL(AEL_node n1, AEL_node n2)
{
	return n1.currentX<n2.currentX?true:false;
}
void polygon :: fill_AET(int x, int y)
{
	v1.x = x; v1.y = y;
	AET_poly[pCount].push_back(v1);
}
void polygon :: fill_table(vector<vertex>AET, int polyNum)
{
	vertex first;
	first.x = -10;
	first.y = -10;
	int count = 0;
	for(int i = 0; i<ImageH;i++)
	{
		if(table[polyNum][i].size())
			table[polyNum][i].clear();
	}
	if(AET.size() <= 2)
		return;
	if(AET.size())
	{
		while(AET.size())
		{
			if(!count)
			{
				v1.x = AET.back().x;
				v1.y = AET.back().y;
				first = v1;
				AET.pop_back();
			}	
			v2.x = AET.back().x;
			v2.y = AET.back().y;
			//vertex 1 will always have lower ordinate
			if(v1.y < v2.y)
			{
				e.vertex1 = v1;
				e.vertex2 = v2;
			}
			else if(v1.y > v2.y)
			{
				e.vertex1 = v2;
				e.vertex2 = v1;	
			}
			if(v1.y != v2.y)
				table[polyNum][e.vertex1.y].push_back(e);
			v1 = v2;
			count = 1;
			AET.pop_back();
		}
		v2 = first;
		if(v1.y < v2.y)
		{
			e.vertex1 = v1;
			e.vertex2 = v2;
		}
		else if(v1.y > v2.y)
		{
			e.vertex1 = v2;
			e.vertex2 = v1;	
		}
		if(v1.y != v2.y)
			table[polyNum][e.vertex1.y].push_back(e);
	}
}


direction polygon :: getDir(int point, int k)
{
	if(k == 0)
	{
		if(point < min_x)
			return out;
		else 
			return in;
	}
	else if(k == 1)
	{
		if(point <= max_x)
			return in;
		else 
			return out;
	}
	else if(k == 2)
	{
		if(point < min_y)
			return out;
		else
			return in;
	}
	else //if k==3
	{
		if(point <= max_y)
			return in;
		else
			return out;
	}
}
vertex polygon :: getIntersection(vertex v1, vertex v2, int k)
{
	float slope;
	float c;
	vertex v;
	if(k == 0 || k == 1)
	{
		slope = (1.0*(v2.y - v1.y)/(v2.x - v1.x));
		c = 1.0*v1.y - slope*v1.x*1.0;
	}

	if(k == 0) //left edge
	{	
		v.x = min_x;
		v.y = (int)floor(slope*v.x + c);
	}
	else if(k == 1)  //right edge
	{
		v.x = max_x;
		v.y = (int)floor(slope*v.x + c);
	}
	else if(k == 2) //bottom edge
	{
		v.y = min_y;
		if(v1.x != v2.x)
		{
			slope = 1.0*(v2.y - v1.y)/(v2.x - v1.x);
			c = 1.0*v1.y - slope*v1.x*1.0;
			v.x = (int)floor((v.y-c)/slope);
		}
		else v.x = v1.x;
	}
	else   //top edge
	{
		v.y = max_y;
		if(v1.x != v2.x)
		{
			slope = 1.0*(v2.y - v1.y)/(v2.x - v1.x);
			c = 1.0*v1.y - slope*v1.x*1.0;
			v.x = (int)floor((v.y-c)/slope);
		}
		else v.x = v1.x;
	}
	return v;
}

void polygon :: clipPoly(int polyNum)
{
	/*
	k = 0   left edge
	k = 1   right edge
	k = 2   bottom edge
	k = 3   top edge
	*/
	direction d1; 
	direction d2;
	vector<vertex>out;
	vector<vertex>temp;
	temp.clear();
	out = AET_poly[polyNum];
	assert(AET_poly[polyNum].size() != 0);
	int count = 0;
	vertex v1, v2, first;	
	for(int k=0;k<4;k++)   //Corresponds to 4 sides of clipping polygon
	{
		if(out.size())
		{
			v1.x = out.back().x;
			v1.y = out.back().y;
			first = v1;
			out.pop_back();
			while(out.size())
			{
				v2.x = out.back().x;
				v2.y = out.back().y;
				out.pop_back();
				if(k == 0 || k == 1)
				{
					d1 = getDir(v1.x,k);
					d2 = getDir(v2.x,k);
				}
				else if(k == 2 || k == 3)
				{
					d1 = getDir(v1.y,k);
					d2 = getDir(v2.y,k);
				}
				if(d1 == in && d2 == in)
				{
					temp.push_back(v2);
				}
				else if(d1 == direction::in && d2 == direction::out )
				{
					temp.push_back(polygon::getIntersection(v1,v2,k));
				}
				else if(d1 == direction::out && d2 == direction::in)
				{
					temp.push_back(polygon::getIntersection(v1,v2,k));
					temp.push_back(v2);
				}
				v1 = v2;
			}
			v2 = first;
			if(k == 0 || k == 1)
			{
				d1 = getDir(v1.x,k);
				d2 = getDir(v2.x,k);
			}
			else if(k == 2 || k == 3)
			{
				d1 = getDir(v1.y,k);
				d2 = getDir(v2.y,k);
			}
			if(d1 == in && d2 == in)
			{
				temp.push_back(v2);
			}
			else if(d1 == direction::in && d2 == direction::out )
			{
				temp.push_back(polygon::getIntersection(v1,v2,k));
			}
			else if(d1 == direction::out && d2 == direction::in)
			{
				temp.push_back(polygon::getIntersection(v1,v2,k));
				temp.push_back(v2);
			}
			out = temp;
			temp.clear();
		}
	}
	p.fill_table(out,polyNum);
}
void polygon :: polyRasterize(int polyNum)
{
	int i = polyNum;
	unsigned int k=0;
	float R, G, B;
	if(table[polyNum].size())
	{
		if(color[polyNum].size())
		{
			R = color[polyNum].at(0);
			G = color[polyNum].at(1);
			B = color[polyNum].at(2);
		}
	
		for(int j=0; j<ImageH; j++)
		{
			for(k=0; k<table[i][j].size(); k++)
			{
				A.currentX = 1.0*table[i][j].at(k).vertex1.x;
				A.maxY = table[i][j].at(k).vertex2.y;
				A.xIncr = 1.0*(table[i][j].at(k).vertex1.x - table[i][j].at(k).vertex2.x)/(table[i][j].at(k).vertex1.y - table[i][j].at(k).vertex2.y);
				AEL.push_back(A);
			}
			sort(AEL.begin(), AEL.end(), &polygon::sortAEL);
			k=0;
			while(k<AEL.size())
			{	
				if(AEL.at(k).maxY == j)
				{
					AEL.erase(AEL.begin() + k);
					k--;
				}
				k++;
			}
			for(k=0;k<AEL.size();k+=2)
			{
				for(unsigned int l = (unsigned int)ceil(AEL.at(k).currentX); l<(unsigned int)floor(AEL.at(k+1).currentX);++l)
				{
					setFramebuffer(l,j,R,G,B);
				}
				AEL.at(k).currentX += AEL.at(k).xIncr;
				AEL.at(k+1).currentX += AEL.at(k+1).xIncr;
			}
		}
	}
	drawit();
}
void keyboard ( unsigned char key, int x, int y )
{
	switch ( key )
	{
		case 'C':
		case 'c':
			s = clip;
			break;
		default:
			s = input;
			break;
	}
}
struct color {
	float r, g, b;		// Color (R,G,B values)
};

// Draws the scene
void polygon :: drawit(void)
{
   glDrawPixels(ImageW,ImageH,GL_RGB,GL_FLOAT,framebuffer);
}

// Clears framebuffer to black
void polygon :: clearFramebuffer()
{
	int i,j;
	for(i=0;i<ImageH;i++) {
		for (j=0;j<ImageW;j++) {
			framebuffer[i][j][0] = 0.0;
			framebuffer[i][j][1] = 0.0;
			framebuffer[i][j][2] = 0.0;
		}
	}
}
// Sets pixel x,y to the color RGB
// I've made a small change to this function to make the pixels match
// those returned by the glutMouseFunc exactly - Scott Schaefer 
void polygon :: setFramebuffer(int x, int y, float R, float G, float B)
{
	// changes the origin from the lower-left corner to the upper-left corner
	y = ImageH - 1 - y;
	if (R<=1.0)
		if (R>=0.0)
			framebuffer[y][x][0]=R;
		else
			framebuffer[y][x][0]=0.0;
	else
		framebuffer[y][x][0]=1.0;
	if (G<=1.0)
		if (G>=0.0)
			framebuffer[y][x][1]=G;
		else
			framebuffer[y][x][1]=0.0;
	else
		framebuffer[y][x][1]=1.0;
	if (B<=1.0)
		if (B>=0.0)
			framebuffer[y][x][2]=B;
		else
			framebuffer[y][x][2]=0.0;
	else
		framebuffer[y][x][2]=1.0;
}

void display(void)
{	
	if(s == clip)
	{
		glColor3f(1.0,1.0,1.0);
		glBegin(GL_LINE_LOOP);
			glVertex2i(left_x, left_y);
			glVertex2i(left_x, right_y);
			glVertex2i(right_x, right_y);
			glVertex2i(right_x, left_y);
		glEnd();
		//p.clearFramebuffer();
		/*for(int i=0;i<pCount;++i)
		{
			p.clipPoly(i);
			p.polyRasterize(i);
		}*/
	}
	else
	{
		if(pCount)
			p.polyRasterize(pCount-1);
	}
	glFlush ( );
}
int press = 0;
void mouseMotion(int x, int y)
{	
	if(s == clip)
	{
		if(!press)
		{
			left_x= x;
			left_y =y; 
			right_x = left_x;
			right_y = left_y;
			press = 1;
		}
		else
		{
			right_x= x;
			right_y =y; 
			glutPostRedisplay();
		}
	}
	glutPostRedisplay();
}
void init(void)
{
	gluOrtho2D ( 0, ImageW - 1, ImageH - 1, 0 );
	p.clearFramebuffer();
}

void mouseMove(int button, int state, int x, int y)
{
	if(s == clip && button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		press = 0;
		if(left_x <= right_x)
		{
			min_x = left_x;
			max_x = right_x;
		}
		else
		{
			min_x = right_x;
			max_x = left_x;
		}
		if(left_y <= right_y)
		{
			min_y = left_y;
			max_y = right_y;
		}
		else
		{
			min_y = right_y;
			max_y = left_y;
		}
		p.clearFramebuffer();
		for(int i=0;i<pCount;++i)
		{
			p.clipPoly(i);
			p.polyRasterize(i);
		}
	}
	else if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && s == input)
		p.fill_AET(x,y);
	else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && s == input)
	{
		float R, G;
		p.fill_AET(x,y);
		p.fill_table(p.AET_poly[pCount],pCount);
		R = 0.1*(rand()%10);
		G = 0.1*(rand()%10);
		p.color[pCount].push_back(R);
		p.color[pCount].push_back(G);
		p.color[pCount].push_back(abs(1.0-R-G));
		++pCount;
	}
	glutPostRedisplay();
}


int main(int argc, char** argv)
{
	srand(time(NULL));
	for(int j=0;j<15;++j)
	{
		p.table.push_back(vector<vector<edge>>());
		p.color.push_back(vector<float>());
		p.AET_poly.push_back(vector<vertex>());
		for(int i=0; i<ImageH; ++i){
			p.table[j].push_back(vector<edge>());
		}
	}

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(ImageW,ImageH);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Richeek Arya - Scan Convert Polygon");
	init();	
	glutDisplayFunc(display);
	glutMouseFunc ( mouseMove );
	glutMotionFunc(mouseMotion);
	glutKeyboardFunc ( keyboard );
	glutMainLoop();
	return 0;
}
