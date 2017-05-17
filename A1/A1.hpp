#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

private:
	void initGrid();
	void updateIndicatorPos();
	void addCube();
	void removeCube(int num);
	void copyStack(int prev_col, int prev_row);
	void flattenCubeIndices();

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Fields related to cube geometry
	GLuint m_cubes_vao;
	GLuint m_cubes_vbo;
	GLuint m_cubes_index_vbo;

	// Fields related to indicator geomety
	GLuint m_indicator_vao;
	GLuint m_indicator_vbo;

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	float colour[24];
	std::vector<glm::vec3> cubes;
	std::vector<std::vector<unsigned int> > cube_indices;
	std::vector<unsigned int> flattened_cube_indices;
	std::vector<glm::vec3> indicator;
	int current_col;
	int current_row;
	int current_column;
	bool is_shift_pressed;
	float zoom;
};
