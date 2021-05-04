#include <stdio.h>
#include <GLFW/glfw3.h>
#define M_MATH_IMPLEMENTATION
#include "m_math.h"
#include <string.h>
#include <stdlib.h>

#include "m_math_extensions.h"

/*
	@notes
	frustrum culling would be cool
*/

#define WINDOW_W 600
#define WINDOW_H 1000

GLuint compile_shader(GLenum type, const char *src);
int compile_shader_program(const char* str_vert_shader, const char* str_frag_shader, const char* attrib_name_0, const char* attrib_name_1);

g_timer general_timer1;
g_timer general_timer2;

#define NUMBER_CACHED_RAND_UNIT_SPHERE 100000
float3* cached_rand_unit_sphere;
int cached_rand_unit_sphere_index = 0;

float3* get_cached_rand_in_unit_sphere() {
	cached_rand_unit_sphere_index = (cached_rand_unit_sphere_index + 1 ) % NUMBER_CACHED_RAND_UNIT_SPHERE;
	return &cached_rand_unit_sphere[cached_rand_unit_sphere_index];
}

void close_callback(GLFWwindow * window) {
    d("close_callback");
}

void size_callback(GLFWwindow * window, int width, int height) {
    d("size_callback");
}

void cursorpos_callback(GLFWwindow * window, double mx, double my) {}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    d("key_callback");
    glfwSetWindowShouldClose(window, 1);
}

void mousebutton_callback(GLFWwindow * window, int button, int action, int mods) {}

void char_callback(GLFWwindow * window, unsigned int key) {
    d("char_callback");
}

void error_callback(int error, const char* description) {
    printf("%s\n", description);
}

void print_fun(const char* f) {
    printf("%s\n", f);
}
void update_particles(int number_particles, float gravity, float* points_vertices, float *points_speed) {
	// start_timer(&general_timer2);
	// print_fun(__FUNCTION__);
    int idx = 0;
	float3* rand_unit_sphere;
	float attenuate = 0.1;
	float expand = 1.0;

	int version = 1;
	if (version == 0){
		for(int _i = 0; _i< number_particles; _i++) {
			// Update position according to speed
			points_vertices[idx]	 += points_speed[idx + 0];
			points_vertices[idx + 1] += points_speed[idx + 1];
			points_vertices[idx + 2] += points_speed[idx + 2];

			idx+=3;
		}	
	} else if (version == 1) {
		for(int _i = 0; _i< number_particles; _i++) {
			// Update Y with gravity
			points_speed[idx + 1] = points_speed[idx + 1] - gravity;

			if (points_vertices[idx + 1] < -50.0){
				// Reset point
				rand_unit_sphere = get_cached_rand_in_unit_sphere();

				points_vertices[idx]	 = rand_unit_sphere->x * expand;
				points_vertices[idx + 1] = rand_unit_sphere->y * expand;
				points_vertices[idx + 2] = rand_unit_sphere->z * expand;
				
				points_speed[idx] 	  = rand_unit_sphere->x * attenuate;
				points_speed[idx + 1] = rand_unit_sphere->y * attenuate;
				points_speed[idx + 2] = rand_unit_sphere->z * attenuate;
			} else {
				// Update position according to speed
				points_vertices[idx]	 += points_speed[idx + 0];
				points_vertices[idx + 1] += points_speed[idx + 1];
				points_vertices[idx + 2] += points_speed[idx + 2];
			}

			idx+=3;
		}	
	} else if (version == 2) {
		for(int _i = 0; _i< number_particles; _i++) {
			rand_unit_sphere = get_cached_rand_in_unit_sphere();
			// Update Y with randomly

			points_speed[idx + 0] = rand_unit_sphere->x;
			points_speed[idx + 1] = rand_unit_sphere->y;
			points_speed[idx + 2] = rand_unit_sphere->z;

			points_vertices[idx]	 += points_speed[idx + 0];
			points_vertices[idx + 1] += points_speed[idx + 1];
			points_vertices[idx + 2] += points_speed[idx + 2];

			idx+=3;
		}
	}
	//stop_print_timer("Up", &general_timer2);
}

void update_camera(float* view_matrix, float time, float cam_speed, float3* camera_position, float3* camera_direction, float3* camera_up) {
	set_float3(camera_position, 20*cos(time*cam_speed) , 20, 20*sin(time*cam_speed));
	set_float3(camera_direction, 0 - camera_position->x ,0 - camera_position->y , 0 - camera_position->z);
	set_float3(camera_up, 0,1,0);
	
	
	m_mat4_lookat(view_matrix, camera_position, camera_direction, camera_up);	
}


int main(int argc, char const *argv[]) {
	printf("Fountain\n");

	// Cache random function calls
	cached_rand_unit_sphere = (float3*) malloc(NUMBER_CACHED_RAND_UNIT_SPHERE * sizeof(float3));
	for (int i = 0; i < NUMBER_CACHED_RAND_UNIT_SPHERE; ++i) {
		cached_rand_unit_sphere[i] = rand_in_unit_sphere(); 
	}

    GLFWwindow* window;
    if(!glfwInit()) {
        d("glfw init failed");
    }
    else {
        d("Glfw initialized");
    }
    window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Fountain", NULL, NULL);
    if(!window) {
        d("Create window failed");
        glfwTerminate();
        return -1;
    }
	
	d("Setting callbacks");
    glfwSetWindowCloseCallback(window, close_callback);
    glfwSetWindowSizeCallback(window, size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mousebutton_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetCursorPosCallback(window, cursorpos_callback);
    glfwSetErrorCallback(error_callback);
    d("Setting context");
    glfwMakeContextCurrent(window);

    float3 camera_position;
    float3 camera_direction;
    float3 camera_up;

	float view_matrix[] = M_MAT4_IDENTITY();
	float projection_matrix[] = M_MAT4_IDENTITY();
	float model_matrix[] = M_MAT4_IDENTITY();

    float aspect = WINDOW_W / (float)WINDOW_H;
    m_mat4_perspective(projection_matrix, 10.0, aspect, 0.1, 100.0);
    m_mat4_identity(view_matrix);

    update_camera(view_matrix, 0, 0, &camera_position, &camera_direction, &camera_up);

    char str_points_vert_shader[] =
        "attribute vec3 position;"
        "attribute vec3 color;"
        "varying vec3 v_color;"
        "uniform mat4 u_view_matrix;"
        "uniform mat4 u_projection_matrix;"
        "void main(){"
        "v_color = color;"
        "gl_Position =  u_projection_matrix * u_view_matrix * vec4(position, 1.0);"
        "}";

    char str_points_frag_shader[] =
        "varying vec3 v_color;"
        "void main() {"
        "gl_FragColor = vec4(v_color, 1.0);"
        "}";

    GLuint points_shader_program = compile_shader_program(str_points_vert_shader,
    													  str_points_frag_shader,
    													  "position", "color");
    GLuint uniform_view_matrix_location = glGetUniformLocation(points_shader_program, "u_view_matrix");
    GLuint uniform_projection_matrix_location = glGetUniformLocation(points_shader_program, "u_projection_matrix");


    // Benchmark value int number_particles = 10000000;
 	int number_particles = 100000;
	
	float* points_vertices;
	float* points_speed;
	float* points_colors;
	int ref_points_index = 0;

	points_vertices = (float*) malloc(number_particles * 3 * sizeof(float)); // multiply by three to have space for 3 floats each
	points_colors 	= (float*) malloc(number_particles * 3 * sizeof(float));
	points_speed 	= (float*) malloc(number_particles * 3 * sizeof(float));

	float3 speed_range;
	speed_range.x = 0.0001;
	speed_range.y = 0.0001;
	speed_range.z = 0.0001;

	float attenuate = 0.01;
	int idx = 0;
	for(int _i = 0; _i< number_particles; _i++) {
		points_vertices[idx]	 = 0;
		points_vertices[idx + 1] = 2;
		points_vertices[idx + 2] = 0;


		float3* rand_unit = get_cached_rand_in_unit_sphere();
		// Setup random color for each point initially
		points_colors[idx] 	   = rand_unit->x; m_randf(); //
		points_colors[idx + 1] = rand_unit->y; m_randf(); //
		points_colors[idx + 2] = rand_unit->z; m_randf(); //

		// Setup random velocities initially
		points_speed[idx] 	  = rand_unit->x * attenuate;
		points_speed[idx + 1] = rand_unit->y * attenuate;
		points_speed[idx + 2] = rand_unit->z * attenuate;

		idx+=3;
	}
	
	float gravity = 0.001;
    float time = 0;
    int refresh_camera = 1;
    float camera_speed = 2.0;

    int frame_count = 0;
    float diff_millis = 0;
    float diff_millis_accumulator = 0;

	glPointSize(4.0);
	glLineWidth(1.0);

	glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		start_timer(&general_timer1);

		if (refresh_camera) {
			update_camera(view_matrix, time, camera_speed, &camera_position, &camera_direction, &camera_up);
		}
	    
        update_particles(number_particles, gravity, points_vertices, points_speed);
		
		glUseProgram(points_shader_program);
		glUniformMatrix4fv(uniform_view_matrix_location, 1, GL_FALSE, view_matrix);
		glUniformMatrix4fv(uniform_projection_matrix_location, 1, GL_FALSE, projection_matrix);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, points_vertices);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, points_colors);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_POINTS, 0, number_particles);
    	glUseProgram(0);    

    	glUseProgram(points_shader_program);
		glUniformMatrix4fv(uniform_view_matrix_location, 1, GL_FALSE, view_matrix);
		glUniformMatrix4fv(uniform_projection_matrix_location, 1, GL_FALSE, projection_matrix);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, points_vertices);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, points_colors);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_LINES, 0, number_particles);
    	glUseProgram(0);    


		glfwSwapBuffers(window);
        glfwPollEvents();
        time +=0.001;
        ++frame_count;

        stop_timer(&general_timer1);
        diff_millis_accumulator+= compute_timer_millis_diff(&general_timer1);
        if ((frame_count % 4) == 0){
        	//printf("Millis avg: %f\n", diff_millis_accumulator/4);
        	diff_millis_accumulator=0;
        }
        
    }


    glfwMakeContextCurrent(NULL);
    glfwDestroyWindow(window);

	return 0;
}



GLuint compile_shader(GLenum type, const char *src) {
    GLuint shader;
    GLint compiled;
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar strInfoLog[infoLogLength];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
        printf("Compilation error in shader %s\n", strInfoLog);
        glDeleteShader(shader);
        return 0;
    }
    printf("Success!\n");
    return shader;
}

int compile_shader_program(const char* str_vert_shader, const char* str_frag_shader, const char* attrib_name_0, const char* attrib_name_1) {
    GLuint vert_shader;
    GLuint frag_shader;
    GLuint prog_object;

    vert_shader = compile_shader(GL_VERTEX_SHADER, str_vert_shader);
    if(vert_shader == 0) {
        d("Error compiling vert shader");
        return 1;
    }

    frag_shader = compile_shader(GL_FRAGMENT_SHADER, str_frag_shader);
    if(frag_shader == 0) {
        d("Error compiling frag shader");
        return 1;
    }

    d("Creating shader program");

    prog_object = glCreateProgram();
    glAttachShader(prog_object, vert_shader);
    glAttachShader(prog_object, frag_shader);

    if (attrib_name_0 != NULL) {
        d("Binding attrib 0");
        glBindAttribLocation(prog_object, 0, attrib_name_0);
    }

    if (attrib_name_1 != NULL) {
        d("Binding attrib 1");
        glBindAttribLocation(prog_object, 1, attrib_name_1);
    }

    d("Linking shader program");
    glLinkProgram(prog_object);

    return prog_object;
}
