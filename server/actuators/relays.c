#include "relays.h"
#include "../log.h"

#include "../bbb_pin.h"


static int GetGPIO(int id)
{
	switch (id)
	{
		case RELAY_CANSELECT:
			return 14;
		case RELAY_CANENABLE:
			return 115;
		case RELAY_MAIN:
			return 112;
	}
	Fatal("Unknown id %d", id);
	return 0;
}

bool Relay_Init(const char * name, int id)
{
	if (!GPIO_Export(GetGPIO(id)))
		return false;
	return GPIO_Set(GetGPIO(id), false);
}

bool Relay_Cleanup(int id)
{
	bool err = GPIO_Set(GetGPIO(id), false);
	GPIO_Unexport(GetGPIO(id));
	return err;
}

bool Relay_Set(int id, double value)
{
	bool set = (bool)value;
	return GPIO_Set(GetGPIO(id), set);
}

bool Relay_Sanity(int id, double value)
{
	//bool set = (bool)value;
	//TODO: Make a more sane sanity check
	return true;
	
}
