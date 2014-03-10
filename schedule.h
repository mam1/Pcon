#ifndef SCHEDULE_H_
#define SCHEDULE_H_

/* routines to work with schedules */
int      save_schedule_data(_SB,int); 			//write to sd (schedule buffer, day)    
int      load_schedule_data(_SB,int); 			//read from sd (schedule buffer, day)
void     dump_sch_recs(_SB, int, int);    		//display schedule (schedule buffer, channel,day)
volatile uint32_t *find_schedule_record(_SB,int,int); 	//search a schedule for a key (schedule buffer,channel,key), return pointer to record or NULL
int      add_sch_rec(_SB,int,int,int);			//add a record to a schedule (schedule buffer,channel,key,state)
int 	 del_sch_rec(_SB,int,int);				//delete record from schedule (schedule buffer,channel,key)
int 	 init_schedule_data(void);				//create a set of empty schedule files if they are not present
void	 clean_sch_buf(_SB);					//set a schedule buffer to all '\0's	 

/* routines to work with individual schedule records */
int  get_key(uint32_t);          			//extract key (lower 31 bits) from a uint32_t
int  get_statex(uint32_t);        			//extract state (high bit) from a uint32_t
void put_key(volatile uint32_t *,int);    	//load key into lower 31 bits of the  uint31_t at the address specified
void put_state(volatile uint32_t *,int);  	//load state into high bit of the uint31_t at the address specified

/* build a file name for each (channel,day) combination */
char *sch_file_name(char *, int, int);  	// *buffer,channel,day: -> *buffer

#endif



