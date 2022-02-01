#include <iostream>
#include <cassert>
#include <array>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const int width = 800;
const int height = 600;
bool b_enable_mouse_movement = false;
glm::vec2 previous_cursor{ 0.0f, 0.0f };

struct directionalLight {
	glm::vec3 direction;
	GLfloat intensity;
};

std::string readFile(const char* file_path) {
	std::string file_contents;
	if (std::ifstream file_stream{ file_path, std::ios::in }) {
		file_contents.assign(
			std::istreambuf_iterator<char>(file_stream),
			std::istreambuf_iterator<char>()
		);
	};

	return file_contents;
}

void checkShader(GLuint shader_id) {
	GLint result = GL_TRUE;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE) {
		GLint info_log_length = 0;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

		if (info_log_length > 0) {
			std::string shader_info_log(info_log_length, '\0');
			glGetShaderInfoLog(shader_id, info_log_length, nullptr, &shader_info_log[0]);

			std::cout << "Erro no shader" << std::endl;
			std::cout << shader_info_log << std::endl;

			assert(false);
		}
	}
}

GLuint loadShader(const char* vertex_shader_file,
				 const char* fragment_shader_file) {

	std::string vertex_shader_source = readFile("shaders/triangle_vert.glsl");
	std::string fragment_shader_source = readFile("shaders/triangle_frag.glsl");

	assert(!vertex_shader_source.empty());
	assert(!fragment_shader_source.empty());

	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vertex_shader_pt = vertex_shader_source.c_str();
	const char* fragment_shader_pt = fragment_shader_source.c_str();

	glShaderSource(vertex_shader_id, 1, &vertex_shader_pt, nullptr);
	glCompileShader(vertex_shader_id);
	checkShader(vertex_shader_id);

	glShaderSource(fragment_shader_id, 1, &fragment_shader_pt, nullptr);
	glCompileShader(fragment_shader_id);
	checkShader(fragment_shader_id);

	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);

	GLint result = GL_TRUE;
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);

	if (result == GL_FALSE) {
		std::cout << "Erro ao linkar programa" << std::endl;
		assert(false);
	}

	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);

	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	return program_id;
}

GLuint loadTexture(const char* texture_file) {
	std::cout << "Carregando texture ... " << texture_file << std::endl;

	//stbi_set_flip_vertically_on_load(true);

	int texture_width = 0;
	int texture_height = 0;
	int number_of_components = 0;
	
	unsigned char* texture_data = stbi_load(texture_file, &texture_width,
		&texture_height, &number_of_components,
		3
	);

	assert(texture_data);

	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, 
		texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE,
		texture_data
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(texture_data);

	return texture_id;
}

struct vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
};

class FlyCamera {
public:
	void look(float yaw, float pitch) {
		yaw *= sensivity;
		pitch *= sensivity;

		const glm::vec3 right = glm::normalize(glm::cross(direction, up));
		const glm::mat4 matrix_identity = glm::identity<glm::mat4>();

		glm::mat4 yaw_rotation = glm::rotate(matrix_identity, glm::radians(yaw), up);
		glm::mat4 pitch_rotation = glm::rotate(matrix_identity, glm::radians(pitch), right);

		direction = yaw_rotation * pitch_rotation * glm::vec4{ direction, 0.0f };
		up = pitch_rotation  * glm::vec4{ up, 0.0f };


		auto rotation_composed = yaw_rotation * pitch_rotation;
		location = rotation_composed * glm::vec4{ location, 1.0f };
		

	}

	void moveForward(float amount) {
		location += direction * amount * speed;
	}

	void moveRight(float amount) {
		glm::vec3 right = glm::normalize(glm::cross(direction, up));
		location += right * amount * speed;
		
		//direction = direction - location;
	}

	glm::mat4 getViewProjection() const {
		glm::mat4 view = glm::lookAt(location, location + direction, up);
		glm::mat4 projection = glm::perspective(fov, aspect_ratio, near, far);
		return projection * view;
	}

	glm::mat4 getView() const {
		return glm::lookAt(location, location + direction, up);
	}

	float speed = 10.0f;
	float sensivity = 1.0f;

	glm::vec3 location{0.0f, 0.0f, 10.0f};
	glm::vec3 direction{ 0.0f, 0.0f, -1.0f };
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };

	float fov = glm::radians(45.0f);
	float aspect_ratio = static_cast<float> (width / height);
	float near = 0.01f;
	float far = 1000.0f;

};

FlyCamera camera;

void mouseButtonCallback(GLFWwindow *window, int button, int action, int modifiers) {
	std::cout << "BUTTON - " << button << std::endl
		<< "ACTION - " << action << std::endl
		<< "MODIFIER - " << modifiers << std::endl;
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			double x, y;
			glfwGetCursorPos(window, &x, &y);
			previous_cursor = glm::vec2{ x, y };

			b_enable_mouse_movement = true;
		}
		if (action == GLFW_RELEASE) {
			b_enable_mouse_movement = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}
void mouseMotionCallback(GLFWwindow* window, double x, double y) {
	if (b_enable_mouse_movement) {

		glm::vec2 current_cursor{ x, y };
		glm::vec2 delta_cursor = current_cursor - previous_cursor;

		camera.look(delta_cursor.x, 0);

		std::cout << glm::to_string(delta_cursor) << std::endl;

		previous_cursor = current_cursor;
	}
}

GLuint loadGeometry() {
	std::array<vertex, 6> quad{
		vertex{ glm::vec3{ -1.0f, -1.0f, 0.0f},
				glm::vec3{0.0f, 0.0f, 1.0f},
				glm::vec3{ 1.0f, 0.0f, 0.0f} ,
				glm::vec2{ 0.0f, 0.0f}
			  },
		vertex{ glm::vec3{ 1.0f, -1.0f, 0.0f},
				glm::vec3{0.0f, 0.0f, 1.0f},
				glm::vec3{ 0.0f, 1.0f, 0.0f} ,
				glm::vec2{ 1.0f, 0.0f}
			  },
		vertex{ glm::vec3{ 1.0f, 1.0f, 0.0f},
				glm::vec3{0.0f, 0.0f, 1.0f},
				glm::vec3{ 0.0f, 1.0f, 0.0f} ,
				glm::vec2{ 1.0f, 1.0f}
			  },
		vertex{ glm::vec3{ -1.0f, 1.0f, 0.0f},
				glm::vec3{0.0f, 0.0f, 1.0f},
				glm::vec3{ 0.0f, 0.0f, 1.0f},
				glm::vec2{ 0.0f, 1.0f}
			  }
	};

	std::array<glm::ivec3, 2> index_buffer{
		glm::ivec3{0, 1, 3},
		glm::ivec3{3, 1, 2}
	};


	GLuint vertex_buffer = 0;
	glGenBuffers(1, &vertex_buffer);

	GLuint element_buffer = 0;
	glGenBuffers(1, &element_buffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer), index_buffer.data(), GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), nullptr);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex),
		reinterpret_cast<void*>(offsetof(vertex, normal))
	);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(vertex),
		reinterpret_cast<void*>(offsetof(vertex, color))
	);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(vertex),
		reinterpret_cast<void*>(offsetof(vertex, uv))
	);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	return vao;

}

void generateSphereMesh(GLuint resolution, 
					std::vector<vertex>& vertex_buf,
					std::vector<glm::ivec3>& indices
					) {
	vertex_buf.clear();
	indices.clear();

	constexpr float pi = glm::pi<float>();
	constexpr float two_pi = glm::two_pi<float>();
	const float inv_resolution = 1.0f / static_cast<float>(resolution - 1);

	for (GLuint u_index = 0; u_index < resolution; u_index++) {
		const float u = u_index * inv_resolution;
		const float theta = glm::mix(0.0f, pi, u);
		for (GLuint v_index = 0; v_index < resolution; v_index++) {
			const float v = v_index * inv_resolution;
			const float phi = glm::mix(0.0f, two_pi, v);

			glm::vec3 vertex_position = {
				glm::sin(theta) * glm::cos(phi),
				glm::sin(theta) * glm::sin(phi),
				glm::cos(theta)
			};

			vertex vertice{
				vertex_position,
				glm::normalize(vertex_position),
				glm::vec3{1.0f, 1.0f, 1.0f},
				glm::vec2{v, u}
			};
			
			vertex_buf.push_back(vertice);
		}
	}

	for (GLuint u = 0; u < resolution - 1; u++) {
		for (GLuint v = 0; v < resolution - 1; v++) {
			GLuint p0 = u + v * resolution;
			GLuint p1 = (u + 1) + v * resolution;
			GLuint p2 = (u + 1) + (v + 1) * resolution;
			GLuint p3 = u + (v + 1) * resolution;

			indices.push_back(glm::ivec3{ p0, p1, p3 });
			indices.push_back(glm::ivec3{ p3, p1, p2 });
		}
	}
}

GLuint loadSphere(GLuint &num_vertices, GLuint &num_indices) {
	std::vector<vertex> vertices;
	std::vector<glm::ivec3> triangles;
	generateSphereMesh(50, vertices, triangles);

	std::cout << vertices.data() << std::endl;

	num_vertices = vertices.size();
	num_indices = triangles.size() * 3;

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex),
		vertices.data(), GL_STATIC_DRAW
	);

	GLuint element_buffer;
	glGenBuffers(1, &element_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(glm::ivec3),
		triangles.data(), GL_STATIC_DRAW
	);


	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);



	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex),
		reinterpret_cast<void*>(offsetof(vertex, normal))
	);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(vertex),
		reinterpret_cast<void*>(offsetof(vertex, color))
	);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(vertex),
		reinterpret_cast<void*>(offsetof(vertex, uv))
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	return vao;
}

int main() {

	glfwInit();

	GLFWwindow* window = glfwCreateWindow(width, height, "Hello opengl!", 
										  nullptr, nullptr);
	assert(window);

	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, mouseMotionCallback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glViewport(0, 0, 800, 600);

	assert(glewInit() == GLEW_OK);

	std::cout << "Vendor Name - " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer - " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "OPENGL - " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL - " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	std::string vertex_shader_source = "shaders/triangle_vert.glsl";
	std::string fragment_shader_source = "shaders/triangle_frag.glsl";

	std::cout << std::endl << vertex_shader_source;
	std::cout << std::endl << fragment_shader_source << std::endl;

	GLuint program_id = loadShader(
		vertex_shader_source.c_str(),
		fragment_shader_source.c_str()
	);

	GLuint texture_id = loadTexture("textures/earth_2k.jpg");
	glm::mat4 matrix_model = glm::rotate(
								glm::identity<glm::mat4>(),
								glm::radians(270.0f),
								glm::vec3{ 1.0f, 0.0f, 0.0f}
							);

	matrix_model = glm::rotate(
		matrix_model,
		glm::radians(270.0f),
		glm::vec3{ 0.0f, 0.0f, 1.0f }
	);

	//GLuint quad_vao = loadGeometry();

	GLuint sphere_num_vertices = 0;
	GLuint sphere_num_indices = 0;
	GLuint sphere_vao = loadSphere(sphere_num_vertices, sphere_num_indices);

	std::cout << "Numero de vertices - " << sphere_num_vertices <<
		std::endl;

	std::cout << "Numero de indices - " << sphere_num_indices <<
		std::endl;


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	double previous_time = glfwGetTime();

	directionalLight light;
	light.direction = glm::vec3{ 0.0f, 0.0f, -1.0f };
	light.intensity = 1.0f;

	while (!glfwWindowShouldClose(window)) {

		glEnable(GL_DEPTH_TEST);

		double current_time = glfwGetTime();
		double delta_time = current_time - previous_time;
		if (delta_time > 0) {
			previous_time = current_time;
			if(!b_enable_mouse_movement)
				matrix_model = glm::rotate(
									matrix_model, 
									glm::radians(0.1f), 
									glm::vec3{ 0.0f, 0.0f, 1.0f }
								);
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(program_id);

		glm::mat4 matrix_normal = glm::inverse(glm::transpose(camera.getView() * matrix_model));
		glm::mat4 view_projection = camera.getViewProjection();
		glm::mat4 matrix_model_view_projection = view_projection * matrix_model;

		GLint model_view_projection_loc =
			glGetUniformLocation(program_id, "model_view_projection");

		glUniformMatrix4fv(model_view_projection_loc, 1, GL_FALSE,
			glm::value_ptr(matrix_model_view_projection)
		);

		GLint normal_loc =
			glGetUniformLocation(program_id, "matrix_normal");

		glUniformMatrix4fv(normal_loc, 1, GL_FALSE,
			glm::value_ptr(matrix_normal)
		);

		GLint light_direction_loc =
			glGetUniformLocation(program_id, "light_direction");

		glUniform3fv(light_direction_loc, 1, glm::value_ptr(
			camera.getView() * glm::vec4{ light.direction, 0.0f }
			)
		);

		GLint light_intensity_loc =
			glGetUniformLocation(program_id, "light_intensity");

		glUniform1f(light_intensity_loc, light.intensity);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		GLint texture_sampler_loc = glGetUniformLocation(program_id, "texture_sampler");
		glUniform1i(texture_sampler_loc, 0);

		glBindVertexArray(sphere_vao);

		glPointSize(1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glDrawElements(GL_TRIANGLES, sphere_num_indices, GL_UNSIGNED_INT, nullptr);
		//glDrawArrays(GL_POINTS, 0, sphere_num_vertices);

		glBindVertexArray(0);
		glUseProgram(0);

		glfwPollEvents();
		glfwSwapBuffers(window);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera.moveForward(1.0 * delta_time);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera.moveForward(-1.0 * delta_time);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			camera.moveRight(-1.0 * delta_time);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			camera.moveRight(1.0 * delta_time);
		}

	}

	//glDeleteVertexArrays(1, &quad_vao);

	glfwTerminate();

	return 0;
}