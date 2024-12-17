#include "action_control.h"

#define OPEN_VALUE	500
#define CLOSE_VALUE	400

void setAction(int value)
{
	if(value >= OPEN_VALUE)
		actionOpen();
	
	if(value <= CLOSE_VALUE)
		actionClose();
}

void actionOpen()
{
	// Open the window and run ventilation
	
	openWindow();
	runVentilation();
}

void actionClose()
{
	// Close the window and stop ventilation
	
	closeWindow();
	stopVentilation();
}

void openWindow()
{

}

void closeWindow()
{
	
}

void runVentilation()
{
	
}

void stopVentilation()
{
	
}

