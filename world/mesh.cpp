#include <string>
#include <vector>
#include <sstream>
#include <GL/glew.h>

#include "mesh.h"
#include "ogl_h_func.h"

using namespace std;

Mesh::Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, Shader shader){
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;  
	this->shader = shader;

	setup_mesh();
}

void Mesh::setup_mesh(){
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

	//The Vertex struct can be used as one big array. So we store all the mesh data in a single VBO 
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
	
	//Vertex data
	glVertexAttribPointer(glGetAttribLocation(shader.program, "in_Position"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0); 
	glEnableVertexAttribArray(glGetAttribLocation(shader.program, "in_Position"));
	
	//Vertex normal data
	glVertexAttribPointer(glGetAttribLocation(shader.program, "in_Normal"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal)); 
	glEnableVertexAttribArray(glGetAttribLocation(shader.program, "in_Normal"));

	//UV data
	glVertexAttribPointer(glGetAttribLocation(shader.program, "in_TexCoord"), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords)); 
	glEnableVertexAttribArray(glGetAttribLocation(shader.program, "in_TexCoord"));

	//"Unbind" vertex array
	glBindVertexArray(0);
}

void Mesh::render(){
	GLuint diffuseNr = 1;
    GLuint specularNr = 1;

	glUseProgram(shader.program);

    for(GLuint i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
        // Retrieve texture number (the N in diffuse_textureN)
        stringstream ss;
        string number;
        string name = textures[i].type;
        if(name == "texture_diffuse"){
			// Transfer GLuint to stream
            ss << diffuseNr++;
		} else if(name == "texture_specular") {
			// Transfer GLuint to stream
            ss << specularNr++; 
		}
        number = ss.str(); 

        glUniform1f(glGetUniformLocation(shader.program, ("material." + name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // Draw mesh
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
