#include "sndlib.h"
#include <stdlib.h>
class AudioFile{
    AudioFile(char * filename){
        int fd, chans, srate,frames;
        mus_long_t samples;
        float length;
        time_t date;
        char *comment;
        char timestr[64];
        mus_sound_initialize();	    /* initialize sndlib */
        fd = mus_file_open_read(filename); /* see if it exists */
        if (fd != -1)
        {
            close(fd);
            date = mus_sound_write_date(filename);
            srate = mus_sound_srate(filename);
            chans = mus_sound_chans(filename);
            samples = mus_sound_samples(filename);
            comment = mus_sound_comment(filename); 
            length = (double)samples / (float)(chans * srate);
            strftime(timestr, 64, "%a %d-%b-%y %H:%M %Z", localtime(&date));
            fprintf(stdout, "%s:\n  srate: %d\n  chans: %d\n  length: %f\n", 
	            filename, srate, chans, length);
            fprintf(stdout, "  type: %s\n  format: %s\n  written: %s\n  comment: %s\n", 
	            mus_header_type_name(mus_sound_header_type(filename)), 
	            mus_data_format_name(mus_sound_data_format(filename)), 
	            timestr, comment);
            
              frames = mus_sound_frames(argv[1]);
              outbytes = BUFFER_SIZE * chans * 2;
              bufs = (mus_sample_t **)calloc(chans, sizeof(mus_sample_t *));
              for (i=0;i<chans;i++) 
                bufs[i] = (mus_sample_t *)calloc(BUFFER_SIZE, sizeof(mus_sample_t));
              obuf = (short *)calloc(BUFFER_SIZE * chans, sizeof(short));
              afd = mus_audio_open_output(MUS_AUDIO_DEFAULT, srate, chans, MUS_AUDIO_COMPATIBLE_FORMAT, outbytes);
              if (afd != -1)
	        {
	          for (i = 0; i < frames; i += BUFFER_SIZE)
	            {
	              mus_sound_read(fd, 0, BUFFER_SIZE - 1, chans, bufs);
	              for (k = 0, j = 0; k < BUFFER_SIZE; k++, j += chans)
		        for (n = 0; n < chans; n++) 
                          obuf[j + n] = MUS_SAMPLE_TO_SHORT(bufs[n][k]);
	              mus_audio_write(afd, (char *)obuf, outbytes);
	            }
	          mus_audio_close(afd);
	        }
              mus_sound_close_input(fd);
              for (i = 0; i < chans; i++) free(bufs[i]);
              free(bufs);
              free(obuf);
        }
        else
        fprintf(stderr, "%s: %s\n", filename, strerror(errno));
        }
};