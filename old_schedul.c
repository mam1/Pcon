/************************************************************************/
/*  schedule.c                                                          */
/*      Fuctions which support loading and saving schedules from a sd   */
/*      card.  Each (day of the week, channel)has a unique schedule.    */
/*      A schedule is an array of usingned int where the first int   d  */
/*      is the number of records in the schedule the following int      */
/*      indicate a time anf state transition for each channel.          */                                                       
/*                                                                      */
/*      Fuctions with support                                           */
/*      buffer to a sd card also CRUD for schedule records              */
/*                                                                      */
/*      A schedule record is a unsigned 32 bit value.  The high bit     */ 
/*      indicated the state (on or off) the lower bits contain the      */
/*                                                                      */
/*      record key (minutes from 00:00)                                 */
/************************************************************************/ 
    
#include <stdio.h>
#include <propeller.h>
#include <unistd.h>
#include "Pcon.h"
#include "schedule.h"
#include "bitlit.h" 
/****************************** externals *******************************/


/***************** global code to text conversion ********************/
extern char *day_names_long[7];     
extern char *day_names_short[7];
extern char *onoff[2];
extern char *con_mode[3];
extern char *sch_mode[2];
/************************** edit state variable *************************/

extern int     edit_channel,edit_day,edit_hour,edit_minute,edit_key;
/***************************** globals **********************************/
 uint32_t   state_mask = B32(10000000,00000000,00000000,00000000);
 uint32_t   key_mask   = B32(01111111,11111111,11111111,11111111);
 uint32_t   bbb[_SCHEDULE_BUFFER];
 char        fn_schedule[_SCHEDULE_NAME_SIZE] = "_F_PREFIX ## _FILE_SET_ID ## _F_SCHEDULE_SUFIX";
 FILE        *fptr_schedule;
/*******************************  fuctions *****************************/
 /* set all elements in a schedule buffer to 0 */
void clean_sch_buf(uint32_t *buf)
{
    int i;                                               
    for (i = 0; i < _SCHEDULE_BUFFER; ++i)
        *buf++ = '\0';
    return;
}
/* generate a file name and load it into target */
char *sch_file_name(char *t,int channel,int day)
{
    char        temp[128];
    // int         sch_num;

    // t = target;
    // sch_num = (channel*7)+day;
    strcpy(t,_F_PREFIX);
    strcat(t,_FILE_SET_ID);
    strcat(t,"d");
    sprintf(temp,"%d",day);
    strcat(t,temp);
    strcat(t,"c");
    sprintf(temp,"%d",channel);
    strcat(t,temp);
    strcat(t,_F_SCHEDULE_SUFIX);
    // printf("schedule file name <%s>\n",t);
    return t;
}
/* extract key from a schedule record */
int get_key(uint32_t *b)
{
    uint32_t     k;
    k = (int)(b & key_mask);
    return (int)k;
}
/* extract state from a schedule record */
int get_statex(uint32_t *b)
{
    if (b & state_mask)
        return 1;
    return 0;
}
/* load key into a schedule record */
void put_key(volatile uint32_t value,int key)
{
    int         hold_state;
    uint32_t    t;

    hold_state = get_statex(*value);
    t=(uint32_t)key;
    if(*value & state_mask) t |= state_mask;
    *value = key;
    put_state(value,hold_state);
    return;
}
/* load state into a schedule record */
void put_state(volatile uint32_t *b,int s)
{
    // printf("setting state to %i\n",s);
    if(s) *b |= state_mask;
    else  *b &= ~state_mask;
    return;
}
/* display all schedule records (schedule) for a (channel,day) */
void dump_sch_recs(volatile uint32_t *sch, int c, int d)
{
    int                         i,rsize;
    volatile uint32_t           *r;

    if(sch[c][0]==0)
    {
        printf("    no schedule records\n");
        return;   
    }

    rsize = sch[c][0];
    r = &(sch[c][1]);
    // printf("channel %i schedule for %s:\n",c,day_names_long[d-1]);
    for(i=0;i<rsize;i++)
    {
        printf("    %02i:%02i - %s\n",get_key(*r)/60,get_key(*r)%60,onoff[get_statex(*r)]);
        r++;
    }
        // printf("\n\n");    
    return;
}
/* write a schedule buffer to the sd card */
int save_schedule_data(volatile uint32_t *sbuf, int day)
{
    int             i, rsize,rcnt;
    char            name_buffer[_SCHEDULE_NAME_SIZE];
    FILE            *sfp;

    // printf("sbuf[2][0] = <%u>\n",sbuf[2][0]);
    // return 0;
        printf("### saving schedule data from <%x>\n",(int)sbuf);
    for(i=0;i<_NUMBER_OF_CHANNELS;i++)
    {                              
        sfp = fopen(sch_file_name(name_buffer,i,day),"w");
        if(sfp==NULL)
        {
            printf("**** problem opening schedule file <%s>\n",sch_file_name(name_buffer,day,i));
            fclose(sfp);
            return 1;    
        }
        printf(" <%s> opened for writing\n",sch_file_name(name_buffer,i,day));
        rsize = (int)sbuf[i][0];
        printf("    rsize = %i\n",rsize);
        if(sfp)
            {
                rcnt = fwrite(&sbuf[i][0],(rsize+1)*4,1,sfp);
                printf("    rcnt %i, rsize %i\n",rcnt,rsize);
                if(rcnt!=1)
                {   
                    printf("**** error writing schedule record\n");
                    return 1;
                }
                else
                fclose(sfp);
            }
        else
            {
                printf("**** error opening schedule file\n");
                return 1;
            }
        // usleep(1000000);
    }
    printf("schedule saved\n");
    return 0;
}


/* load a schedule buffer from the sd card */
int load_schedule_data(volatile uint32_t *sch,int day)
{
    int             i, rsize,rcnt;
    char            name_buffer[_SCHEDULE_NAME_SIZE];
    FILE            *sfp;

    printf("### loading schedule data to <%x>\n",(int)sch);
    for(i=0;i<_NUMBER_OF_CHANNELS;i++)
    {
        printf("attempting to open <%s)\n",sch_file_name(name_buffer,i,day));                              
        sfp = fopen(sch_file_name(name_buffer,i,day),"r");
        if(sfp==NULL)
        {
            printf("\nschedule file <%s> not found\n",sch_file_name(name_buffer,i,day));
            fclose(sfp);
            return 1;    
        }
        printf("  <%s> opened  ",sch_file_name(name_buffer,i,day));
        rcnt = fread(&sch[i][0],4,1,sfp);
        if(rcnt!=1)
            {   
                printf(" error reading count\n");
                return 1;
            }
        rsize = (int)sch[i][0];
        printf("    loading rsize = %i\n",rsize);
        fclose(sfp);
        sfp = fopen(sch_file_name(name_buffer,i,day),"r");
        if(sfp)
            {
                 rcnt = fread(&sch[i][0],(rsize*4)+4,1,sfp);
                 printf("    rcnt %i, rsize %i, v1 %i, v2 %i\n",rcnt,rsize,(int)sch[i][0],(int)sch[i][1]);
                if(rcnt!=1)
                {   
                    printf(" error reading schedule record\n");
                    return 1;
                }
                else
                fclose(sfp);
            }
        else
            {
                printf("error opening schedule file\n");
                return 1;
            }
    }
    return 0;
}
/* search a schedule for a record with the key, return pointer to record or NULL */
volatile uint32_t *find_schedule_record(volatile uint32_t *sch,int c,int k)
{
    int                              i, rsize;
    volatile uint32_t               *r;
    
    rsize = (int)sch[c][0];
    r = &sch[c][1];
    // printf("r1 <%x>\n",(unsigned int)r);
    for(i=0;i<rsize;i++)
    {
        // printf("get_key returns <%i>\n",get_key(*r));
        if(k==get_key(*r))
        {
            // printf("hit, k=%i\n",k);
            return r;
        }
        r++;
    }
    // printf("no hit, key=%i\n",k);
    return NULL;
}
/* delete a schedule record with matching key */
int del_sch_rec(volatile uint32_t *sch,int c, int k)
{
    volatile uint32_t       rsize;
    int                     i,hit;

    hit = 0;
    rsize = (int)sch[c][0];
    // r = &sch[c][1];
    for(i=0;i<rsize;i++)
    {
        if(k==get_key(sch[c][i+1]))
            hit = 1;
        if(hit)
            sch[c][i+1] = sch[c][i+2]; 
        // r++;    
    }
    if(hit)
    {
        sch[c][0] -= 1;
        return 0;
    }     
    return 1;
}
/* add or change a schedule record */
int add_sch_rec(volatile uint32_t *sch,int c, int k, int s)
{
    volatile uint32_t       rsize, *r;
    int                     i,ii;

    // printf("add_sch_rec called with <%x> <%i> <%i> <%i>\n",sch,c,k,s);

    if (r=find_schedule_record(sch,c,k))
    {
        put_state(r,s);
        return 0;
    }
    if((int)sch[c][0] >= _MAX_SCHEDULE_RECS)
    {
        printf("*** too many schedule records\n");
        return 1;
    }
    if((int)sch[c][0]==0)
    {
        sch[c][0] = 1;
        put_state(&sch[c][1],s);
        put_key(&sch[c][1],k);
        return 0;
    }
    i = 1;
    rsize = (int)sch[c][0];
    for(i=1;i<rsize+1;i++)
        if(k < get_key(sch[c][i]))
        {
            sch[c][0] += 1;
            for(ii=sch[c][0];ii>i;ii--)
                sch[c][ii] = sch[c][ii-1];
            put_state(&sch[c][i],s);
            put_key(&sch[c][i],k);
            return 0;       
        }
        sch[c][0] += 1;
        put_state(&sch[c][sch[c][0]],s);
        put_key(&sch[c][sch[c][0]],k);

    return 0;
}

int init_schedule_data(void)
{
    int         c,d;
    uint32_t    drec[3];
    char        name_buffer[30];
    FILE            *sfp;

    sfp = fopen(sch_file_name(name_buffer,1,1),"r");
    if(sfp)
    {
        // printf("schedule files found\n");
        fclose(sfp);
        return 0;    
    }
    drec[0] = 0;
    printf("creating schedule files\n");
    // printf("r0<%i>, 1<%i>, 2<%i>", drec[0], drec[1], drec[2]);
    for(c=0;c<_NUMBER_OF_CHANNELS;c++)
        for(d=0;d<_DAYS_PER_WEEK;d++)
        {
            // printf("channel %i, day %i, generated schedule name: %s\n",c,d,sch_file_name(name_buffer,c,d));
            sfp = fopen(sch_file_name(name_buffer,c,d),"w");
            printf(".");
            if(sfp)
            {
                if(fwrite(&drec,4,1,sfp)!=1)
                {
                    printf("error writing schedule record\n");
                    return 1;
                }
                fclose(sfp);
                continue;
            }
            else
                return 1;
        }
        printf("\n");
    return 0;
}




