/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*					SEG4145: Real-Time and Embedded System Design
*
*							Assignment #4 Skeleton
*
*                     (c) Copyright 2010- Stejarel C. Veres, cveres@site.uottawa.ca
* 					  Portions adapted after Jean J. Labrosse
*
*                As is, this program will create a main (startup) task which will in turn
*             spawn two children. One of them will count odd numbers, the other - even ones.
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE			512		/* Size of start task's stacks                         */
#define TASK_START_PRIO			0		/* Priority of your startup task		       	 */
#define N_TASKS                     2           /* Number of (child) tasks to spawn                    */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK TaskStartStk[TASK_STK_SIZE];			/* Start task's stack						 */
OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];         /* Stacks for other (child) tasks				 */
INT8U TaskData[N_TASKS];				/* Parameters to pass to each task                     */

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void TaskStart(void *data);				/* Startup task							 */
static void TaskStartCreateTasks(void);         /* Will be used to create all the child tasks          */
void Task(void*);						/* The body of each child task                         */

/*
*********************************************************************************************************
*                                             MAIN FUNCTION
*********************************************************************************************************
*/

int main(void)
{
    OSInit();						/* Initialize uC/OS-II						 */

    /* 
     * Create and initialize any semaphores, mailboxes etc. here
     */

    OSTaskCreate(TaskStart, (void *) 0, 
		     &TaskStartStk[TASK_STK_SIZE - 1], TASK_START_PRIO);	/* Create the startup task	 */

    OSStart();						/* Start multitasking						 */

    return 0;
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void TaskStart(void *pdata)
{   INT16S key;
    pdata = pdata;                                         /* Prevent compiler warning                 */


#if OS_TASK_STAT_EN
    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */
#endif

    /*
     * Display version information
     */

    printf("Startup (Main) Task:\n");
    printf("--------------------\n");
    printf("Running under uC/OS-II V%4.2f (with WIN32 port V%4.2f).\n",
           ((FP32) OSVersion())/100, ((FP32)OSPortVersion())/100);
    printf("Press the Escape key to stop.\n\n");

    /* 
     * Here we create all other tasks (threads)
     */

    TaskStartCreateTasks();

    while (1)								/* Startup task's infinite loop	       */
    {
	  /*
	   * Place additional code for your startup task here
         * or before the loop, as needed
         */

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* If yes, see if it's the ESCAPE key       */
                exit(0);  	                             /* End program                              */
            }
        }

        /* 
         * Don't forget to call the uC/OS-II scheduler with OSTimeDly(), 
         * to give other tasks a chance to run
         */

        OSTimeDly(10);						     /* Wait 10 ticks                            */
    }
}

/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static void TaskStartCreateTasks(void)
{
    INT8U i;
    INT8U prio;

    for (i = 0; i < N_TASKS; i++) {
        prio = i + 1;
        TaskData[i] = prio;
        OSTaskCreateExt(Task,
                        (void *) &TaskData[i],
                        &TaskStk[i][TASK_STK_SIZE - 1],
                        prio,
                        0,
                        &TaskStk[i][0],
                        TASK_STK_SIZE,
                        (void *) 0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR | OS_TASK_OPT_SAVE_FP);
    }
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void Task(void *pdata)
{
    INT8U whoami = *(int*) pdata;
    INT8U counter = whoami % 2;

    while (1) {
      printf("I am task #%d and my counter is at %d.\n",
             whoami, counter);
      counter += 2;

	OSTimeDly(50);
    }
}
