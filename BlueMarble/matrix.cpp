#include <iostream>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

void printMatrix(const glm::mat4 &matrix) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout
				<< std::setw(10)
				<< std::setprecision(2)
				<< std::fixed
				<< matrix[j][i]
				<< " ";
		}
		std::cout << std::endl;
	}
}

void translationMatrix() {
	std::cout << std::setw(10);
	std::cout << "-----------------------" << std::endl;
	std::cout << "|Translation Operation|" << std::endl;
	std::cout << "-----------------------" << std::endl;

	glm::mat4 matrix_identity = glm::identity<glm::mat4>();
	glm::vec3 translation{ 10.0f, 10.0f, 10.0f};
	glm::mat4 matrix_translation = glm::translate(matrix_identity, translation);

	printMatrix(matrix_translation);
	std::cout << std::endl;

	glm::vec4 position{ -5.0f, 1.0f, 2.0f, 1.0f };

	std::cout << "Before translation" << std::endl;
	std::cout << glm::to_string(position) << std::endl;
	std::cout << "After translation" << std::endl;
	std::cout << glm::to_string(matrix_translation * position) << std::endl;

	std::cout << std::endl;
}

void scaleMatrix() {
	std::cout << std::setw(10);
	std::cout << "-----------------------" << std::endl;
	std::cout << "|Scale Operation|" << std::endl;
	std::cout << "-----------------------" << std::endl;

	glm::mat4 matrix_identity = glm::identity<glm::mat4>();
	glm::vec3 scale{ 2.0f, 2.0f, 2.0f };
	glm::mat4 matrix_scale = glm::scale(matrix_identity, scale);

	printMatrix(matrix_scale);
	std::cout << std::endl;

	glm::vec4 position{ -5.0f, 1.0f, 2.0f, 1.0f };

	std::cout << "Before scale" << std::endl;
	std::cout << glm::to_string(position) << std::endl;
	std::cout << "After scale" << std::endl;
	std::cout << glm::to_string(matrix_scale * position) << std::endl;

	std::cout << std::endl;
}

void rotationMatrix() {
	std::cout << std::setw(10);
	std::cout << "-----------------------" << std::endl;
	std::cout << "|Rotation Operation|" << std::endl;
	std::cout << "-----------------------" << std::endl;

	glm::mat4 matrix_identity = glm::identity<glm::mat4>();
	constexpr float angle = glm::radians(90.0f);
	glm::vec3 axis{ 0, 0, 1 };
	glm::mat4 matrix_rotation = glm::rotate(matrix_identity, angle, axis);

	printMatrix(matrix_rotation);
	std::cout << std::endl;

	glm::vec4 position{ 100.0f, 0.0f, 0.0f, 1.0f };

	std::cout << "Before scale" << std::endl;
	std::cout << glm::to_string(position) << std::endl;
	std::cout << "After scale" << std::endl;
	std::cout << glm::to_string(matrix_rotation * position) << std::endl;

	std::cout << std::endl;
}

void composedMatrix() {
	std::cout << std::setw(10);
	std::cout << "-----------------------" << std::endl;
	std::cout << "|Composed Matrix|" << std::endl;
	std::cout << "-----------------------" << std::endl;

	glm::mat4 matrix_identity = glm::identity<glm::mat4>();

	glm::vec3 translation{ 10.0f, 10.0f, 10.0f };
	glm::mat4 matrix_translation = glm::translate(matrix_identity, translation);

	glm::vec3 scale{ 2.0f, 2.0f, 2.0f };
	glm::mat4 matrix_scale = glm::scale(matrix_identity, scale);

	constexpr float angle = glm::radians(90.0f);
	glm::vec3 axis{ 0, 0, 1 };
	glm::mat4 matrix_rotation = glm::rotate(matrix_identity, angle, axis);

	glm::vec3 position{ 10.0f, 10.0f, 0.0f };
	
	glm::mat4 model_matrix = matrix_translation * matrix_rotation * matrix_scale;

	std::cout << "Model Matrix" << std::endl;
	printMatrix(model_matrix);
}

void modelViewProjection() {
	std::cout << std::setw(10);
	std::cout << "-----------------------" << std::endl;
	std::cout << "|Model View Projection|" << std::endl;
	std::cout << "-----------------------" << std::endl;

	glm::mat4 matrix_model = glm::identity<glm::mat4>();

	glm::vec3 eye{0.0f, 0.0f, 10.0f};
	glm::vec3 center{0.0f, 0.0f, 0.0f};
	glm::vec3 up{0.0f, 1.0f, 0.0f};
	glm::mat4 matrix_view = glm::lookAt( eye, center, up );

	std::cout << "View Matrix" << std::endl;
	printMatrix(matrix_view);

	constexpr float fov = glm::radians(45.0f);
	const float aspect_ratio = 640.0f / 480.0f;
	const float near = 0.001f;
	const float far = 1000.0f;
	glm::mat4 matrix_projection = glm::perspective(fov, aspect_ratio, near, far);

	std::cout << "Projection Matrix" << std::endl;
	printMatrix(matrix_projection);

	glm::mat4 matrix_model_view_projection = matrix_projection * matrix_view * matrix_model;

	std::cout << "Model View Projection Matrix" << std::endl;
	printMatrix(matrix_model_view_projection);

}

int main() {
	//translationMatrix();
	//scaleMatrix();
	//rotationMatrix();
	//composedMatrix();
	modelViewProjection();

	return 0;
}