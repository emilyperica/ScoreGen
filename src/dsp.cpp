// https://github.com/libsndfile/libsndfile/blob/master/examples/sfprocess.c

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <memory>
#include <sndfile.h>

#define BUFFER_LEN 1024

// digital signal processing
int dsp(const char *file_name) {
	// open file
	static double data[BUFFER_LEN];
	SNDFILE *infile;
	SF_INFO file_info;
	int count;

	memset(&file_info, 0, sizeof(file_info));

	// read data
	if (!(infile = sf_open(file_name, SFM_READ, &file_info)))
	{	/* Open failed so print an error message. */
		printf("Failed to opend %s\n", file_name);
		/* Print the error message from libsndfile. */
		puts(sf_strerror(NULL));
		return 1;
	};

	// process data
	//while ((count = (int)sf_read_double(infile, data, BUFFER_LEN)))
	//{
	//	
	//};

	//const float *buffer = malloc(file_info.frames * file_info.channels * sizeof(float));
	std::vector<float> buffer(file_info.frames + file_info.channels);
	std::cout << buffer[1] << std::endl;

	sf_close(infile);

	return 0;
}
