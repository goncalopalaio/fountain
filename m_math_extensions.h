float rand_float_range(float min, float max) {
	return min + (max-min) * m_randf();
}

float3 rand_in_unit_sphere() {
    float3 res;
    float len = 0;
    do {
        res.x = 2.0 * m_randf() - 1;
        res.y = 2.0 * m_randf() - 1;
        res.z = 2.0 * m_randf() - 1;
        //squared length
        len = res.x*res.x + res.y*res.y + res.z*res.z;
    } while(len >= 1.0);

    return res;
}

float radians_to_degrees(float radians) {
   return radians * 180/M_PI;
}

float degrees_to_radians(float degrees) {
    return degrees * M_PI / 180;
}

float3 make_float3(float x, float y, float z) {
	float3 t;
	t.x = x;
	t.y = y;
	t.z = z;
	return t;
}

void set_float3(float3* v, float x, float y, float z) {
    v->x = x;
    v->y = y;
    v->z = z;
}

void d(char *s) {
    printf("%s\n", s);
}
void d3(char *s, float3 f) {
    printf("%s %f %f %f\n", s,f.x,f.y,f.z);
}
void dm(char*name, float matrix[]) {
    int index = 0;
    printf("%s:\n", name);
    printf("%f %f %f %f\n", matrix[index+0], matrix[index+1],matrix[index+2],matrix[index+3]);
    index += 4;
    printf("%f %f %f %f\n", matrix[index+0], matrix[index+1],matrix[index+2],matrix[index+3]);
    index += 4;
    printf("%f %f %f %f\n", matrix[index+0], matrix[index+1],matrix[index+2],matrix[index+3]);
    index += 4;
    printf("%f %f %f %f\n", matrix[index+0], matrix[index+1],matrix[index+2],matrix[index+3]);
}

// @todo move this out of here
#ifdef __MACH__

#include <sys/time.h>
#define CLOCK_MONOTONIC 0
int clock_gettime(int clock_id, struct timespec* t);

int clock_gettime(int clock_id, struct timespec* t){
	//this is a straigthforward implementation of clock_gettime since is not implemented on OSX
	//original function by Dmitri Bouianov - http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
	//might me ineficient of inacurate when compared to the original implementation - gettimeofday is deprecated
	struct timeval now;
	int rv = gettimeofday(&now, NULL);
	if(rv) return rv;//gettimeofday returns 0 for success
	t->tv_sec = now.tv_sec;
	t->tv_nsec = now.tv_usec * 1000;
	return 0;
}	
#endif

typedef struct g_timer{
	struct timespec start_time;
	struct timespec end_time;
}g_timer;

void start_timer(g_timer* counter) {
	clock_gettime(CLOCK_MONOTONIC, &counter->start_time);
}

void stop_timer(g_timer* counter) {
	clock_gettime(CLOCK_MONOTONIC, &counter->end_time);
}

float compute_timer_millis_diff(g_timer* counter) {
	const float p10_to_minus6 = pow(10,-6);
	long diff_sec  =counter->end_time.tv_sec - counter->start_time.tv_sec;
	long diff_nano =counter->end_time.tv_nsec - counter->start_time.tv_nsec;
	long diff_millis = diff_sec * 1000 + diff_nano * p10_to_minus6;
	return diff_millis;
}

void stop_print_timer(const char* message, g_timer* counter) {
	stop_timer(counter);
	float diff_millis= compute_timer_millis_diff(counter);
	printf("%s millis %f \n",message, diff_millis);
}

int compute_timer_fps(g_timer* counter) {
	int end_seconds = counter->end_time.tv_sec;
	int start_seconds = counter->start_time.tv_sec;
	int end_nanosec = counter->end_time.tv_nsec;
	int start_nanosec = counter->start_time.tv_nsec;

	int diff = 1000000000L * (end_seconds - start_seconds) + end_nanosec - start_nanosec;
	int fps =  1/(diff / 1000000000.0);
	return fps;
}


// CPU cycle profiling

/*
 * Returns the processor time stamp. The processor time stamp records the number of clock cycles since the last reset
*/
//#include <stdint.h>
#ifdef _WIN32
#include <intrin.h>
uint64_t rdtsc(){
    return __rdtsc();
}
#else
// Linux/GCC
uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
#endif

struct debug_record {
	char tag[17];
	char* file_name;
	int line_number;
	uint64_t clocks;
};

struct cpu_timestamp {
	struct debug_record record;

	cpu_timestamp(int line_number, char* file_name,const char tag[17] ) {
		// __FUNCTION__ is defined by the compiler not the preprocessor. Copying the string is the workaround i found.
		strcpy(record.tag, tag);
		record.file_name = file_name;
		record.line_number = line_number;
		
		record.clocks = - rdtsc();
	}

	~cpu_timestamp() {
		record.clocks += rdtsc();
		printf("ln: %d %s %s clocks: %lld \n", record.line_number, record.file_name, record.tag, record.clocks);
	}
};
#define TIMED_BLOCK struct cpu_timestamp temp_cpu_timestamp(__LINE__, __FILE__, __FUNCTION__)
