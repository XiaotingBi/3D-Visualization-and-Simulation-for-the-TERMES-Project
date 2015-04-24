#include "stdafx.h"
#define GLUT_DISABLE_ATEXIT_HACK

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include<GL/glut.h>
#include <gl/glaux.h>

const int ImageWidth = 64;
const int  ImageHeight = 64;
const int BMP_Header_Length = 54;
const int WindowHeight = 400, WindowWidth = 400;

void grab()
{
	FILE * pDummyFile;
	FILE * pWritingFile;
	GLubyte* pPixelData;
	GLubyte BMP_Header[BMP_Header_Length];
	GLint i, j;
	GLint PixelDataLength;

	i = WindowWidth * 3;
	while (i%4 != 0)
		i ++;
	PixelDataLength = i * WindowHeight;

	pPixelData = (GLubyte*)malloc(PixelDataLength);
	if (pPixelData == 0)
		exit(0);

	pDummyFile = fopen("brick5.bmp", "rb");
	if (pDummyFile == 0)
		exit(0);

	pWritingFile = fopen("result.bmp", "wb");
	if (pWritingFile == 0)
		exit(0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glReadPixels(0,0,WindowWidth, WindowHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, pPixelData);

	fread(BMP_Header, sizeof(BMP_Header), 1, pDummyFile);
	fwrite(BMP_Header, sizeof(BMP_Header), 1, pWritingFile);
	fseek(pWritingFile, 0x0012, SEEK_SET);
	i = WindowWidth;
	j = WindowHeight;
	fwrite(&i, sizeof(i), 1, pWritingFile);
	fwrite(&j, sizeof(j), 1, pWritingFile);

	fclose(pDummyFile);
	fclose(pWritingFile);
	free(pPixelData);
}

int power_of_two(int n)
{
	if (n <= 0)
		return 0;
	return (n & (n-1)) == 0;
}

GLuint load_texture(const char* file_name)
{
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint texture_id = 0;
	GLint last_texture_id;

	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
		return 0;

	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	{
		GLint line_bytes = width * 3;
		while(line_bytes%4 != 0)
			line_bytes ++;
		total_bytes = line_bytes * height;
	}

	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		return 0;
	}

	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if (!power_of_two(width) || !power_of_two(height) || width > max || height > max)
		{
			const GLint new_width = 256;
			const GLint new_height = 256;
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;

			new_line_bytes = new_width*3;
			while(new_line_bytes%4 != 0)
				new_line_bytes ++;
			new_total_bytes = new_line_bytes * new_height;

			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if (new_pixels == 0)
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}

			gluScaleImage(GL_RGB, width, height, GL_UNSIGNED_BYTE, pixels, new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	glGenTextures(1, &texture_id);
	if (texture_id == 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture_id);

	free(pixels);
	return texture_id;
}

GLuint texGround;
GLuint texWall;

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75,1,1,21);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(1,5,5,0,0,0,0,0,1);

	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-8.0f, -8.0f, 0.0f);   
		glTexCoord2f(0.0f, 5.0f); glVertex3f(-8.0f, 8.0f, 0.0f);   
		glTexCoord2f(5.0f, 5.0f); glVertex3f(8.0f, 8.0f, 0.0f);   
		glTexCoord2f(5.0f, 0.0f); glVertex3f(8.0f, -8.0f, 0.0f); 
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texWall);
	glBegin(GL_QUADS);   
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -3.0f, 0.0f);   
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-6.0f, -3.0f, 1.5f);   
		glTexCoord2f(5.0f, 1.0f); glVertex3f(6.0f, -3.0f, 1.5f);   
		glTexCoord2f(5.0f, 0.0f); glVertex3f(6.0f, -3.0f, 0.0f);   
	glEnd();   

	glRotatef(-90, 0, 0, 1);   
	glBegin(GL_QUADS);   
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -3.0f, 0.0f);   
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-6.0f, -3.0f, 1.5f);   
	glTexCoord2f(5.0f, 1.0f); glVertex3f(6.0f, -3.0f, 1.5f);   
	glTexCoord2f(5.0f, 0.0f); glVertex3f(6.0f, -3.0f, 0.0f);   
	glEnd();   

	// 交换缓冲区，并保存像素数据到文件   
	//glutSwapBuffers();   
	glFlush();  
	//grab();   
}

int main(int argc, char** argv)   
{   
	// GLUT初始化   
	glutInit(&argc, argv);   
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);   
	glutInitWindowPosition(100, 100);   
	glutInitWindowSize(WindowWidth, WindowHeight);   
	glutCreateWindow("Texture");   
	glutDisplayFunc(&display);   
  
	glEnable(GL_DEPTH_TEST);   
	glEnable(GL_TEXTURE_2D);   
	texGround = load_texture("Koala.bmp");   
	texWall = load_texture("Koala.bmp");   
 
	glutMainLoop();   

	return 0;   
}   