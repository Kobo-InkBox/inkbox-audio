#include <fstream>
#include <string>

// sound
#include <alsa/asoundlib.h>
#include <stdio.h>

#include "../functions.h"
#include "play.h"

// libsndfile
#include <sndfile.h>
#include <unistd.h>

using namespace std;

extern bool pausePlay;
extern unsigned int fileOffsetPause;
extern bool continuePlay;
extern string recentFile;
extern bool isPLaying;
extern bool threadToBeJoined;
extern bool canBeContinued;
extern mutex overAllMutex;

// config
extern string mixerName;

const string emitter = "play";

void playFile(string filePath) {
  // https://gist.github.com/ghedo/963382/
  log("Playing file: " + filePath, emitter);
  recentFile = filePath;

  int pcm;
  snd_pcm_t *pcm_handle;
  snd_pcm_hw_params_t *params;
  snd_pcm_uframes_t frames;

  // Opening device
  pcm = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (pcm < 0)
    log("Failed to open device", emitter);

  // Get default parameters
  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(pcm_handle, params);

  // Get informations from wav file using library
  unsigned int rate;
  unsigned int channels;
  SF_INFO sfinfo;
  SNDFILE *wavFile = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
  rate = sfinfo.samplerate;
  channels = sfinfo.channels;
  sf_close(wavFile);

  log("Informations from wav file:", emitter);
  log("Rate:" + to_string(rate), emitter);
  log("Channels:" + to_string(channels), emitter);

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
  file.seekg(44, ios::beg); // meta info
  overAllMutex.lock();
  if (continuePlay == true) {
    continuePlay = false;
    file.seekg(fileOffsetPause, ios::beg);
  } else {
    fileOffsetPause = 44;
    file.seekg(44, ios::beg);
  }
  overAllMutex.unlock();

  short buffer[channels * frames];
  while (file.read(reinterpret_cast<char *>(buffer), sizeof(buffer))) {
    snd_pcm_sframes_t framesWritten =
        snd_pcm_writei(pcm_handle, buffer, frames);

    fileOffsetPause = fileOffsetPause + sizeof(buffer);
    overAllMutex.lock();
    if (pausePlay == true) {
      log("Stopping playing because of request", emitter);
      overAllMutex.unlock();
      // Stop now
      snd_pcm_drop(pcm_handle);
      break;
    }
    overAllMutex.unlock();

    if (framesWritten < 0) {
      log("Error while playing audio", emitter);
    }
  }

  overAllMutex.lock();
  if (pausePlay == true) {
    pausePlay = false;
    canBeContinued = true;
  } else {
    // Wait for it to finish
    snd_pcm_drain(pcm_handle);
    threadToBeJoined = true;
  }

  snd_pcm_close(pcm_handle);
  isPLaying = false;
  overAllMutex.unlock();
  while (true) {
    log("Waiting for thread to be started joining", emitter);
    sleep(1);
    overAllMutex.lock();
    if(threadToBeJoined == false) {
      overAllMutex.unlock();
      break;
    }
    overAllMutex.unlock();
  }
  // Lazy thread management and free delay for music
  sleep(2);
  return void();
}

// Should be from 0 - 100, if the sound card is weird, then implement a
// converter. I don't need to
void setVolumeLevel(int level) {
  log("Setting volume to " + to_string(level));
  // Open the mixer
  snd_mixer_t *mixer;
  snd_mixer_open(&mixer, 0);

  // Attach the mixer to the default sound card
  snd_mixer_attach(mixer, "default");

  // Register the mixer
  snd_mixer_selem_register(mixer, NULL, NULL);

  // Load the mixer elements
  snd_mixer_load(mixer);

  // For every device mixerName is diffrent, so its set in config file
  snd_mixer_elem_t *elem = snd_mixer_first_elem(mixer);
  while (elem != NULL) {
    log("Mixer: " + string(snd_mixer_selem_get_name(elem)), emitter);
    if (snd_mixer_selem_is_active(elem) &&
        snd_mixer_selem_get_name(elem) == mixerName) {
      break;
    }
    elem = snd_mixer_elem_next(elem);
  }

  // Set the volume level of the "Master" mixer element
  long min, max;
  snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
  log("Min volume: " + to_string(min));
  log("Max volume: " + to_string(max));

  snd_mixer_selem_set_playback_volume_all(elem, level);

  // Close the mixer
  snd_mixer_close(mixer);
}
