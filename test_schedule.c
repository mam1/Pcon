/**
 * This is the main test_schedule program file.
 */

#include <stdio.h>
#include <propeller.h>
#include "Pcon.h"
#include "schedule.h"

 extern char    		fn_schedule[];
 extern uint32_t       	working_schedules[];

int main(void)
{
	int 			i;
	uint32_t		*rec;

	printf("_NUMBER_OF_CANNELS %i\n",_NUMBER_OF_CHANNELS);
	printf("_BYTES_PER_CHANNEL %i\n",_BYTES_PER_CHANNEL);
	printf("_BYTES_PER_DAY %i\n",_BYTES_PER_DAY);
	printf("schedule buffer size %i\n",_SCHEDULE_BUFFER);
	printf("schedule file name<%s>\n",fn_schedule);
	read_sch(working_schedules);
	write_sch(working_schedules);
	read_sch(working_schedules);
	printf("loading buffer\n");
	clear_sch(working_schedules);
	// write_sch(working_schedules);

	// dump_schs(get_schedule(working_schedules,0,0));
/*
	rec = (int)get_schedule(working_schedules,0,0);
	for(i=0;i<_MAX_SCHEDULE_RECS+1;i++)
		printf("%04i ",*rec++);
	printf("\n");

	rec = (int)get_schedule(working_schedules,6,7);
	for(i=0;i<_MAX_SCHEDULE_RECS+1;i++)
		printf("%04i ",*rec++);
	printf("\n");
*/
	// dump_schs(working_schedules);
	printf("clear buffer and add \n");
	add_sch_rec(get_schedule(working_schedules,0,0),4,1);

	add_sch_rec(get_schedule(working_schedules,0,0),10,1);

	add_sch_rec(get_schedule(working_schedules,0,0),3,1);

	add_sch_rec(get_schedule(working_schedules,0,0),11,0);

	add_sch_rec(get_schedule(working_schedules,0,0),0,0);

	add_sch_rec(get_schedule(working_schedules,0,0),6,1);

	add_sch_rec(get_schedule(working_schedules,0,0),4,0);

	dump_schs(working_schedules);

	del_sch_rec(get_schedule(working_schedules,0,0),6);
	del_sch_rec(get_schedule(working_schedules,0,0),0);


	dump_schs(working_schedules);

	// get_schedule(working_schedules,0,1);
	// get_schedule(working_schedules,0,2);
	// get_schedule(working_schedules,1,0);
	// get_schedule(working_schedules,6,7);


  	return 0;
}

