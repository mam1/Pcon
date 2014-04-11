/************************************************************************/
/*  schedule.c                                                          */
/*      Functions which support loading and saving schedules from a sd   */
/*      card.  Each (day of the week, channel)has a unique schedule.    */
/*      A schedule is an array of unsigned int where the first int   d  */
/*      is the number of records in the schedule the following int      */
/*      indicate a time and state transition for each channel.          */                                                       
/*                                                                      */
/*      Functions with support                                           */
/*      buffer to a sd card also CRUD for schedule records              */
/*                                                                      */
/*      A schedule record is a unsigned 32 bit value.  The high bit     */ 
/*      indicated the state (on or off) the lower bits contain the      */
/*                                                                      */
/*      record key (minutes from 00:00)                                 */
/************************************************************************/

/***************************** includes *********************************/
 #include <stdio.h>
 #include <string.h>
 #include <propeller.h>
 #include <unistd.h>
 #include "Pcon.h"
 #include "schedule.h"
 #include "bitlit.h" 

/****************************** externals *******************************/
 /* rtc control block */ 
 extern struct {
    unsigned stack[_STACK_SIZE_RTC];
    volatile RTC_CB rtc;
 } rtc_cb;
 /* control block & stack for dio cog */
 extern struct {
    unsigned stack[_STACK_SIZE_DIO];
    volatile DIO_CB dio;
 } dio_cb;
/******************** global code to text conversion ********************/
 extern char *day_names_long[7];     
 extern char *day_names_short[7];
 extern char *onoff[2];
 extern char *con_mode[3];
 extern char *sch_mode[2];
/************************** edit state variable *************************/
 extern int     edit_channel,edit_day,edit_hour,edit_minute,edit_key;
/***************************** globals **********************************/
 uint32_t       state_mask = B32(10000000,00000000,00000000,00000000);
 uint32_t       key_mask   = B32(01111111,11111111,11111111,11111111);
 uint32_t       bbb[_SCHEDULE_BUFFER];
 char fn_schedule[_SCHEDULE_NAME_SIZE] = _F_PREFIX _FILE_SET_ID _F_SCHEDULE_SUFIX;
/*******************************  functions ******************************/
int read_sch(uint32_t *sbuf)    // read data from SD card load buffer 
 {
    FILE    *sfp;
    int     rtn;

    sfp = fopen(fn_schedule,"r");
    // printf("fopen returned <%x> trying to open %s reading\n",(uint32_t)sfp,fn_schedule);

    if(sfp)
    {
        rtn = fread(bbb,_SCHEDULE_BUFFER*4,1,sfp);
        // if(rtn!=1)
        // {
            // printf("*** error reading schedula data\n");
            // return 1;
        // }
        printf("schedule data loaded into buffer from SD card\n");;
        fclose(sfp);
    }
    return 0;
 }

int write_sch(uint32_t *sbuf)   // write data from buffer to SD card 
 {
    FILE    *sfp;
    int     rtn;

    sfp = fopen(fn_schedule,"w");
    printf("fopen returned <%x> trying to open %s for writing\n",(uint32_t)sfp,fn_schedule);
    if(sfp)
    {
        rtn = fwrite(bbb,_SCHEDULE_BUFFER*4,1,sfp);
        if(rtn!=1)
        {
            printf("*** error writting schedula data\n");
            return 1;
        }
        printf("fwrite returned <%i> writing %i bytes from buffer at $%x\n",rtn,_SCHEDULE_BUFFER,(uint32_t)bbb);
        fclose(sfp);   
    }

    return 0;
 }

void clear_sch(uint32_t *sbuf)  // fill schedule buffer with 0
 {
    int         i;
    for(i=0;i<_SCHEDULE_BUFFER;i++) *sbuf++ = '\0';
    return;
 }

void ld_sch(uint32_t *sbuf)     // load schedule buffer with 0 - _SCHEDULE_BUFFER
 {
    int         i;
    for(i=0;i<_SCHEDULE_BUFFER;i++) *sbuf++ = (uint32_t)i;
    return;
 }

 int init_sch(uint32_t *sbuf)
 {
    FILE    *sfp;

    printf("schedule file name <%s>\n",fn_schedule);

    sfp = fopen(fn_schedule,"r");
    if(sfp==0)
    {
        printf("  schedule file <%s> not found, it will be created\n",fn_schedule);
        sfp = fopen(fn_schedule,"w");
        if(sfp)
        {
            printf("  schedule file <%s> created\n",fn_schedule);
            return 0;
        }
        else
        {
            printf("*** error can't create shedule file <%s>\n",fn_schedule);
            return 1;
        }
    }
    printf("  schedle file <%s> found\n",fn_schedule);
    fclose(sfp);
    return 0;
 }

void dump_schs(uint32_t *sbuf)
 {
    int         i,ii;
    ii = 0;
    // printf("day %i\n",iii++);
    // printf("\nchannel %i: ",ii);
    for(i=0;i<_SCHEDULE_BUFFER;i++)
    {
        printf("%08x ",*sbuf++);
        if(( ((i+1) % _BYTES_PER_CHANNEL)==0) &&(i>0))
        {
            ii++;
            printf("\n");
        }
        if(ii == _NUMBER_OF_CHANNELS)
        {
                printf("\n");
            ii = 0;   
        } 
        // if(  ((i%_BYTES_PER_DAY)==0)&&(i>0) ) printf("\n");
    } 
    if((i%80)==0) printf("\n");    
    return;
 }

void dump_sch(uint32_t *sbuf)
 {
    int         i;

    printf("\n");
    for(i=0;i<_MAX_SCHEDULE_RECS+1;i++)
    {
        printf("%08x ",*sbuf++);

    } 
    printf("\n");    
    return;
 }

/* display all schedules records (schedule) for a (channel,day) */
void dspl_sch(uint32_t *sbuf, int d, int c)
{
    int                         i,rsize;
    volatile uint32_t           *r;

    r = get_schedule(sbuf,d,c); 
    if(*r==0)
    {
        printf("    no schedule records\n");
        return;   
    }

    rsize = *r++;
  
    for(i=0;i<rsize;i++)
    {
        printf("    %02i:%02i - %s\n",get_key(*r)/60,get_key(*r)%60,onoff[get_s(*r)]);
        r++;
    }
        // printf("\n\n");    
    return;
}

uint32_t *get_schedule(uint32_t *sbuf,int d,int c)  // return pointer to  a schedule
 {
    SCH         *sch_ptr;   
    DAY         *day_ptr;   

    day_ptr = (DAY *)sbuf;      //set day pointer to the start of the schedule buffer
    day_ptr += d-1;             //move day pointer to the start of the requested day
    sch_ptr = (SCH *)day_ptr;   //set channel pointer to the start of the requested day
    sch_ptr += c;               //move channel pointer to the requested channel

    return (uint32_t *)sch_ptr;
 }


int get_key(uint32_t b)    // extract key from a schedule record 
 {
    uint32_t     k;
    k = (int)(b & key_mask);
    return (int)k;
 }

int get_s(uint32_t b) // extract state from a schedule record 
 {
    if (b & state_mask)
        return 1;
    return 0;
 }

void put_key(volatile uint32_t *value,int key)   // load key into a schedule record 
 {
    int         hold_state;
    uint32_t    t;

    hold_state = get_s(*value);
    t=(uint32_t)key;
    if(*value & state_mask) t |= state_mask;
    *value = key;
    put_state(value,hold_state);
    return;
 }

void put_state(volatile uint32_t *b,int s)  // load state into a schedule record
 {
    // printf("setting state to %i\n",s);
    if(s) *b |= state_mask; 
    else  *b &= ~state_mask;
    return;
 } 

int add_sch_rec(uint32_t *sch, int k, int s)  // add or change a schedule record */
 {
    uint32_t       *end, *r;

    /* schedule has no records - insert one */
    if((int)*sch==0)
    {
        *sch++ = 1;
        put_state(sch,s);
        put_key(sch,k);
        return 0;
    }
    /* see if there is room to add another record */
    if((int)*sch >= _MAX_SCHEDULE_RECS)
    {
        printf("*** too many schedule records\n");
        return 1;
    }
    /* if record exists change it */
    r=find_schedule_record(sch,k);
    if (r)
    {
        put_state(r,s);
        return 0;
    }
    /* insert new record in ordered list */
    *sch += 1;                  //increase record count
    end = (sch + *sch) - 1;   //set pointer to end of the list
    sch++;
    printf("\n\n");
    while(sch <= end)
    {
        if(k < get_key(*sch))
            break; 
        sch++;      
    } 
    while(end >= sch)
        *(end+1) = *end--;
    put_state(sch,s);
    put_key(sch,k);
    return 0;      
 }

int del_sch_rec(uint32_t *sch, int k)    // delete a schedule record with matching key 
{
    uint32_t            *rsize;
    int              i,hit;

    if(*sch==0)
        return 0;
    if(*sch==1)
    {
        *sch = 0;
        return 0;
    }

    hit = 0;
    rsize = sch++;

    for(i=0;i<*rsize;i++)
    {
        if((k==get_key(*sch)) || (hit==1))
        {
            hit = 1;
            *sch = *(sch+1);   
        }    
        sch++;
    }

    if(hit)
    {
        *rsize -= 1;
        return 0;
    }     
    return 1;
}

uint32_t *find_schedule_record(uint32_t *sch,int k)  // search schedule for record with key match, return pointer to record or NULL 
 {
    int                              i, rsize;
    
    rsize = (int)*sch++;
    for(i=0;i<rsize;i++)
    {
        if(k==get_key(*sch))
            return sch;
        sch++;
    }
    return NULL;
 }

 void disp_all_schedules(uint32_t *buffer)
 {
    uint32_t        *rec_ptr;
    int             i;
    int             day,channel;
    char            time_state[9];
    int             rcnt[_DAYS_PER_WEEK],mrcnt;


    // day_ptr = buffer;
    // sch_ptr = buffer;
    // rec_ptr = &buffer[0];
    // frec_ptr = rec_ptr;


    for(channel=0;channel<_NUMBER_OF_CHANNELS;channel++)
    {
    /* print channel header */        
        printf("channel %i <%s> control %s, ",channel,dio_cb.dio.cca[channel].name,con_mode[dio_cb.dio.cca[channel].c_mode]);
        printf(" channel is %s",onoff[dio_cb.dio.cca[channel].state]);
        printf(" as of %i:%02i, %s\n           ",
        rtc_cb.rtc.td_buffer.hour,
        rtc_cb.rtc.td_buffer.min,
        day_names_long[rtc_cb.rtc.td_buffer.dow-1]);


        for (day=0;day<_DAYS_PER_WEEK;day++)
            printf("%s         ",day_names_short[day]);
        printf("\n");
        mrcnt = 0;
        for(day=0;day<_DAYS_PER_WEEK;day++)
        {
            rcnt[day] = (int)*get_schedule(bbb,day+1,channel);
            // printf("rcnt[%i] = %i\n",day,rcnt[day]);
            if(rcnt[day] > mrcnt)
                mrcnt = rcnt[day];        //max number of records for the week
        }
        // printf("mrcnt %i\n",mrcnt);
        for(i=0;i<mrcnt;i++)
        {
            printf("         ");
            for(day=0;day<_DAYS_PER_WEEK;day++)
            {
                rec_ptr = get_schedule(bbb,day+1,channel);
                rec_ptr += (i+1);
                // printf("XXXXXX\n");
                if(*get_schedule(bbb,day+1,channel) <= i)
                    strcpy(time_state,"         ");
                else
                    sprintf(time_state,"%02i:%02i %s",get_key((uint32_t)*rec_ptr)/60,get_key((uint32_t)*rec_ptr)%60,onoff[get_s((uint32_t)*rec_ptr)]);

                printf("%s   ",time_state);

            }
            printf("\n");
        }
        printf("\n");
     } 

    return;  
 }


