/*
Author: Xiaoting Bi
*/

#include "stdafx.h"
#include <windows.h>
#define GLUT_DISABLE_ATEXIT_HACK
#include<GL/glut.h>
#include<vector>
#include "Point.h"
#include "GetData.h"
#include <iostream>
#include <gl\glaux.h>
#include <cstdio>


using namespace std;

int diemater = 1;
int frame_length = 13;

// path to reach the location of a block
vector<vector<Point>> path_block;
// target location of a block
vector<Point> block_location;

int ** block_height_count;

int path_block_num; // a path index
int block_num; // the index of block in a path

int robot_block_num = 0;

// the position of the matrix
// the size of the matrix is: -x/2 .. column, -z/2 .. row
int begin_x, begin_z;

// size of the matrix in use
int column, row;

bool is_robot_finished = false;

// matrix entrance
int entrance_x, entrance_y;

// the path is for removing
bool is_remove_path;

// begin to process display() function
bool begin_display;

// variables for texture
unsigned char* data;
unsigned int width, height;
static GLuint texName;

int loadBMP(const char* path, unsigned char*& data, unsigned int& width, unsigned int& height)
{
	// Data read from the header of the BMP file
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int imageSize;   // = width*height*3
	// Open the file in binary mode
	FILE* fp = fopen(path,"rb");
	if (!fp)                              
	{
		cout<<"Image could not be opened\n"; 
		return 0;
	}

	if ( fread(header, 1, 54, fp)!=54 )
	{ 
		// If not 54 bytes read : problem
		cout<<"Not a correct BMP file\n";
		return 0;
	}

	if ( header[0]!='B' || header[1]!='M' )
	{
		cout<<"Not a correct BMP file\n";
		return 0;
	}

	// Read ints from the byte array
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    
		imageSize=(width)*(height)*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      
		dataPos=54; // The BMP header is done 

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	if (fread(data,1,imageSize,fp) != imageSize)
	{
		fclose(fp);
		return 0;
	}

	//Everything is in memory now, the file can be closed
	fclose(fp);
	return 1;
}

// new initial function
void myinit()
{
	if (!loadBMP("truck2.bmp",data, width, height))
		cout<<"Error loading bmp\n";
	else
		cout<<"Loaded bmp successfully!\n";

	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);	//to prevent color interpolation
	glEnable(GL_DEPTH_TEST);	

	//specify pixels are byte-aligned while unpacking from memory
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texName);//generate
	glBindTexture(GL_TEXTURE_2D, texName);//bind to a 2-D texture

	//wrap the image along s coordinate (width)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//wrap the image along t coordinate (height)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//how to treat magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//how to treat de-magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//bmp stores BGR instead of RGB
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
}

void init()
{
	// input data
	GetData * get_file = new GetData("F://WorkSpace//JAVAworkspace//jonnys-visualization//test1.txt");

	path_block = get_file->path_block;
	block_location = get_file->block_location;
//cout << "path block size " << path_block.size() << endl;

	is_remove_path = false;

	begin_display = false;

	column = get_file->column;
	row = get_file->row;

	path_block_num = 0; // begin with the first path
	block_num = 0; // the first block of the first path

	entrance_x = get_file->entrance_x;
	entrance_y = get_file->entrance_y;
	
	// initialize the matrix containing the height of each location
	block_height_count = new int*[column];
	
	for (int i = 0; i < column; i ++)
	{
		block_height_count[i] = new int[row];
		for (int j = 0; j < row; j ++)
		{
			block_height_count[i][j] = 0;
		}	
	}	

	begin_x = row / 2 * (-1);
	begin_z = column / 2 * (-1);

	glClearColor(1.0,1.0,1.0,1.0);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE); 

	// 2D texture
	glEnable(GL_TEXTURE_2D);

	myinit();
}
/************************************************************************/
/* begin testing 2D texture                                             */
/************************************************************************/

// draw robot with a better outlook
void draw_robot(double x, double y, double z)
{/**/
 	glLineWidth(1);
 	y = y / 2;
 	glPushMatrix();
 	glTranslatef(x, y, z);
 	glScalef(1,0.4,1);

	glColor3f(0.8,0,0);
	glutSolidCube(0.96*diemater);
	glColor3f(0.2, 0.2, 0.2);
	glutWireCube(diemater);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_2D, texName);
	glBegin(GL_QUADS);
	//top
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.5f, 0.5f,-0.5f);
	glTexCoord2f(1.0, 0.0); glVertex3f(-0.5f, 0.5f, 0.5f);
	glTexCoord2f(1.0, 1.0); glVertex3f( 0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0, 1.0); glVertex3f( 0.5f, 0.5f,-0.5f);

	//bottom
//  glTexCoord2f(0.0, 0.0); glVertex3f(-0.5f,-0.5f,-0.5f);
// 	glTexCoord2f(1.0, 0.0); glVertex3f( 0.5f,-0.5f,-0.5f);
// 	glTexCoord2f(1.0, 1.0); glVertex3f( 0.5f,-0.5f, 0.5f);
// 	glTexCoord2f(0.0, 1.0); glVertex3f(-0.5f,-0.5f, 0.5f);

	//front	
// 	glTexCoord2f(0.0, 0.0); glVertex3f(-0.5f,-0.5f, 0.5f);
// 	glTexCoord2f(1.0, 0.0); glVertex3f( 0.5f,-0.5f, 0.5f);
// 	glTexCoord2f(1.0, 1.0); glVertex3f( 0.5f, 0.5f, 0.5f);
// 	glTexCoord2f(0.0, 1.0); glVertex3f(-0.5f, 0.5f, 0.5f);

	//back	
// 	glTexCoord2f(0.0, 0.0); glVertex3f(-0.5,0.5,-0.5);
// 	glTexCoord2f(1.0, 0.0); glVertex3f( 0.5, 0.5,-0.5);
// 	glTexCoord2f(1.0, 1.0); glVertex3f( 0.5,-0.5,-0.5);
// 	glTexCoord2f(0.0, 1.0); glVertex3f(-0.5,-0.5,-0.5);

	//left too tiny...
// 	glTexCoord2f(0.0, 0.0); glVertex3f(-0.5,-0.5,-0.5);
// 	glTexCoord2f(1.0, 0.0); glVertex3f(-0.5,-0.5, 0.5);
// 	glTexCoord2f(1.0, 1.0); glVertex3f(-0.5, 0.5, 0.5);
// 	glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 0.5,-0.5);	

	//right
// 	glTexCoord2f(0.0, 0.0); glVertex3f( 0.5,-0.5, 0.5);
// 	glTexCoord2f(1.0, 0.0); glVertex3f( 0.5,-0.5,-0.5);
// 	glTexCoord2f(1.0, 1.0); glVertex3f( 0.5, 0.5,-0.5);
// 	glTexCoord2f(0.0, 1.0); glVertex3f( 0.5, 0.5, 0.5);

	glEnd();
	glPopMatrix();
	glFlush();
	glDisable(GL_TEXTURE_2D);
}
/*
// new display function
void mydisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(-1,1,-0.5,0,0,0,0,1.0,0.0);

	draw_robot(-2, 0, 0);
	
	glutSwapBuffers();
	glDisable(GL_TEXTURE_2D);
}*/

/************************************************************************/
/* end testing 2D texture                                               */
/************************************************************************/

void draw_block(double x, double y, double z)
{
	glLineWidth(1);
	y = y / 2;

	glPushMatrix();
	glTranslatef(x, y, z);
	glScalef(1,0.5,1);

	glColor3f(0.75,0.5,0.4);
	glutSolidCube(diemater*0.96);
	glColor3f(0.2,0.2,0.2);
	glutWireCube(diemater);
	glPopMatrix();
}

void draw_robot2(double x, double y, double z)
{
	glLineWidth(1);
	y = y / 2;
	glPushMatrix();
	glTranslatef(x, y, z);
	glScalef(1,0.5,1);
	glColor3f(0.8,0,0);
	glutSolidCube(diemater);	
	glColor3f(0,0,0);
	glutWireCube(diemater);
	glPopMatrix();
}

void draw_reservoir()
{
	glLineWidth(1);
	int reser_x = 0, reser_y = 0;
	if (entrance_x == 0) 
	{
		reser_x = begin_x - 5;
		reser_y = begin_z + entrance_y;
	}
	else
	{
		reser_x = begin_x + entrance_x;
		reser_y = begin_z - 5;
	}

	glPushMatrix();	
	glTranslated(reser_x, 1, reser_y);	
	glScaled(2,2,2);
	glColor3f(0,0,1);
	glutSolidCube(diemater);
	glColor3f(0,0,0);
	glutWireCube(diemater);
	glPopMatrix();
}

void draw_ground()
{		
	glLineWidth(2);
	for (int i = -1*frame_length; i <= frame_length; i ++)
	{
		for (int j = -1*frame_length; j <= frame_length; j ++) 
		{
			glPushMatrix();
			glTranslatef(j, -0.75, i);
			glColor3f(0, 0.8, 0);
			glutSolidCube(diemater);	
			glColor3f(1,1,1);
			glutWireCube(diemater);		
			
			glPopMatrix();
		}	
	}
}
			
void show_current_path()
{
	int current_x, current_z;
	double current_y;

	// putting down the block, the path color is yellow

	//bool is_the_last = false;
	for (size_t i = 0; i < path_block[path_block_num].size()-1; i ++)
	{
		if (block_num <= path_block[path_block_num].size()-2)
		{
			current_x = path_block[path_block_num][i].x;
			current_z = path_block[path_block_num][i].y;
			if (i >= 4)
			{
				current_y = block_height_count[current_x][current_z];
			} else 
			{
				current_y = 0;
			}
			if (!is_remove_path)
				glColor3f(1,1,0);
			else
				glColor3f(1,0.5,0);
			glBegin(GL_QUADS);
			glVertex3d(begin_x+current_x-0.5, current_y/2-0.25, begin_z+current_z-0.5);
			glVertex3d(begin_x+current_x-0.5, current_y/2-0.25, begin_z+current_z+0.5);
			glVertex3d(begin_x+current_x+0.5, current_y/2-0.25, begin_z+current_z+0.5);
			glVertex3d(begin_x+current_x+0.5, current_y/2-0.25, begin_z+current_z-0.5);
			glEnd();

			// highlight the target location
			int temp_x = path_block[path_block_num][path_block[path_block_num].size()-2].x;
			int temp_z = path_block[path_block_num][path_block[path_block_num].size()-2].y;
			double temp_y = current_y;

			glLineWidth(2);
			glColor3f(0.5,0,0);
			glBegin(GL_LINE_LOOP);
			glVertex3d(begin_x+temp_x-0.4, temp_y/2-0.25, begin_z+temp_z-0.4);
			glVertex3d(begin_x+temp_x-0.4, temp_y/2-0.25, begin_z+temp_z+0.4);
			glVertex3d(begin_x+temp_x+0.4, temp_y/2-0.25, begin_z+temp_z+0.4);
			glVertex3d(begin_x+temp_x+0.4, temp_y/2-0.25, begin_z+temp_z-0.4);
			glEnd();
		} else
		{
			if (i < path_block[path_block_num].size()-2)
			{
				current_x = path_block[path_block_num][i].x;
				current_z = path_block[path_block_num][i].y;
				if (i >= 4)
				{
					current_y = block_height_count[current_x][current_z];
				} else 
				{
					current_y = 0;
				}
				if (!is_remove_path)
					glColor3f(1,1,0);
				else
					glColor3f(1,0.5,0);
				glBegin(GL_QUADS);
				glVertex3d(begin_x+current_x-0.5, current_y/2-0.25, begin_z+current_z-0.5);
				glVertex3d(begin_x+current_x-0.5, current_y/2-0.25, begin_z+current_z+0.5);
				glVertex3d(begin_x+current_x+0.5, current_y/2-0.25, begin_z+current_z+0.5);
				glVertex3d(begin_x+current_x+0.5, current_y/2-0.25, begin_z+current_z-0.5);
				glEnd();
			}			
		}
	}	
}

void timer_funct(int value)
{
	glutPostRedisplay();
	glutTimerFunc(100, timer_funct, 1);
}

void keyboard_funct(int key, int x, int y)
{
	if (key == GLUT_KEY_RIGHT)
		begin_display = true;
	else if (key == GLUT_KEY_LEFT)
		begin_display = false;
}

void display()
{
	if (!begin_display)
		return;

	glClearColor(1,1,1,1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(-1,1,-0.5,0,0,0,0,1.0,0.0);
	
	draw_reservoir();

	// if the robot goes back
	if (path_block_num < path_block.size()) // put down a block
	{		
		// show current path
		show_current_path();
		draw_ground();
		// blocks that has been already put down		
		for (int i = 0; i < path_block_num; i ++)
		{			
			int loc_x = block_location[i].x;
			int loc_z = block_location[i].y;
			int height_count = block_height_count[loc_x][loc_z];
			for (int itemp = 0; itemp < height_count; itemp ++)
			{
				draw_block(begin_x + loc_x, itemp, begin_z + loc_z);
			}			
		}

		int size_of_a_path = path_block[path_block_num].size()-1;
//cout << "size of a path " << size_of_a_path << endl;
		if (path_block[path_block_num][size_of_a_path].x == 1) // x = 1 means putting down a block; x = -1 means removing a block
		{					
			// show the built paths
			if (block_num < path_block[path_block_num].size()-2) // still in this path
			{
				int loc_x = path_block[path_block_num][block_num].x;
				int loc_z = path_block[path_block_num][block_num].y;

				int block_height_temp = 0;
				if (block_num >= 4)
					block_height_temp = block_height_count[loc_x][loc_z];
				else 
					block_height_temp = 0;

				draw_robot(begin_x + loc_x, block_height_temp, begin_z + loc_z);
				draw_block(begin_x + loc_x, block_height_temp+1, begin_z + loc_z);

	//cout << "path block num" << path_block_num << ": " << loc_x << " " << loc_z << endl;
	
				block_num ++;
				robot_block_num ++;
				is_robot_finished = true;
				is_remove_path = false;
			} else if (block_num == path_block[path_block_num].size()-2)
			{
				if (path_block[path_block_num].size() == 6)
				{
					if (entrance_x == 0)
					{
						draw_block(0,0,entrance_y);
						draw_robot(-1,0,entrance_y);
					} else 
					{
						draw_block(entrance_x,0,0);
						draw_robot(entrance_x,0,-1);
					}
					block_height_count[entrance_x][entrance_y] ++;
					block_num ++;
					is_robot_finished = false;
					is_remove_path = false;
				} else
				{
					int loc_x = path_block[path_block_num][block_num].x;
					int loc_z = path_block[path_block_num][block_num].y;

					// the previous location
					int p_loc_x = path_block[path_block_num][block_num-1].x;
					int p_loc_z = path_block[path_block_num][block_num-1].y;

					draw_robot(begin_x + p_loc_x, block_height_count[p_loc_x][p_loc_z], begin_z + p_loc_z);
					draw_block(begin_x + loc_x, block_height_count[loc_x][loc_z], begin_z + loc_z);

					block_height_count[loc_x][loc_z] ++;

					//cout << "path block num" << path_block_num << ": " << loc_x << " " << loc_z << endl;

					block_num ++;
					is_robot_finished = false;
					is_remove_path = false;
				}			
			} else if (robot_block_num > 0)
			{
				int loc_x = block_location[path_block_num].x;
				int loc_z = block_location[path_block_num].y;
				draw_block(begin_x + loc_x, block_height_count[loc_x][loc_z]-1, begin_z + loc_z);
			
				robot_block_num --;

				int r_loc_x = path_block[path_block_num][robot_block_num].x;
				int r_loc_z = path_block[path_block_num][robot_block_num].y;

				int block_height_temp = 0;
				if (robot_block_num >= 4)
					block_height_temp = block_height_count[r_loc_x][r_loc_z];
				else 
					block_height_temp = 0;

				draw_robot(begin_x + r_loc_x, block_height_temp, begin_z + r_loc_z); 
			} else
			{
				int loc_x = path_block[path_block_num][block_num-1].x;
				int loc_z = path_block[path_block_num][block_num-1].y;
				draw_block(begin_x + loc_x, block_height_count[loc_x][loc_z]-1, begin_z + loc_z);

				path_block_num ++;
				block_num = 0;
				robot_block_num = 0;
				is_robot_finished = true;
				is_remove_path = false;
			}			
		} else if (path_block[path_block_num][size_of_a_path].x == -1)// x = -1 means removing a block
		{
			// show the built paths
			if (block_num < path_block[path_block_num].size()-2) // still in this path
			{
				int loc_x = path_block[path_block_num][block_num].x;
				int loc_z = path_block[path_block_num][block_num].y;

				int block_height_temp = 0;
				if (block_num >= 4)
					block_height_temp = block_height_count[loc_x][loc_z];
				else 
					block_height_temp = 0;

				draw_robot(begin_x + loc_x, block_height_temp, begin_z + loc_z);

				block_num ++;
				robot_block_num ++;
				is_robot_finished = true;
				is_remove_path = true;
			} else if (block_num == path_block[path_block_num].size()-2)
			{
				if (path_block[path_block_num].size() == 6)
				{
					if (entrance_x == 0)
					{
						//draw_block(0,0,entrance_y);
						draw_robot(-1,0,entrance_y);
					} else 
					{
						//draw_block(entrance_x,0,0);
						draw_robot(entrance_x,0,-1);
					}
					block_height_count[entrance_x][entrance_y] --;
					block_num ++;
					is_robot_finished = false;
					is_remove_path = true;
				} else
				{
					int loc_x = path_block[path_block_num][block_num].x;
					int loc_z = path_block[path_block_num][block_num].y;
					// the previous location
					int p_loc_x = path_block[path_block_num][block_num-1].x;
					int p_loc_z = path_block[path_block_num][block_num-1].y;

					draw_robot(begin_x + p_loc_x, block_height_count[p_loc_x][p_loc_z], begin_z + p_loc_z);
					//draw_block(begin_x + loc_x, block_height_count[loc_x][loc_z], begin_z + loc_z);

					block_height_count[loc_x][loc_z] --;

					//cout << "path block num" << path_block_num << ": " << loc_x << " " << loc_z << endl;

					block_num ++;
					//robot_block_num ++;
					is_robot_finished = false;
					is_remove_path = true;
				}			
			} else if (robot_block_num > 0)
			{
				//int loc_x = block_location[path_block_num].x;
				//int loc_z = block_location[path_block_num].y;
				robot_block_num --;
				int r_loc_x = path_block[path_block_num][robot_block_num].x;
				int r_loc_z = path_block[path_block_num][robot_block_num].y;

				int block_height_temp = 0;
				if (robot_block_num >= 4)
					block_height_temp = block_height_count[r_loc_x][r_loc_z];
				else 
					block_height_temp = 0;

				draw_block(begin_x + r_loc_x, block_height_temp+1, begin_z + r_loc_z);
				draw_robot(begin_x + r_loc_x, block_height_temp, begin_z + r_loc_z); 
			} else
			{
				int loc_x = path_block[path_block_num][block_num-1].x;
				int loc_z = path_block[path_block_num][block_num-1].y;
				draw_block(begin_x + loc_x, block_height_count[loc_x][loc_z]-1, begin_z + loc_z);

				path_block_num ++;
				block_num = 0;
				robot_block_num = 0;
				is_robot_finished = true;
				is_remove_path = true;
			}			

		}
	} else
	{
		draw_ground();
		for (int i = 0; i < path_block_num; i ++)
		{
			int loc_x = block_location[i].x;
			int loc_z = block_location[i].y;
			//draw_block(begin_x + loc_x, block_height_count[loc_x][loc_z], begin_z + loc_z);

			int height_count = block_height_count[loc_x][loc_z];
			for (int itemp = 0; itemp < height_count; itemp ++)
			{
				draw_block(begin_x + loc_x, itemp, begin_z + loc_z);
			}
		}
	}	

	glutSwapBuffers();
	
}

void reshape(int w,int h)
{
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-1 * frame_length,frame_length,-1 * frame_length *h/w, frame_length *h/w,-1 * frame_length,frame_length);
	else
		glOrtho(-1 * frame_length *w/h,frame_length *w/h, -1 *frame_length, frame_length,-1 * frame_length,frame_length);
}

int main(int argc,char** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(700,700);
	glutInitWindowPosition(0,0);
	glutCreateWindow("RobotVis3D");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutTimerFunc(100, timer_funct, 1);
	glutSpecialFunc(keyboard_funct);
	init();

	//glutDisplayFunc(&mydisplay);
	//myinit();
	/* replace with any animate code */
	//glutIdleFunc(doIdle);

	glutMainLoop();
}