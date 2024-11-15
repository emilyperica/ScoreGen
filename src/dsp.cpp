#include <iostream>
#include <sndfile.h>
#include "detectPitch.h"

/* Length of buffer used to hold frames being processed */
#define BUFFER_SIZE 4096

/* Digital Signal Processing */
int dsp(char const* infilename) 
{
	int readcount;
	SNDFILE *infile;
	/* Initialize SF_INFO struct before use. */
    SF_INFO sfinfo;
	memset(&sfinfo, 0, sizeof(sfinfo));

	/* Open the SNDFILE in read mode, pass pointer for SF_STRUCT for 
	libsndfile to fill with information about the audio file. */
    if (!(infile = sf_open (infilename, SFM_READ, &sfinfo)))
	{	/* Open failure, print error message. */
		printf("Not able to open input file %s.\n", infilename) ;
		/* Print libsndfile error message. */
		puts(sf_strerror (NULL));
		return 1;
	}

	/* Initialize buffer, allocate memory */
    float *buf = (float *)malloc(BUFFER_SIZE * sizeof(float));
    if (!buf)
	{
		/* Memory allocation failure, print an error message, close the 
		opened SNDFILE */
        fprintf(stderr, "Memory allocation failed.\n");
        sf_close(infile);
        return 1;
    }

	/* Read in the data from the SNDFILE, pass into buffer. readcount will store the 
	number of frames/samples being passed in, once all time-domain values have been read,
	0 will be returned */
	while ((readcount = (int)sf_read_float(infile, buf, BUFFER_SIZE)))
	{
		detectPitch(buf, readcount, sfinfo.samplerate);
	}

	free(buf);
	sf_close(infile);

	return 0;
}