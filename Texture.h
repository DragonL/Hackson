#ifndef GL_TEXTURE_LOADER
#define GL_TEXTURE_LOADER

typedef struct													
{
	GLubyte	*imageData;											
	GLuint	bpp;												
	GLuint	width;												
	GLuint	height;												
	GLuint	texID;												
} TextureTga;


BOOL BuildTexture(char *szPathName, GLuint &texid);


BOOL BuildTexture(char *filename, TextureTga *texture);

#endif