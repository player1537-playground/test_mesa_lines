#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/time.h>
#include "glad.h"
//#include <GL/gl.h>
#define GLAPIENTRY APIENTRY
#include <GL/osmesa.h>

int main(int argc, char **argv) {
	char *VSS, *FSS;
	size_t nedges;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s VSS FSS nedges\n", argv[0]);
		return 1;
	}

	VSS = argv[1];
	FSS = argv[2];
	nedges = atoi(argv[3]);

	FILE *ff;

	size_t nsize;
	ff = fopen("nodes.f32f32.dat", "r");
	fseek(ff, 0, SEEK_END);
	nsize = ftell(ff);
	fseek(ff, 0, SEEK_SET);

	GLfloat *nodes;
	GLsizei nnodes;
	nnodes = nsize / 2 / sizeof(*nodes);
	nodes = malloc(nnodes * 2 * sizeof(*nodes));
	fread(nodes, sizeof(*nodes), 2 * nnodes, ff);
	fclose(ff);

	size_t esize;
	ff = fopen("edges.u32u32.dat", "r");
	fseek(ff, 0, SEEK_END);
	esize = ftell(ff);
	fseek(ff, 0, SEEK_SET);

	GLuint *edges;
	//GLsizei nedges;
	//nedges = esize / 2 / sizeof(*edges);
	edges = malloc(nedges * 2 * sizeof(*edges));
	fread(edges, sizeof(*edges), 2 * nedges, ff);
	fclose(ff);

	GLfloat uMinimum[2];
	GLfloat uMaximum[2];

	uMinimum[0] = uMaximum[0] = nodes[0];
	uMinimum[1] = uMaximum[1] = nodes[1];
	for (GLsizei i=1; i<nnodes; ++i) {
		if (nodes[2*i+0] < uMinimum[0]) uMinimum[0] = nodes[2*i+0];
		if (nodes[2*i+1] < uMinimum[1]) uMinimum[1] = nodes[2*i+1];
		if (nodes[2*i+0] > uMaximum[0]) uMaximum[0] = nodes[2*i+0];
		if (nodes[2*i+1] > uMaximum[1]) uMaximum[1] = nodes[2*i+1];
	}
	fprintf(stderr, "min: %f %f\n", uMinimum[0], uMinimum[1]);
	fprintf(stderr, "max: %f %f\n", uMaximum[0], uMaximum[1]);

	fprintf(stderr, "nodes\n");
	for (GLsizei i=0; i<10; ++i) {
		fprintf(stderr, "%d: %f %f\n", i, nodes[2*i+0], nodes[2*i+1]);
	}

	fprintf(stderr, "edges\n");
	for (GLsizei i=0; i<10; ++i) {
		fprintf(stderr, "%d: %d %d\n", i, edges[2*i+0], edges[2*i+1]);
	}

	int width, height;
	width = height = 1024;

	fprintf(stderr, "OSMesaGetProcAddress\n");
	void *v = OSMesaGetProcAddress("glDrawElements");
	fprintf(stderr, "v = %p\n", v);

	OSMesaContext context;

	fprintf(stderr, "OSMesaCreateContextExt\n");
    	context = OSMesaCreateContextExt(OSMESA_RGBA, 16, 0, 0, NULL);
    	if(!context){
       		fprintf(stderr, "could not init OSMesa context\n");
   		exit(1);
    	}

	GLubyte *image;
	
	fprintf(stderr, "imageSize\n");
    	unsigned int imageSize = width * height * 4 * sizeof(GLubyte);
    	image = (GLubyte *)malloc(imageSize);
    	if(!image){
        	fprintf(stderr, "could not allocate image\n");
    		exit(1);
    	}
	
	fprintf(stderr, "OSMesaMakeCurrent\n");
	int ret = OSMesaMakeCurrent(context, image, GL_UNSIGNED_BYTE, width, height);
	fprintf(stderr, "after\n");
	if(!ret){
        	fprintf(stderr, "could not bind to image buffer\n");
        	exit(1);
    	}

	fprintf(stderr, "initializing glad\n");
	if (!gladLoadGLLoader((GLADloadproc)OSMesaGetProcAddress)){
		printf("gladLoadGL failed\n");
		exit(1);
	}
	printf("OpenGL %s\n", glGetString(GL_VERSION));

	int success;
	char infoLog[512];

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const char * const*)&VSS, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("ERROR: Vertex Shader Compilation Failed\n%s\n", infoLog);
		exit(1);
	}
	
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, (const char * const*)&FSS, NULL);
	glCompileShader(fragmentShader);
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("ERROR: Fragment Shader Compilation Failed\n%s\n", infoLog);
		exit(1);	
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ERROR: Shader Program Linking Failed\n%s\n", infoLog);
		exit(1);
	}
	
	glUseProgram(shaderProgram);

	GLuint uMinimumLocation;
	uMinimumLocation = glGetUniformLocation(shaderProgram, "uMinimum");
	glUniform2fv(uMinimumLocation, 1, uMinimum);

	GLuint uMaximumLocation;
	uMaximumLocation = glGetUniformLocation(shaderProgram, "uMaximum");
	glUniform2fv(uMaximumLocation, 1, uMaximum);

	GLuint aPosBuffer;
	glGenBuffers(1, &aPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, aPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, nnodes * 2 * sizeof(*nodes), nodes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint aPosLocation;
	aPosLocation = glGetAttribLocation(shaderProgram, "aPos");
	glBindBuffer(GL_ARRAY_BUFFER, aPosBuffer);
	glVertexAttribPointer(aPosLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(aPosLocation);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nedges * 2 * sizeof(*edges), edges, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	struct timeval start;
	gettimeofday(&start, NULL);

	int n;
	for (n=0; n<50; ++n) {
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
		//glBindBuffer(GL_ARRAY_BUFFER, aPosBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glDrawElements(GL_LINES, 2*nedges, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);

		glFlush();
	}

	struct timeval end;
	gettimeofday(&end, NULL);

	double elapsedTimems;

	elapsedTimems = (end.tv_sec - start.tv_sec) * 1000.0;
	elapsedTimems += (end.tv_usec - start.tv_usec) / 1000.0;
	fprintf(stderr, "%.2lf FPS\n", 1000.0 / (elapsedTimems / n));

	FILE *f;
	f = fopen("out.rgba.bin", "w");
	fwrite(image, sizeof(*image), width * height * 4, f);
	fclose(f);
}
