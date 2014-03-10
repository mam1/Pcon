/**
 * This is the main test_schedule program file.
 */

#include <stdio.h>
#include <propeller.h>
#include "Pcon.h"
// #include "schedule.h"


int main(void)
{
	printf("_NUMBER_OF_CANNELS %i\n",_NUMBER_OF_CHANNELS);
	printf("_BYTES_PER_CHANNEL %i\n",_BYTES_PER_CHANNEL);
	printf("_BYTES_PER_DAY %i\n",_BYTES_PER_DAY);
	printf("schedule buffer size %i\n",_SCHEDULE_BUFFER);
  	return 0;
}

