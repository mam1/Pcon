#ifndef SCHEDULE_H_
#define SCHEDULE_H_

/* routines that work with the schedule buffer */
 int read_sch(uint32_t *);
 int write_sch(uint32_t *);
 void clear_sch(uint32_t *);
 void ld_sch(uint32_t *);
 int init_sch(uint32_t *);
 void dump_sch(uint32_t *);
 void dump_schs(uint32_t *);
 void dspl_schs(uint32_t *, int, int);
 void dspl_sch(uint32_t *, int, int);
 uint32_t *get_schedule(uint32_t *,int,int);
 void disp_all_schedules(uint32_t *);

/* routines to work with schedules */
// int      save_schedule_data(uint32_t *,int); 			//write to sd (schedule buffer, day)    
// int      load_schedule_data(uint32_t *,int); 			//read from sd (schedule buffer, day)
// void     dump_sch_recs(uint32_t *, int, int);    		//display schedule (schedule buffer, channel,day)
uint32_t *find_schedule_record(uint32_t *,int); 	//search a schedule for a key (schedule buffer,channel,key), return pointer to record or NULL
int		add_sch_rec(uint32_t *,int,int);			//add a record to a schedule (schedule buffer,channel,key,state)
int 	del_sch_rec(uint32_t *,int);			//delet a record
// int 	 del_sch_rec(uint32_t *,int,int);				//delete record from schedule (schedule buffer,channel,key)
// int 	 init_schedule_data(void);				//create a set of empty schedule files if they are not present
// void	 clean_sch_buf(uint32_t *);					//set a schedule buffer to all '\0's	 

/* routines to work with individual schedule records */
 int  get_key(uint32_t);          			//extract key (lower 31 bits) from a uint32_t
 int  get_s(uint32_t);        			//extract state (high bit) from a uint32_t
 void put_key(volatile uint32_t *,int);    	//load key into lower 31 bits of the  uint31_t at the address specified
 void put_state(volatile uint32_t *,int);  	//load state into high bit of the uint31_t at the address specified

/* build a file name for each (channel,day) combination */
// char *sch_file_name(char *, int, int);  	// *buffer,channel,day: -> *buffer

#endif



