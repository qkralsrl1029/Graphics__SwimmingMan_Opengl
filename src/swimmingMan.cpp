//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "cube.h"
#include "glm/glm.hpp"		//must be to use glm

//for matrix transformation
#include "glm/gtc/matrix_transform.hpp"	
#include "glm/gtx/transform.hpp"


//declaration of 4X4 mat
glm::mat4 projectMat;
glm::mat4 viewMat;
//declaration of 4X4 vector
//glm::vec4

GLuint pvmMatrixID;		//vertex shader uniform ID

float rotAngle = 0.0f;
float legRotAngle = 0.0f;
float legMaxAngle = 0.6f;
float legMinAngle = -0.6f;
bool isLegUp = true;
bool isDrawingCar = true;

typedef glm::vec4  color4;
typedef glm::vec4  point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.5, -0.5, 0.5, 1.0),
	point4(-0.5, 0.5, 0.5, 1.0),
	point4(0.5, 0.5, 0.5, 1.0),
	point4(0.5, -0.5, 0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5, 0.5, -0.5, 1.0),
	point4(0.5, 0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

// RGBA colors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(0.0, 1.0, 1.0, 1.0),   // cyan
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 1.0, 1.0, 1.0)  // white
};

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[b]; points[Index] = vertices[b];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[d]; points[Index] = vertices[d];  Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	colorcube();

	//Create Buffer and send to GPU once at a time
	
	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl");	
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	pvmMatrixID = glGetUniformLocation(program, "mPVM");		//uniform으로 정의된 mPVM, 모든 vertex를 대상으로 동일한 작업 수행 <-> in/out

	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
	viewMat = glm::lookAt(glm::vec3(0, 0, 6), glm::vec3(0.2, 0, 0), glm::vec3(0, 1, 0));	//Camera pos

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------

void drawCar(glm::mat4 carMat)
{
	glm::mat4 modelMat, pvmMat;
	glm::vec3 wheelPos[4];

	wheelPos[0] = glm::vec3(0.3, 0.24, -0.1); // rear right
	wheelPos[1] = glm::vec3(0.3, -0.24, -0.1); // rear left
	wheelPos[2] = glm::vec3(-0.3, 0.24, -0.1); // front right
	wheelPos[3] = glm::vec3(-0.3, -0.24, -0.1); // front left

	// car body
	modelMat = glm::scale(carMat, glm::vec3(1, 0.6, 0.2));		//scaling, 차체를 만들기 위해 납작하고 길게
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// car top
	// model * translation * scailing * vertices
	modelMat = glm::translate(carMat, glm::vec3(0, 0, 0.2));  //P*V*C*T*S*v
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.6, 0.2));
	// projection* view * model
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// car wheel
	for (int i = 0; i < 4; i++)
	{
		modelMat = glm::translate(carMat, wheelPos[i]);  //P*V*C*T*S*v	//translation과 rotation의 순서 주의
		modelMat = glm::scale(modelMat, glm::vec3(0.2, 0.1, 0.2));
		//wheel rotation
		modelMat = glm::rotate(modelMat, -rotAngle*50.0f, glm::vec3(0, 1, 0));	// 좌우방향 회전을 위해 y축을 기준으로 회전
		pvmMat = projectMat * viewMat * modelMat;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}
}

void drawSwimmingMan(glm::mat4 basis)
{
	glm::mat4 modelMat, pvmMat;

	

	//body
	modelMat = glm::scale(basis, glm::vec3(1.8, 1, 0.6));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//head
	modelMat = glm::translate(basis, glm::vec3(-1.4, 0, 0));  //P*V*C*T*S*v
	modelMat = glm::scale(modelMat, glm::vec3(0.5, 0.6, 0.2));
	// projection* view * model
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);


	//right_arm
	//TRST
	modelMat = glm::translate(basis, glm::vec3(-0.4, 0, 0.4));				//이후 body에 맞춰 위치 설정
	modelMat = glm::rotate(modelMat, rotAngle , glm::vec3(0, 0, 1));	//회전
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.5, 0.1));				//스케일링
	modelMat = glm::translate(modelMat, glm::vec3(0.4, 0, 0));		//로컬 회전축으로 회전시키기 위해 임의의 회전축 방향으로 먼저 이동
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//right_forearm
	//TRST
	modelMat = glm::translate(basis, glm::vec3(-0.4, 0, 0.4));				//이후 body에 맞춰 위치 설정
	modelMat = glm::rotate(modelMat, rotAngle, glm::vec3(0, 0, 1));	//회전
	modelMat = glm::scale(modelMat, glm::vec3(0.6, 0.5, 0.1));				//스케일링
	modelMat = glm::translate(modelMat, glm::vec3(2, 0, 0));		//로컬 회전축으로 회전시키기 위해 임의의 회전축 방향으로 먼저 이동
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//left_arm
	//TRST
	modelMat = glm::translate(basis, glm::vec3(-0.4, 0, -0.4));				//이후 body에 맞춰 위치 설정
	modelMat = glm::rotate(modelMat, rotAngle, glm::vec3(0, 0, 1));	//회전
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.5, 0.1));				//스케일링
	modelMat = glm::translate(modelMat, glm::vec3(-0.4, 0, 0));		//로컬 회전축으로 회전시키기 위해 임의의 회전축 방향으로 먼저 이동
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//left_forearm
	//TRST
	modelMat = glm::translate(basis, glm::vec3(-0.4, 0, -0.4));				//이후 body에 맞춰 위치 설정
	modelMat = glm::rotate(modelMat, rotAngle, glm::vec3(0, 0, 1));	//회전
	modelMat = glm::scale(modelMat, glm::vec3(0.6, 0.5, 0.1));				//스케일링
	modelMat = glm::translate(modelMat, glm::vec3(-2, 0, 0));		//로컬 회전축으로 회전시키기 위해 임의의 회전축 방향으로 먼저 이동
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	 
	


	// right_upperLeg
	// TRST
	modelMat = glm::translate(basis, glm::vec3(1, 0, 0.2));
	modelMat = glm::rotate(modelMat, legRotAngle,glm::vec3(0, 0, 1));
	modelMat = glm::scale(modelMat, glm::vec3(1, 0.5, 0.1));
	modelMat = glm::translate(modelMat, glm::vec3(0.5, 0, 0));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// right_lowerLeg
	// TRST
	modelMat = glm::translate(basis, glm::vec3(1, 0, 0.2));
	modelMat = glm::rotate(modelMat, legRotAngle, glm::vec3(0, 0, 1));
	modelMat = glm::scale(modelMat, glm::vec3(1, 0.5, 0.1));
	modelMat = glm::translate(modelMat, glm::vec3(1.6, 0, 0));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);


	// left_upperLeg
	// TRST
	modelMat = glm::translate(basis, glm::vec3(1.1, 0, -0.2));
	modelMat = glm::rotate(modelMat, -legRotAngle, glm::vec3(0, 0, 1));
	modelMat = glm::scale(modelMat, glm::vec3(1, 0.5, 0.1));
	modelMat = glm::translate(modelMat, glm::vec3(0.5, 0, 0));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// left_lowerLeg
	// TRST
	modelMat = glm::translate(basis, glm::vec3(1.1, 0, -0.2));
	modelMat = glm::rotate(modelMat, -legRotAngle, glm::vec3(0, 0, 1));
	modelMat = glm::scale(modelMat, glm::vec3(1, 0.5, 0.1));
	modelMat = glm::translate(modelMat, glm::vec3(1.6, 0, 0));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);



}


void display(void)
{
	glm::mat4 worldMat, pvmMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	worldMat = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 1.0f, 0.0f));	//for rotation, rotate(기준,회전각,회전축)

	/*
	if (isDrawingCar)
	{
		drawCar(worldMat);
	}
	else
	{
		pvmMat = projectMat * viewMat * worldMat;		//프로젝션 * 뷰 * 모델
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}
	*/


	drawSwimmingMan(worldMat);
	glutSwapBuffers();

}

//----------------------------------------------------------------------------

void idle()		//OpenGl이 아이들 상태일때 호출되는 함수, 회전각 및 화면 최신화
{
	static int prevTime = glutGet(GLUT_ELAPSED_TIME);	//return how much time takes after program runs
	int currTime = glutGet(GLUT_ELAPSED_TIME);

	if (abs(currTime - prevTime) >= 20)
	{
		//to rotate consistently regardless of HW
		float t = abs(currTime - prevTime);

		if (legRotAngle >= legMaxAngle)
			isLegUp = false;
		else if (legRotAngle <= legMinAngle)
			isLegUp = true;

		if(isLegUp)
			legRotAngle += glm::radians(t * 360.0f / 10000.0f);
		else
			legRotAngle -= glm::radians(t * 360.0f / 10000.0f);

		rotAngle += glm::radians(t*360.0f / 10000.0f);		//10초에 한바퀴
		prevTime = currTime;
		glutPostRedisplay();
	}
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)	//change mode
{
	switch (key) {
	case 'c': case 'C':
		isDrawingCar = !isDrawingCar;
		break;
	case 033:  // Escape key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	}
}

//----------------------------------------------------------------------------

void resize(int w, int h)	//when window size changed
{
	//match with size of window, regular aspect ratio
	float ratio = (float)w / (float)h;
	glViewport(0, 0, w, h);

	projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f);		// calculate projection transfotmation

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Color Car");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resize);	//called when window size changed
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
