#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

typedef struct
{
	float time;
	float value;
} DataPoint;




int main(int argc, char ** argv)
{
	assert(argc == 3);
	
	int bufsiz = atoi(argv[1]);
	int numpoints = atoi(argv[2]);
	assert(bufsiz > 0);
	DataPoint * buffer = (DataPoint*)(calloc(bufsiz, sizeof(DataPoint)));



	FILE * file = fopen("data.bin", "wb");

	struct timeval start_time;
	gettimeofday(&start_time, NULL);
	
	int i = 0;
	while (i < numpoints)
	{
		int j = 0;
		for (j = 0; j < bufsiz && i < numpoints; ++j)
		{
			buffer[j].time = i;
			buffer[j].value = i;
			
		}
		i += j;
		assert(fwrite(buffer, sizeof(DataPoint), j, file) == j);
		

	}

	struct timeval end_time;
	gettimeofday(&end_time, NULL);


	fclose(file);
	free(buffer);
	float time_elapsed = (float)(end_time.tv_sec - start_time.tv_sec) + 1e-6*(end_time.tv_usec - start_time.tv_usec);
	printf("%f\n", time_elapsed);
	return 0;
}
