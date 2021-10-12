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


////////////////////////////////////////////////////////////
float armRotAngle = 0.0f;
float legRotAngle = 0.0f;
float legMaxAngle = 0.6f;
float legMinAngle = -0.6f;
bool isLegUp = true;

float gap = 0.1f;
float armRotGap = 0.9f;
float legRotGap = 0.1f;

//Component Scale
glm::vec3 bodyScale = glm::vec3(1.8, 1, 0.6);
glm::vec3 headScale = glm::vec3(0.5, 0.6, 0.2);
glm::vec3 armScale = glm::vec3(0.8, 0.5, 0.1);
glm::vec3 forearmScale = glm::vec3(0.6, 0.5, 0.1);
glm::vec3 upperlegScale = glm::vec3(1, 0.5, 0.1);
glm::vec3 lowerlegScale = glm::vec3(1, 0.5, 0.1);

//Component Position with matching parent to make Hierachy
glm::vec3 bodyPos = glm::vec3(0, 0, 0);		// highest parent
glm::vec3 headPos = glm::vec3(bodyPos.x - bodyScale.x + headScale.x, bodyPos.y, bodyPos.z);
glm::vec3 rightArmPos = glm::vec3(bodyPos.x - (armScale.x / 2), bodyPos.y, bodyPos.z + (bodyScale.z / 2) + gap);
glm::vec3 rightForearmPos = glm::vec3(rightArmPos.x, rightArmPos.y, rightArmPos.z);
glm::vec3 leftArmPos = glm::vec3(bodyPos.x - (armScale.x / 2), bodyPos.y, bodyPos.z - (bodyScale.z / 2) - gap);
glm::vec3 leftForearmPos = glm::vec3(leftArmPos.x, leftArmPos.y, leftArmPos.z);
glm::vec3 rightUpperlegPos = glm::vec3(bodyPos.x + upperlegScale.x, bodyPos.y, bodyPos.z + upperlegScale.z + gap);
glm::vec3 rightLowerlegPos = glm::vec3(rightUpperlegPos.x, rightUpperlegPos.y, rightUpperlegPos.z);
glm::vec3 leftUpperlegPos = glm::vec3(bodyPos.x + upperlegScale.x, bodyPos.y, bodyPos.z - upperlegScale.z - gap);
glm::vec3 leftLowerLegPos = glm::vec3(leftUpperlegPos.x, leftUpperlegPos.y, leftUpperlegPos.z);


////////////////////////////////////////////////////////////

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

void drawSwimmingMan(glm::mat4 basis)
{
	glm::mat4 modelMat, pvmMat;

	
	//body
	modelMat = glm::translate(basis, bodyPos);
	modelMat = glm::scale(modelMat, bodyScale);
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//head
	modelMat = glm::translate(basis, headPos);  
	modelMat = glm::scale(modelMat, headScale);
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);


	//right_arm
	//TRST
	modelMat = glm::translate(basis, rightArmPos);										//이후 body에 맞춰 위치 설정
	modelMat = glm::rotate(modelMat, armRotAngle, glm::vec3(0, 0, 1));		//회전
	modelMat = glm::scale(modelMat, armScale);										//스케일링
	modelMat = glm::translate(modelMat, glm::vec3(armScale.x/2, 0, 0));		//로컬 회전축으로 회전시키기 위해 임의의 회전축 방향으로 먼저 이동
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//right_forearm
	//TRST
	modelMat = glm::translate(basis, rightForearmPos);			
	modelMat = glm::rotate(modelMat, armRotAngle, glm::vec3(0, 0, 1));	
	modelMat = glm::scale(modelMat, forearmScale);				
	modelMat = glm::translate(modelMat, glm::vec3(armScale.x+(forearmScale.x/2)+ armRotGap, 0, 0));		
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//left_arm
	//TRST
	modelMat = glm::translate(basis, leftArmPos);				
	modelMat = glm::rotate(modelMat, armRotAngle, glm::vec3(0, 0, 1));	
	modelMat = glm::scale(modelMat, armScale);				
	modelMat = glm::translate(modelMat, glm::vec3(-armScale.x / 2, 0, 0));		
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//left_forearm
	//TRST
	modelMat = glm::translate(basis, leftForearmPos);				
	modelMat = glm::rotate(modelMat, armRotAngle, glm::vec3(0, 0, 1));	
	modelMat = glm::scale(modelMat, forearmScale);			
	modelMat = glm::translate(modelMat, glm::vec3(-(armScale.x + (forearmScale.x / 2) + armRotGap), 0, 0));		
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	 
	
	// right_upperLeg
	// TRST
	modelMat = glm::translate(basis, rightUpperlegPos);
	modelMat = glm::rotate(modelMat, legRotAngle,glm::vec3(0, 0, 1));
	modelMat = glm::scale(modelMat,upperlegScale);
	modelMat = glm::translate(modelMat, glm::vec3(upperlegScale.x/2, 0, 0));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// right_lowerLeg
	// TRST
	modelMat = glm::translate(basis, rightLowerlegPos);
	modelMat = glm::rotate(modelMat, legRotAngle, glm::vec3(0, 0, 1));
	modelMat = glm::scale(modelMat, lowerlegScale);
	modelMat = glm::translate(modelMat, glm::vec3(upperlegScale.x + (lowerlegScale.x / 2) + legRotGap, 0, 0));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);


	// left_upperLeg
	// TRST
	modelMat = glm::translate(basis, leftUpperlegPos);
	modelMat = glm::rotate(modelMat, -legRotAngle, glm::vec3(0, 0, 1));
	modelMat = glm::scale(modelMat, upperlegScale);
	modelMat = glm::translate(modelMat, glm::vec3(upperlegScale.x/2, 0, 0));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// left_lowerLeg
	// TRST
	modelMat = glm::translate(basis, leftLowerLegPos);
	modelMat = glm::rotate(modelMat, -legRotAngle, glm::vec3(0, 0, 1));
	modelMat = glm::scale(modelMat, lowerlegScale);
	modelMat = glm::translate(modelMat, glm::vec3(upperlegScale.x + (lowerlegScale.x / 2) + legRotGap, 0, 0));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}


void display(void)
{
	glm::mat4 worldMat, pvmMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	worldMat = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 1.0f, 0.0f));	//for rotation, rotate(기준,회전각,회전축)

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
			legRotAngle += glm::radians(t * 360.0f / 5000.0f);
		else
			legRotAngle -= glm::radians(t * 360.0f / 5000.0f);

		armRotAngle += glm::radians(t*360.0f / 5000.0f);		//5초에 한바퀴
		prevTime = currTime;
		glutPostRedisplay();
	}
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)	//change mode
{
	switch (key) {
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
	glutCreateWindow("Swimming Man");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resize);	//called when window size changed
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
