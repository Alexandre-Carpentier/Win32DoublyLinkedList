// Demo using LIST_ENTRY
#include <windows.h>
#include <stdio.h>
#include <process.h>

// iState
#define WAIT		2000
#define RUN			2001
#define STOP		2003

typedef struct {
	INT iState;
	LIST_ENTRY list_entry;
	CHAR* szModuleName;
	CHAR* szAction;
	LPVOID obj;
}TASKSTRUCT, * PTASKSTRUCT;

PLIST_ENTRY HeadList;

VOID FORCEINLINE InitializeListHead(PLIST_ENTRY ListHead)
{
	ListHead->Flink = ListHead->Blink = ListHead;
}

VOID FORCEINLINE InsertHeadList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
	PLIST_ENTRY Flink;

	Flink = ListHead->Flink;
	Entry->Flink = Flink;
	Entry->Blink = ListHead;
	Flink->Blink = Entry;
	ListHead->Flink = Entry;
}

VOID FORCEINLINE InsertTailList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
	PLIST_ENTRY Blink;

	Blink = ListHead->Blink;
	Entry->Flink = ListHead;
	Entry->Blink = Blink;
	Blink->Flink = Entry;
	ListHead->Blink = Entry;
}

DWORD ProcessTask(LPVOID arg)
{
	PTASKSTRUCT Task = arg;
	Sleep(5000);
	Task->iState = STOP;
	return 0;
}

BOOL CreateTask(LPVOID obj, CHAR* module, CHAR* action)
{
	// Create some task object
	TASKSTRUCT* task = NULL;  task = (TASKSTRUCT*)malloc(sizeof(TASKSTRUCT));
	if (NULL == task)
		return FALSE;

	task->iState = WAIT;
	task->szModuleName = module;
	task->szAction = action;

	InsertTailList(HeadList, &(task->list_entry));

	return TRUE;
}

VOID DestroyAllTask(PLIST_ENTRY list)
{
	PLIST_ENTRY temp = list;

	while (temp->Flink != list)
	{
		temp = temp->Flink; // first item  // blink point to list head
		temp = temp->Flink; // second item // blink point to first item
		free(CONTAINING_RECORD(temp->Blink, TASKSTRUCT, list_entry)); // free memory of the first item and loop
	}
	free(list); // Free ptr of the list
}

INT main(VOID)
{
	// Initialize the HeadList entry
	HeadList = (PLIST_ENTRY)malloc(sizeof(LIST_ENTRY));
	InitializeListHead(HeadList);

	// Create some task stuctures and add it to the list entry
	CreateTask(NULL, "Actuator", "push");
	CreateTask(NULL, "Supply", "set");
	CreateTask(NULL, "DMM", "read");
	CreateTask(NULL, "Supply", "read");
	CreateTask(NULL, "Supply", "set");
	CreateTask(NULL, "Actuator", "home");

	// Execute each task in the queue and remove object at the end
	PLIST_ENTRY temp = HeadList;
	INT iCount = 0;
	while (temp->Flink != HeadList) // Loop on temp list until we back again on the list head
	{
		temp = temp->Flink; // next item 

		iCount++;
		printf("[*] Task %i on module: %s\n", iCount, CONTAINING_RECORD(temp, TASKSTRUCT, list_entry)->szModuleName);
		printf("[*] Performing : %s\n", CONTAINING_RECORD(temp, TASKSTRUCT, list_entry)->szAction);
		printf("[*] STATUS : %s\n", CONTAINING_RECORD(temp, TASKSTRUCT, list_entry)->iState == WAIT ? "WAIT" : "RUN");
		CONTAINING_RECORD(temp, TASKSTRUCT, list_entry)->iState = RUN;


		_beginthread(ProcessTask, 0, CONTAINING_RECORD(temp, TASKSTRUCT, list_entry)); //Send the entire structure to a thread to process

		while (1)
		{
			switch (CONTAINING_RECORD(temp, TASKSTRUCT, list_entry)->iState)
			{
			case WAIT:
			{
				printf("[*] Task %i on module: %s is waiting\n", iCount, CONTAINING_RECORD(temp, TASKSTRUCT, list_entry)->szModuleName);
				Sleep(1000);
				break;
			}
			case STOP:
			{
				printf("[*] Task %i on module: %s is stopping\n", iCount, CONTAINING_RECORD(temp, TASKSTRUCT, list_entry)->szModuleName);
				goto task_end;
			}
			case RUN:
			{
				printf("[*] Task %i on module: %s is running\n", iCount, CONTAINING_RECORD(temp, TASKSTRUCT, list_entry)->szModuleName);
				Sleep(1000);
				break;
			}
			}
		}
	task_end:
		printf("[*] Next task\n\n");
	}

	printf("[*] Cleanning tasks in memory and exit\n\n");
	DestroyAllTask(HeadList);

	return EXIT_SUCCESS;
}