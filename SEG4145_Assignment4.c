/*
 * Ahmed Ben Messaoud 	  - 4291509
 * Elvis-Philip Niyonkuru - 3441001
 */

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

#define TASK_STK_SIZE			512		/* Size of start task's stacks */
#define TASK_START_PRIO			0		/* Priority of your startup task */
#define TASK_MOTORS_PRIO		1		/* Priority of your motors task	*/
#define TASK_DISPLAY_PRIO		2		/* Priority of your display task */
#define Q_SIZE					10		/* The length of the queue */

enum MotorEnum { MoveForward, MoveBackward, Turn90CW, Turn90CCW,
	Stopped, PerformCircle, ChangeDirection, IncreaseRadius };

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

INT8U Mode;									/* The current mode of the robot */
INT8U Direction;							/* The current direction of the robot */
OS_STK TaskStartStk[TASK_STK_SIZE];			/* Start task's stack */
OS_STK TaskMotorsStk[TASK_STK_SIZE];		/* Motors task stack */
OS_STK TaskDisplayStk[TASK_STK_SIZE];		/* Motors task stack */

/* Queue Events Pointers */
OS_EVENT *MotorsEvent;
OS_EVENT *DisplayEvent;

/* Semaphores */
OS_EVENT *SemMode;
OS_EVENT *SemDirection;

/* Message Queues */
void *MessageStorageMotors[Q_SIZE];
void *MessageStorageDisplay[Q_SIZE];

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void TaskStart(void *data);				/* Startup task	*/
void TaskMotors(void *data);			/* Motors task	*/
void TaskDisplay(void *data);			/* Display task	*/
void SW0Pressed();
void SW1Pressed();
void SW2Pressed();
void SW3Pressed();
static void testResult(INT8U result);
static void sendMessage(OS_EVENT *event, const INT8U msg);
void Motors(INT8U action);
void Lcd(INT8U action, INT8U mode);
void SevSegDisplay(INT8U mode);
void Led(INT8U action);

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

    MotorsEvent = OSQCreate((void**)&MessageStorageMotors, Q_SIZE);		/* Motors Queue Initialize */
    DisplayEvent = OSQCreate((void**)&MessageStorageDisplay, Q_SIZE);	/* Display Queue Initialize */

    SemMode = OSSemCreate(1);
    SemDirection = OSSemCreate(1);

    if( !MotorsEvent && !DisplayEvent)
    {
    	printf("Unable to create queues\n");
    	exit(0);
    }

    OSTaskCreate(TaskStart, NULL,
    		&TaskStartStk[TASK_STK_SIZE - 1], TASK_START_PRIO);	/* Create the startup task */

    OSStart();						/* Start multitasking						 */

    return 0;
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void TaskStart(void *pdata)
{
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

	/*
	 * Spawn Motor and Display Tasks
	 */
    testResult(OSTaskCreate(TaskMotors, NULL,
				&TaskMotorsStk[TASK_STK_SIZE - 1], TASK_MOTORS_PRIO));	/* Create the Motors task */

    testResult(OSTaskCreate(TaskDisplay, NULL,
				&TaskDisplayStk[TASK_STK_SIZE - 1], TASK_DISPLAY_PRIO));	/* Create the Display task */

	printf("Press the Escape key to stop.\n\n");

    /* 
     * Check input characters
     */

    INT16S key;
	while (1)								/* Startup task's infinite loop	       */
	{
		if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
			switch(key)
			{
				case 0x30:
					SW0Pressed();
					break;
				case 0x31:
					SW1Pressed();
					break;
				case 0x32:
					SW2Pressed();
					break;
				case 0x33:
					SW3Pressed();
					break;
				case 0x1B:
					exit(0);
					break;
				default:
					break;
			}
		}

		/*
		 * Don't forget to call the uC/OS-II scheduler with OSTimeDly(),
		 * to give other tasks a chance to run
		 */

		OSTimeDly(10);						     /* Wait 10 ticks                            */
	}
}

void SW0Pressed()
{
	if(!Mode)
	{
		// Mode 1
		// Send to the Motor
		sendMessage(MotorsEvent, MoveForward);

		// Send to the Display
		sendMessage(DisplayEvent, MoveForward);
	}
	else
	{
		// Mode 2
		// No resource is updated
	}
}

void SW1Pressed()
{
	if(!Mode)
	{
		// Mode 1
		// Send to the Motor
		sendMessage(MotorsEvent, MoveBackward);

		// Send to the Display
		sendMessage(DisplayEvent, MoveBackward);
	}
	else
	{
		// Mode 2
		// Changing Direction of Circle
		INT8U err;
		OSSemPend(SemDirection, 0, &err);
		Direction = !Direction;
		OSSemPost(SemDirection);
	}
}

void SW2Pressed()
{
	if(!Mode)
	{
		// Mode 1
		// Send to the Motor
		sendMessage(MotorsEvent, Turn90CW);

		// Send to the Display
		sendMessage(DisplayEvent, Turn90CW);
	}
	else
	{
		// Mode 2
		// Send to the Motor
		sendMessage(MotorsEvent, PerformCircle);

		// Send to the Display
		sendMessage(DisplayEvent, PerformCircle);
	}
}

void SW3Pressed()
{
	INT8U err;

	OSSemPend(SemMode, 0, &err);
	Mode = !Mode;
	OSSemPost(SemMode);

	sendMessage(DisplayEvent, Stopped);
}

static void testResult(INT8U result)
{
	switch(result)
	{
		case OS_ERR_NONE:
			return;
		case OS_ERR_Q_FULL:
			printf("OS_ERR_Q_FULL");
			break;
		case OS_ERR_EVENT_TYPE:
			printf("OS_ERR_EVENT_TYPE");
			break;
		case OS_ERR_PEVENT_NULL:
			printf("OS_ERR_PEVENT_NULL");
			break;
		default:
			printf("An Error Occurred - %uu", result);
	}
	exit(0);
}

static void sendMessage(OS_EVENT *event, const INT8U msg)
{
	INT8U *msgNew = malloc(sizeof(INT8U));
	*msgNew = msg;

	// Send to device
	testResult(OSQPost(event, (void*)msgNew));
}

/*
*********************************************************************************************************
*                                                  Motors
*********************************************************************************************************
*/
void TaskMotors(void *pdata)
{
	printf("Startup (Motors) Task\n");
	INT8U *msg = NULL;
	INT8U err;
	while(1)
	{
		// blocking operation, waits until an msg is available
		msg = OSQPend(MotorsEvent, 0, &err);

		// Send to motors resource
		Motors((INT8U)*msg);

		// free the string
		free(msg);
	}
}

/*
*********************************************************************************************************
*                                                  Display
*********************************************************************************************************
*/
void TaskDisplay(void *pdata)
{
	printf("Startup (Display) Task\n\n");
	INT8U *msg = NULL;
	INT8U err;
	while(1)
	{
		// blocking operation, waits until an msg is available
		msg = OSQPend(DisplayEvent, 0, &err);

		// Send to all display resource
		OSSemPend(SemMode, 0, &err);
		Lcd( (INT8U)*msg, Mode );
		SevSegDisplay(Mode);
		Led( (INT8U)*msg );
		OSSemPost(SemMode);
	}
}

/*
*********************************************************************************************************
*                                                  Resources
*********************************************************************************************************
*/
void Motors(INT8U action)
{
	switch(action)
	{
		case MoveForward:
			printf("[Motor Subsystem] - Moving Forward\n");
			break;
		case MoveBackward:
			printf("[Motor Subsystem] - Moving Backward\n");
			break;
		case Turn90CW:
			printf("[Motor Subsystem] - Turning 90 degrees Clockwise\n");
			break;
		case Turn90CCW:
			printf("[Motor Subsystem] - Turning 90 degrees Counter Clockwise\n");
			break;
		case PerformCircle:
			printf("[Motor Subsystem] - Performing Circle\n");
			OSTimeDly(300);	/* Longer delay for the circle */
			break;
	}

	OSTimeDly(100);						     /* Wait 100 ticks */
	printf("[Motor Subsystem] - Motors Stopped\n");
	sendMessage(DisplayEvent, Stopped);					/* Send Message to Display */
}

void Lcd(INT8U action, INT8U mode)
{
	printf("[Display Subsystem] LCD Resource Display - Line 1 - Mode = %d\n", mode+1);
	printf("[Display Subsystem] LCD Resource Display - Line 2 - ");
	switch(action)
	{
		case MoveForward:
			 printf("Moving FWD\n");
			break;
		case MoveBackward:
			printf("Moving BK\n");
			break;
		case Turn90CW:
			printf("Turning CW\n");
			break;
		case Turn90CCW:
			printf("Turning Turning CCW\n");
			break;
		case Stopped:
			printf("Stopped\n");
			break;
		case PerformCircle:
			printf("Performing Circle\n");
			break;
	}
}

void SevSegDisplay(INT8U mode)
{
	printf("[Display Subsystem] Seven Segment Resource Display - %d\n", mode+1);
}

void Led(INT8U action)
{
	printf("[Display Subsystem] LED Resource Display - ");

	switch(action)
	{
		case MoveForward:
			printf("01010101\n");
			break;
		case MoveBackward:
			printf("11001100\n");
			break;
		case Turn90CW:
			printf("00111100\n");
			break;
		case Turn90CCW:
			printf("11000011\n");
			break;
		case Stopped:
			printf("00000000\n");
			break;
	}
}
