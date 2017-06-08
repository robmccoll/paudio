#define _XOPEN_SOURCE 500

#include    "portaudio.h"
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>

#define SAMPLE_RATE 44100
#define SAMPLE_TIME (1.0f/(SAMPLE_RATE))

typedef struct {double l; double r; } steriodbl;
typedef  steriodbl (*updater_t)(void *, double);
 
typedef struct {
  double freqHz;
  double phase;
  double panning;
} siner_t;

steriodbl
siner_update(void * v, double time) {
  siner_t * s = (siner_t *)v;
  double val = sin(time * s->freqHz * 2 * M_PI + s->phase);
  return (steriodbl){s->panning * val, (1.0 - s->panning) * val};
}

typedef struct {
  int64_t sample;
  int64_t count;
  updater_t *updaters;
  void **data;
} status_t;
 
int sines(const void *ibuf, void *obuf, unsigned long framesperbuf,
    const PaStreamCallbackTimeInfo* timeinfo,
    PaStreamCallbackFlags statusflags, void * userdata) {

  status_t *data = (status_t *)userdata;
  float *out = obuf;

  for(unsigned long i = 0; i < framesperbuf; i++) {
      double time = ((data->sample + i) * SAMPLE_TIME);
      steriodbl val = {0, 0};
      for(int64_t u = 0; u < data->count; u++) {
        steriodbl cur = data->updaters[u](data->data[u], time);
        val.l += cur.l;
        val.r += cur.r;
      }
      *out++ = val.l / data->count;
      *out++ = val.r / data->count;
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

  siner_t sine1 = {660, 0, 0};
  siner_t sine2 = {440, 0, 1};
  updater_t * updaters = (updater_t[]){siner_update, siner_update};
  void ** data = (void *[]){&sine1, &sine2};
  status_t status = {0, 2, updaters, data};

  PaStream *stream = NULL;
  if(paNoError != (err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, SAMPLE_RATE, 256, sines, &status))) {
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
