/*
 * Copyright (C) CSR plc 2011
 * Copyright 2010, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: marco.sinigaglia@csr.com for CSR plc
 */

#define LOG_TAG "V4l2_handler_Fm"
#define LOG_NDEBUG 1

#ifdef LINUX
#define ALOGI printf
#define ALOGE printf
#else
#include "utils/Log.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include "../libfmjni/android_fm.h"
#include "v4l2_ioctl.h"

//RDS
#define RDS_THREAD_ON                    1
#define RDS_THREAD_OFF                   0
#define BUFFER_RDS_SIZE                  300                // rds buff size
#define GROUP_SIZE                       8                  // group is composed by 2byte * 4 packets
#define GROUP_EMPTY                      0
#define GROUP_INCOMPLETE                 1
#define GROUP_0A                         0
#define GROUP_0B                         1
#define GETBYTE(blockNuber,byteNum)      blockNuber*2+byteNum
#define CLEAN_RDS                        last_block_num= -1; next_expected_block= 0; group_status = GROUP_EMPTY;
//Mute
#define DEFAULT_VOLUME                  255
#define MUTE_OFF                        0
#define MUTE_ON                         1
//Scan
#define SCAN_RUN                        1
#define SCAN_STOP                       0
#define LOCKTIME                        40000           // wait 40ms for card to lock on
#define MAX_FREQS                       50              // number of max freq buffer for a full scan
#define DEFAULT_THRESHOLD               500             // threshold for scan

/* session struct holded by the FM SE stack */
typedef struct fm_v4l2_data_t {
  int low_freq;
  int high_freq;
  int grid;
  int fd;
  int threshold;
  int freq;
  float fact;
  struct v4l2_tuner vt;
  pthread_t thread_rds;                                 /* thread used to read rds data */
  char scan_band_run;                                   /* flag to stop the scan*/
  char thread_rds_run;                                  /* flag to stop the rds thread*/
} fm_v4l2_data;


fm_v4l2_data* get_session_data(void **data) {return *data;}
int get_standard_freq(int freq, int fact) {return freq / fact;}
int get_proprietary_freq(int freq, int fact) {return freq * fact;}

void* th_read_rds(void *thread_rds_info);

static int v4l2_rx_start_func (void **data, int low_freq, int high_freq, int default_freq, int grid)
{
  char	*dev = DEFAULT_DEVICE;
  fm_v4l2_data* session;

  ALOGI("%s:\n", __FUNCTION__);
  ALOGI("low_freq %d, high_freq %d, default_freq %d, grid %d\n", low_freq, high_freq, default_freq, grid);

  session = malloc(sizeof(fm_v4l2_data));
  if (session== NULL){
      ALOGE("error on malloc");
      return -1;
  }

  memset(session, 0, sizeof(fm_v4l2_data));
  *data =  session;

  session->fd = open_dev(dev);
  if (session->fd < 0){
      ALOGE("error on open dev\n");
      return -1;
  }

  if (get_tun_radio_cap(session->fd) !=1) {
      ALOGE("error on check tunner radio capability");
      return -1;
  }

  session->vt.index=0;
  if ( get_v4l2_tuner(session->fd,  &session->vt) <0 ){
      ALOGE("error on get V4L tuner\n");
      return -1;
  }

  session->fact = get_fact(session->fd, &session->vt);
  if ( session->fact < 0) {
      ALOGE("error on get fact\n");
      return -1;
  }

  session->freq = get_proprietary_freq(default_freq, session->fact);
  session->low_freq =  get_proprietary_freq(low_freq,  session->fact);
  session->high_freq =  get_proprietary_freq(high_freq,  session->fact);
  session->grid = get_proprietary_freq(grid,  session->fact);
  session->threshold = DEFAULT_THRESHOLD;
  
  if (set_freq(session->fd, session->freq) < 0 ){
      ALOGE("error on set freq\n");
      return -1;
  }

  if (set_volume(session->fd, DEFAULT_VOLUME)<0){
      ALOGE("error on set volume\n");
      return -1;
  }

  if (set_mute(session->fd, MUTE_OFF) <0){
      ALOGE("error on mute\n");
      return -1;
  }

  return 0;
}

int v4l2_reset(void** session_data)
{
  fm_v4l2_data* session;
  int ret;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  ret = set_mute(session->fd, MUTE_ON);
  if (ret < 0)
    return -1;

  ret = close(session->fd);
  if (ret < 0)
    return -1;

  free(session);
  return 0;
}

int v4l2_pause(void** session_data){
  fm_v4l2_data* session;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  return set_mute(session->fd, MUTE_ON);
}

int v4l2_resume(void** session_data){
  fm_v4l2_data* session;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  return set_mute(session->fd, MUTE_OFF);
}

int v4l2_mute(void** session_data, int mute){
  fm_v4l2_data* session;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  return set_mute(session->fd, mute);
}


int v4l2_set_frequency(void** session_data, int frequency){
  fm_v4l2_data* session;
  int ret;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  session->freq = get_proprietary_freq( frequency, session->fact);
  ret= set_freq(session->fd,  session->freq);
  if (ret < 0)
      return -1;

  return frequency;
}

int v4l2_get_frequency (void ** session_data){
  fm_v4l2_data* session;
  int ret;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  ret = get_freq(session->fd);
  if (ret < 0)
      return -1;

  session->freq = get_standard_freq(ret, session->fact);

  return session->freq;
}

int v4l2_get_threshold (void ** session_data){
  fm_v4l2_data* session;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  return session->threshold;
 }

int v4l2_set_threshold (void ** session_data, int threshold){
   fm_v4l2_data* session;

   ALOGI("%s:\n", __FUNCTION__);
   session = get_session_data(session_data);

   session->threshold = threshold;
   return 0;
}

int v4l2_get_signal_strength (void ** session_data){
    fm_v4l2_data* session;
    int ret;

    ALOGI("%s:\n", __FUNCTION__);
    session = get_session_data(session_data);

    ret = get_signal_strength(session->fd, &session->vt);
    return ret;
}

int v4l2_is_playing_in_stereo (void ** session_data){
    fm_v4l2_data* session;
    int ret;

    ALOGI("%s:\n", __FUNCTION__);
    session = get_session_data(session_data);

    ret = get_stereo(session->fd, &session->vt);
    return ret;
}

int v4l2_scan (void ** session_data, enum fmradio_seek_direction_t direction){
   fm_v4l2_data* session;
   int increment, rate, freqi, ret;

   ALOGI("%s:\n", __FUNCTION__);
   session = get_session_data(session_data);

   session->scan_band_run=SCAN_RUN;
   if (direction==FMRADIO_SEEK_DOWN)
     increment = -  session->grid;
   else                                      //FMRADIO_SEEK_UP
     increment =  session->grid;

  freqi = session->freq + increment;         //drop the current frequency
  ALOGI("Starting scanning...\n");
  while (session->scan_band_run==SCAN_RUN){

      ret= set_freq(session->fd, freqi);
      if (ret<0)
          return -1;

      usleep(LOCKTIME);                       // let it lock on
      rate= get_signal_strength( session->fd,  &session->vt);
      if (rate < 0)
         return -1;

      ALOGI("final rate %d > %d \n", rate , session->threshold);

      if (rate >  session->threshold){
      ALOGI("Found freq, %d\n", freqi);
      session->scan_band_run=SCAN_STOP;
      session->freq = freqi;
      return get_standard_freq(freqi, session->fact);
      }

      freqi +=  increment;
      if ( freqi >  session->high_freq)
         freqi =  session->low_freq;
      if ( freqi <  session->low_freq)
          freqi =  session->high_freq;
  }
  ALOGI("End scan\n");
  return 0;
}

int v4l2_full_scan (void ** session_data, int ** found_freqs, int ** signal_strenghts){
  fm_v4l2_data* session;
  int founded, i, ret;
  int freqi, rate;
  int *temp_freq, *temp_strenght;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  session->scan_band_run=SCAN_RUN;

  temp_freq = (int *) malloc(sizeof(int) *MAX_FREQS);
  temp_strenght = (int *) malloc(sizeof(int) *MAX_FREQS);
  if (temp_freq == NULL || temp_strenght==NULL){
    ALOGE("error on allocate");
    return -1;
  }

  ALOGI("Starting full scanning...low freq:%d, high freq:%d\n", session->low_freq, session->high_freq);

  for (freqi =  session->low_freq, founded=0 ; ((freqi < session->high_freq)  && (founded < MAX_FREQS) && (session->scan_band_run==SCAN_RUN)) ; freqi += session->grid){

      ret = set_freq(session->fd, freqi);
      if (ret<0)
         return -1;

      usleep(LOCKTIME);		/* let it lock on */
      rate= get_signal_strength(session->fd, &session->vt);
      if (rate < 0)
          return -1;

      ALOGI("final rate %d > %d \n", rate ,session->threshold);

      if (rate > session->threshold){
          ALOGI("Founded index %d, freq %d\n", founded, freqi);
          temp_freq[founded]= freqi;
          temp_strenght[founded]= rate;
          founded++;
      }
  }

  *found_freqs = (int *) malloc(sizeof(int) * founded);
  *signal_strenghts = (int *) malloc(sizeof(int) * founded);
  if (*found_freqs == NULL || *signal_strenghts==NULL){
    ALOGE("error on allocate");
    return -1;
  }

  //copy founded frequencies
  for (i=0; i<founded; i++){
      (*found_freqs)[i] = get_standard_freq(temp_freq[i], session->fact);
      (*signal_strenghts)[i] = temp_strenght[i];
      ALOGI("Copied index %d, freq %d, signal %d\n",i, (*found_freqs)[i], (*signal_strenghts)[i] );
  }

  ALOGI("End full scan\n");
  free(temp_freq);
  free(temp_strenght);
  session->scan_band_run=SCAN_STOP;

  return i;
}

int v4l2_stop_scan(void ** session_data){
  fm_v4l2_data* session;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);
  session->scan_band_run=SCAN_STOP;
  ALOGI("Stop scan value is %d\n", session->scan_band_run);

  return 1;
}

int v4l2_set_force_mono (void ** session_data, int force_mono){
  fm_v4l2_data* session;
  int ret;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  ret = set_force_mono(session->fd, &session->vt, force_mono);
  return ret;
}

int v4l2_is_rds_data_supported (void ** session_data){
  fm_v4l2_data* session;
  int ret;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  ret = get_RDS_cap(session->fd);
  return ret;
}

int v4l2_is_tuned_to_valid_channel (void ** session_data){
  fm_v4l2_data* session;
  int signal;

  ALOGI("%s:\n", __FUNCTION__);
  session = get_session_data(session_data);

  signal = get_signal_strength(session->fd, &session->vt);
  if ( signal > session->threshold )
    return 1;
  else
    return 0;
}

int v4l2_get_rds(void * * session_data, struct fmradio_rds_bundle_t * fmradio_rds_bundle) {
  int ret = 1;
  int bytesNum, i, j, blocknum, last_block_num, next_expected_block, group_status, group_type;
  char * buf, * group;
  char b0, b1, b2, b1_l5;

  int index;
  fm_v4l2_data * session = get_session_data(session_data);

  buf = malloc(sizeof(char) * BUFFER_RDS_SIZE);
  group = malloc(sizeof(char) * GROUP_SIZE);
  CLEAN_RDS

  bytesNum = read(session -> fd, buf, BUFFER_RDS_SIZE);
  if (bytesNum == -1 && errno == EINTR) {
    ALOGE("Error on RDS read\n");
    ret = 2;
  }


  if (bytesNum > 0) {
    ALOGI("read num bytes %d\n", bytesNum); //assuming to have always multiple of 3

    for (i = 0; i < bytesNum; i += 3) {
      b0 = buf[i];
      b1 = buf[i + 1];
      b2 = buf[i + 2];

      if ((b2 & 0x80) != 0) {
        CLEAN_RDS
        continue;
      }

      blocknum = b2 & 0x07; // What's the differnce between "Received Offset"

      if (blocknum == 4) blocknum = 2; // Treat C' as C
      if ((blocknum == 5) || (blocknum == 6)) continue; // ignore E Blocks

      if (blocknum == 7) { //invalid block
        CLEAN_RDS
        continue;
      }

      if (blocknum == last_block_num) continue;

      if ((group_status == GROUP_EMPTY) && (blocknum != 0)) continue;

      if (blocknum != next_expected_block) {
        CLEAN_RDS
        continue;
      }

      if (blocknum == 1) {
        group_type = (b1 >> 3);
      }

      group[2 * blocknum] = b0;
      group[2 * blocknum + 1] = b1;
      group_status = GROUP_INCOMPLETE;

      last_block_num = blocknum;
      next_expected_block = blocknum + 1;

      if (next_expected_block <= 3) continue;

      // PI code in block 0:
      fmradio_rds_bundle->pi = (group[GETBYTE(0, 1)] << 8) | group[GETBYTE(0, 0)];

      //some other info common in all groups:
      fmradio_rds_bundle->tp = group[GETBYTE(1, 1)] & 0x04;
      fmradio_rds_bundle->pty = (((group[GETBYTE(1, 1)] << 3) & 0x18) | ((group[GETBYTE(1, 0)] >> 5) & 0x07));
      // special
      b1_l5 = (group[GETBYTE(1, 0)] & 0x1F);

      switch (group_type) {
      case GROUP_0A:
      case GROUP_0B:

        fmradio_rds_bundle->ta = (b1_l5 & 0x10);
        fmradio_rds_bundle->ms = (b1_l5 & 0x08);

        index = (b1_l5 & 0x03) << 1;

        if (isprint(group[GETBYTE(3, 1)]) && isprint(group[GETBYTE(3, 0)])) {
          fmradio_rds_bundle->psn[index] = group[GETBYTE(3, 1)];
          fmradio_rds_bundle->psn[index + 1] = group[GETBYTE(3, 0)];
        } else {
          ret = 3;
          break;
        }

        if (index == 6) {
          fmradio_rds_bundle->psn[8] = '\0';
          ALOGI("Event Rds called!\n");
          ret = 4;
        } else {
          ret = 0;
        }
        break;
      }
      CLEAN_RDS
    }
  }

  free(buf);
  free(group);

  return ret;
}

int register_fmradio_functions(long *signature, struct fmradio_vendor_methods_t *vendor_methods)
{
    memset(vendor_methods, 0, sizeof(vendor_methods));

    vendor_methods->set_frequency = v4l2_set_frequency;
    vendor_methods->get_frequency = v4l2_get_frequency;
    vendor_methods->get_threshold = v4l2_get_threshold;
    vendor_methods->set_threshold = v4l2_set_threshold;
    vendor_methods->is_rds_data_supported = v4l2_is_rds_data_supported;
    vendor_methods->scan = v4l2_scan;
    vendor_methods->is_tuned_to_valid_channel = v4l2_is_tuned_to_valid_channel;
    vendor_methods->full_scan = v4l2_full_scan;
    vendor_methods->stop_scan = v4l2_stop_scan;
    vendor_methods->is_playing_in_stereo = v4l2_is_playing_in_stereo;
    vendor_methods->get_signal_strength = v4l2_get_signal_strength;
    vendor_methods->rx_start = v4l2_rx_start_func;
    vendor_methods->pause=v4l2_pause;
    vendor_methods->resume=v4l2_resume;
    vendor_methods->reset=v4l2_reset;
    vendor_methods->set_force_mono=v4l2_set_force_mono;
    vendor_methods->mute=v4l2_mute;
    vendor_methods->get_rds=v4l2_get_rds;

    *signature = FMRADIO_SIGNATURE;
    return 0;
}
