#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "ogl_h_func.h"

/* A simple function that will read a file into an allocated char pointer buffer */
char* filetobuf(const char *file)
{
    FILE *fptr;
    long length;
    char *buf;
 
    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) { /* Return NULL on failure */
		printf("Couldn't open file: %s!\n", file);
        return NULL;
	}
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */
 
    return buf; /* Return the buffer */
}

Shader compile_shader(std::string vert_file, std::string frag_file){
	Shader shader;

    int maxLength;
    char *vertexInfoLog;
    char *fragmentInfoLog;
    char *programInfoLog;

    GLint IsCompiled_VS, IsCompiled_FS, IsLinked;

    /* These pointers will receive the contents of our shader source code files */
    GLchar *vertexsource, *fragmentsource;

    /* Read our shaders into the appropriate buffers */
    vertexsource = filetobuf(vert_file.c_str());
    fragmentsource = filetobuf(frag_file.c_str());

    /* Create an empty vertex shader handle */
    shader.vertex = glCreateShader(GL_VERTEX_SHADER);
 
    /* Send the vertex shader source code to GL */
    /* Note that the source code is NULL character terminated. */
    /* GL will automatically detect that therefore the length info can be 0 in this case (the last parameter) */
    glShaderSource(shader.vertex, 1, (const GLchar**)&vertexsource, 0);

    /* Compile the vertex shader */
    glCompileShader(shader.vertex);

	/* Free the vertex source */
    free(vertexsource);

	/* Check if there were a compilation error */
    glGetShaderiv(shader.vertex, GL_COMPILE_STATUS, &IsCompiled_VS);
    if(IsCompiled_VS == GL_FALSE)
    {
       glGetShaderiv(shader.vertex, GL_INFO_LOG_LENGTH, &maxLength);
 
       /* The maxLength includes the NULL character */
       vertexInfoLog = (char *)malloc(maxLength);
 
       glGetShaderInfoLog(shader.vertex, maxLength, &maxLength, vertexInfoLog);
 
       printf("Vert error: %s\n", vertexInfoLog);

	   /* Free the allocated char array */
       free(vertexInfoLog);
	   shader.error = true;
	   return shader;
	}

    /* Create an empty fragment shader handle */
    shader.fragment = glCreateShader(GL_FRAGMENT_SHADER);
 
    /* Send the fragment shader source code to GL */
    /* Note that the source code is NULL character terminated. */
    /* GL will automatically detect that therefore the length info can be 0 in this case (the last parameter) */
    glShaderSource(shader.fragment, 1, (const GLchar**)&fragmentsource, 0);
 
    /* Compile the fragment shader */
    glCompileShader(shader.fragment);
 
    glGetShaderiv(shader.fragment, GL_COMPILE_STATUS, &IsCompiled_FS);
    if(IsCompiled_FS == GL_FALSE)
    {
       glGetShaderiv(shader.fragment, GL_INFO_LOG_LENGTH, &maxLength);
 
       /* The maxLength includes the NULL character */
       fragmentInfoLog = (char *)malloc(maxLength);
 
       glGetShaderInfoLog(shader.fragment, maxLength, &maxLength, fragmentInfoLog);
 
       printf("Frag error: %s\n", fragmentInfoLog);

	   /* Free the allocated char array */
       free(fragmentInfoLog);
	   shader.error = true;
	   return shader;
    }

    /* If we reached this point it means the vertex and fragment shaders compiled and are syntax error free. */
    /* We must link them together to make a GL shader program */
    /* GL shader programs are monolithic. It is a single piece made of 1 vertex shader and 1 fragment shader. */
    /* Assign our program handle a "name" */
    shader.program = glCreateProgram();
 
    /* Attach our shaders to our program */
    glAttachShader(shader.program, shader.vertex);
    glAttachShader(shader.program, shader.fragment);
 
    /* Bind attribute index 0 (coordinates) to in_Position and attribute index 1 (color) to in_Color */
    /* Attribute locations must be setup before calling glLinkProgram. */
    glBindAttribLocation(shader.program, 0, "in_Position");
    glBindAttribLocation(shader.program, 1, "in_Normal");
    glBindAttribLocation(shader.program, 2, "in_TexCoord");
 
    /* Link our program */
    /* At this stage, the vertex and fragment programs are inspected, optimized and a binary code is generated for the shader. */
    /* The binary code is uploaded to the GPU, if there is no error. */
    glLinkProgram(shader.program);
 
    /* Again, we must check and make sure that it linked. If it fails, it would mean either there is a mismatch between the vertex */
    /* and fragment shaders. It might be that you have surpassed your GPU's abilities. Perhaps too many ALU operations or */
    /* too many texel fetch instructions or too many interpolators or dynamic loops. */
 
    glGetProgramiv(shader.program, GL_LINK_STATUS, &IsLinked);
    if(IsLinked == GL_FALSE)
    {
       /* Noticed that glGetProgramiv is used to get the length for a shader program, not glGetShaderiv. */
       glGetProgramiv(shader.program, GL_INFO_LOG_LENGTH, &maxLength);
 
       /* The maxLength includes the NULL character */
       programInfoLog = (char *)malloc(maxLength);
 
       /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
       glGetProgramInfoLog(shader.program, maxLength, &maxLength, programInfoLog);
 
       printf("program error: %s\n", programInfoLog);

	   /* Free the allocated char array */
       free(programInfoLog);
	   shader.error = true;
	   return shader;
    }

	return shader;
}

void free_shader(Shader shader){
    glDetachShader(shader.program, shader.vertex);
    glDetachShader(shader.program, shader.fragment);
    glDeleteProgram(shader.program);
    glDeleteShader(shader.vertex);
    glDeleteShader(shader.fragment);
}

GLuint *surf_to_texture(SDL_Surface *surf){

    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);

	glBindTexture(GL_TEXTURE_2D, tex_id);

	int Mode = GL_RGB;

	if(surf->format->BytesPerPixel == 4) {
		Mode = GL_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, Mode, surf->w, surf->h, 0, Mode, GL_UNSIGNED_BYTE, surf->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint *id_ptr = new GLuint;

	*id_ptr = tex_id;

	return id_ptr;
}