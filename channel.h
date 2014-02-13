#ifndef CHANNEL_H_
#define CHANNEL_H_

int init_channel_data(void);
void disp_channel_data(int);    //0-7: disp data for channel, -1: disp data all channels
int save_channel_data(void);
int load_channel_data(void);
char *channel_file_name(char *);    // *buffer -> *buffer
int set_channel_state(int);
int set_channel_control_mode(int);
int set_channel_schedule_mode(int);
int set_channel_name(char *);

#endif