#define _XOPEN_SOURCE 500

#include    "portaudio.h"
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>

#define SAMPLE_RATE 44100
#define SAMPLE_TIME (1.0f/(SAMPLE_RATE))
 
typedef struct {
  double freqHz;
  double phase;
} siner_t;

double
siner_update(siner_t * s, double time) {
  return sin(time * s->freqHz * 2 * M_PI + s->phase);
}

typedef struct {
  int64_t sample;
  int64_t count;
  double (** updaters)(void *, double);
  void ** data;
} status_t;
 
int sines(const void *ibuf, void *obuf, unsigned long framesperbuf,
    const PaStreamCallbackTimeInfo* timeinfo,
    PaStreamCallbackFlags statusflags, void * userdata) {

  status_t *data = (status_t *)userdata;
  float *out = obuf;

  for(unsigned long i = 0; i < framesperbuf; i++) {
      double time = ((data->sample + i) * SAMPLE_TIME);
      for(int64_t u = 0; u < data->count; u++) {
        float val = siner_update(&data->sine,  time);
      }
      *out++ = val;
      *out++ = val;
  }

  data->sample += framesperbuf;
  return 0;
}

int
main(int argc, char *argv[])
{
  PaError err = paNoError;
  if(paNoError != (err = Pa_Initialize())) {
    printf("PA Init failed: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  status_t data = {0, {880, 0}};

  PaStream *stream = NULL;
  if(paNoError != (err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, SAMPLE_RATE, 256, sines, &data))) {
    printf("PA Stream open failed: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  if(paNoError != (err = Pa_StartStream(stream))) {
    printf("PA Stream start failed: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  Pa_Sleep(1 * 1000);

  if(paNoError != (err = Pa_StopStream(stream))) {
    printf("PA Stream stop failed: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  if(paNoError != (err = Pa_CloseStream(stream))) {
    printf("PA Stream close failed: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  if(paNoError != (err = Pa_Terminate())) {
    printf("PA Term failed: %s\n", Pa_GetErrorText(err));
    return -1;
  }
  return 0;
}
