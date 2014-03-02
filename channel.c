 #include <stdio.h>
#include <propeller.h>
#include <unistd.h>
#include "Pcon.h"
/********************* externals ********************/
//    extern CCR  cca[_NUMBER_OF_CHANNELS];
extern char        *file_set_prefix[_SCHEDULE_FILE_NAME_SIZE];

extern int         active_channel;

extern struct {     // control block & stack for dio cog 
    unsigned stack[_STACK_SIZE_DIO];
    volatile DIO_CB dio;
} dio_cb;

/********************** globals *********************/
    char        fn_channel[_CHANNEL_FILE_NAME_SIZE];
    FILE        *fptr_channel;
/***************** global code to text conversion ********************/
extern char *day_names_long[7];     
extern char *day_names_short[7];
extern char *onoff[2];
extern char *con_mode[3];
extern char *sch_mode[2]; 
/****************** start channel support fuctions *************************/
void disp_channel_data(int cn)
{
    int     i;
    if(cn<0)
    {
        for(i=0;i<_NUMBER_OF_CHANNELS;i++)
            printf("channel %i:  state <%s>, mode <%s>, name <%s>\n",
                i, 
                onoff[dio_cb.dio.cca[i].state], 
                con_mode[dio_cb.dio.cca[i].c_mode],
                dio_cb.dio.cca[i].name);
        printf("\n");
        return;
    }       
    printf("channel %i:  state <%s>, mode <%s>, name <%s>\n",
        i, 
        onoff[dio_cb.dio.cca[i].state], 
        con_mode[dio_cb.dio.cca[i].c_mode],
        dio_cb.dio.cca[i].name);
    printf("\n");
    return;
}
int load_channel_data(void)
{
    fptr_channel = fopen(fn_channel,"r");
    if(fptr_channel)
      {
        int h = fread(dio_cb.dio.cca,sizeof(dio_cb.dio.cca),1,fptr_channel);
        if(h != 1)
        {
            fclose(fptr_channel);
            printf("*** bad read ***\nfread returned %i\n",h);
            return 1;
        }
        else
        {
            fclose(fptr_channel);
            return 0;
        }
      }
    else
    {
        printf("*** bad open ***\nfile name <%s>\n",fn_channel);
        return 1;
    }
}
int save_channel_data(void)
{
    fptr_channel = fopen(fn_channel,"w");
    if(fptr_channel)
      {
        int h = fwrite(dio_cb.dio.cca,sizeof(dio_cb.dio.cca),1,fptr_channel);
        if(h != 1)
        {
            fclose(fptr_channel);
            printf("*** bad write ***\nfwrite returned %i\n",h);
            return 1;
        }
        else
        {
            fclose(fptr_channel);
            return 0;
        }
      }
    else
    {
        printf("*** bad open ***\n");
        return 1;
    }
}
char *channel_file_name(char *cfn)
{
    char *ptr;
    strcpy(cfn,_F_PREFIX);
    ptr = cfn + sizeof(_F_PREFIX)- 1;
    strcpy(ptr,_F_CHANNEL_SUFIX);
    return cfn;
}

int init_channel_data(void)
{
    strcat(fn_channel,file_set_prefix);
    strcat(fn_channel,_F_CHANNEL_SUFIX);
    printf("channel file name <%s>\n",fn_channel);
    fptr_channel = fopen(fn_channel,"r");
    if(fptr_channel)
        printf("channel file found\n")
        ;
    else
    {
        printf("*** channel file <%s> does not exsit,it will be created\n",fn_channel);
        fptr_channel = fopen(fn_channel,"w");
        if(fptr_channel)
        {
            printf("channel file <%s> created\n",fn_channel);
            if(fwrite(dio_cb.dio.cca,sizeof(dio_cb.dio.cca),1,fptr_channel)!=1)
            {   
                printf("\n\n**** can't write channel file\n");
                printf("**** aborting ***** \n");
                return 1;
            }
            printf("channel file <%s> intialized\n",fn_channel);
            fseek(fptr_channel,0,SEEK_SET);
        }
        else
        {
            printf("\n\n**** can't create channel file\n");
            printf("**** aborting ***** \n");
            return 1;
        }        
    }
    fclose(fptr_channel);
    return 0;
}
int set_channel_state(int s)
{
    // printf("setting channel state to <%i>, active_channel <%i>\n",s,active_channel);
    if(s==0 || s==1)
    {
        dio_cb.dio.cca[active_channel].state = s;
        // printf("dio_cb.dio.cca[4].state = %i\n",dio_cb.dio.cca[active_channel].state );
        return 0;
    }
    printf("** invalid channel state, state must be 0 or 1\n");
    return 1;
}
int set_channel_control_mode(int m)
{
    if(m==0 || m==1 || m==2)
    {
        dio_cb.dio.cca[active_channel].c_mode = m;
        return 0;
    }
    printf("** invalid channel mode, mode must be 0, 1, or 2\n");
    return 1;
}
/*
int set_channel_schedule_mode(int m)
{
    if(m==0 || m==1)
    {
        dio_cb.dio.cca[active_channel].s_mode = m;
        return 0;
    }
    printf("** invalid schedule mode, mode must be 0 or 1\n");
    return 1;
}
*/
int set_channel_name(char *n)
{
    int             sl;

    sl = strlen(n);
    if((sl) > _CHANNEL_NAME_SIZE)
    { 
        printf("** channel name truncated\n");
        sl = _CHANNEL_NAME_SIZE;
    }
    n[sl-1]='\0';
    strcpy(dio_cb.dio.cca[active_channel].name,++n);
    return 0;
}

/********************** end channel support fuctions *************************/
