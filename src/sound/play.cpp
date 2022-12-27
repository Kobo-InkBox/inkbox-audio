#include <fstream>
#include <string>

// sound
#include <alsa/asoundlib.h>
#include <stdio.h>

#include "../functions.h"
#include "play.h"

using namespace std;

extern unsigned int rate;
extern unsigned int channels;

const string emitter = "play";

void playFile(string filePath) {
  // https://gist.github.com/ghedo/963382/
  log("Playing file: " + filePath, emitter);
  log("Channels:" + to_string(channels));
  log("Rate:" + to_string(rate));

  int pcm;
  snd_pcm_t *pcm_handle;
  snd_pcm_hw_params_t *params;
  snd_pcm_uframes_t frames;

  // Opening device
  pcm = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (pcm < 0)
    log("Failed to open device", emitter);

  // Set default parameters
  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(pcm_handle, params);

  // Apply them
  pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
                                     SND_PCM_ACCESS_RW_INTERLEAVED);
  if (pcm < 0)
    log("Failed set interleaved mode", emitter);

  pcm = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
  if (pcm < 0)
    log("Failed set format", emitter);

  pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels);
  if (pcm < 0)
    log("Failed set number of channels", emitter);

  pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0);
  if (pcm < 0)
    log("Failed set rate", emitter);

  // Write them
  pcm = snd_pcm_hw_params(pcm_handle, params);
  if (pcm < 0)
    log("Failed to write hardware parameters", emitter);

  // Some logs
  log("Sound card name: " + string(snd_pcm_name(pcm_handle)), emitter);
  log("Sound card state: " +
          string(snd_pcm_state_name(snd_pcm_state(pcm_handle))),
      emitter);

  snd_pcm_hw_params_get_channels(params, &channels);

  string mode;
  if (channels == 1) {
    mode = "mono";
  } else if (channels == 2) {
    mode = "stereo";
  }
  log("Current mode is: " + mode, emitter);

  snd_pcm_hw_params_get_rate(params, &rate, 0);
  log("Current rate: " + std::to_string(rate), emitter);

  // Actual playing

  // Get the sample rate (in Hz)
  snd_pcm_hw_params_get_period_size(params, &frames, 0);
  log("Frames are: " + to_string(frames), emitter);

  std::ifstream file(filePath, std::ios::binary);
  file.seekg(44, ios::beg);

  short buffer[channels * frames];
  while (file.read(reinterpret_cast<char *>(buffer), sizeof(buffer))) {
    snd_pcm_sframes_t framesWritten =
        snd_pcm_writei(pcm_handle, buffer, frames);
    // log("Frames written: " + to_string(framesWritten), emitter);
    if (framesWritten < 0) {
      log("An error?");
    }
  }

  snd_pcm_drain(pcm_handle);
  snd_pcm_close(pcm_handle);
}

// lack of special kernek
// those file are needed in toolchain add them
// git ignore too
