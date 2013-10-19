/**
 * @file control.c
 * @brief Handles all client control requests (admin related)
 */
#include "common.h"
#include "options.h"
#include "control.h"
#include "sensor.h"
#include "actuator.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct ControlData {
	ControlModes current_mode;
	pthread_mutex_t mutex;
	struct timespec start_time;
	char user_name[31]; // The user who owns the currently running experiment
	char experiment_dir[BUFSIZ]; //Directory for experiment
	char experiment_name[BUFSIZ];
} ControlData;

ControlData g_controls = {CONTROL_STOP, PTHREAD_MUTEX_INITIALIZER, {0}};

bool DirExists(const char *path)
{
	DIR *dir = opendir(path);
	if (dir) {
		closedir(dir);
		return true;
	}
	return false;
}

bool PathExists(const char *path) 
{
	FILE *fp = fopen(path, "r");
	if (fp) {
		fclose(fp);
		return true;
	}
	return false;
}

/**
 * Lists all experiments for the current user.
 * @param The context to work in
 */
void ListExperiments(FCGIContext *context) 
{
	DIR * dir = opendir(context->user_dir);
	if (dir == NULL)
	{
		FCGI_RejectJSON(context, "Failed to open user directory");
		return;
	}
	struct dirent * ent;
	FCGI_BeginJSON(context, STATUS_OK);
	FCGI_JSONKey("experiments");
	FCGI_PrintRaw("[");

	bool first = true;
	while ((ent = readdir(dir)) != NULL) {
		char *ext = strrchr(ent->d_name, '.');
		if (ext && !strcmp(ext, ".exp")) {
			if (!first) {
				FCGI_PrintRaw(", ");
			}

			*ext = '\0'; // Ummm... probably not a great idea
			FCGI_PrintRaw("\"%s\"", ent->d_name);
			first = false;
		}
	}
	FCGI_PrintRaw("]");
	FCGI_EndJSON();
	
	closedir(dir);
	return;
}

/**
 * System control handler. This covers high-level control, including
 * admin related functions and starting/stopping experiments..
 * @param context The context to work in
 * @param params The input parameters
 */
void Control_Handler(FCGIContext *context, char *params) {
	const char *action = "";
	const char *name = "";
	bool force = false;
	ControlModes desired_mode;
	
	// Login/auth now handled entirely in fastcgi.c and login.c
	//TODO: Need to not have the ability for any user to stop someone else' experiment...
	// (achieve by storing the username of the person running the current experiment, even when they log out?)
	// (Our program should only realisitically support a single experiment at a time, so that should be sufficient)
	FCGIValue values[3] = {
		{"action", &action, FCGI_REQUIRED(FCGI_STRING_T)},
		{"force", &force, FCGI_BOOL_T},
		{"name", &name, FCGI_STRING_T}
	};

	if (!FCGI_ParseRequest(context, params, values, 3))
		return;

	if (!strcmp(action, "identify")) {
		FCGI_BeginJSON(context, STATUS_OK);
		FCGI_JSONLong("control_state_id", g_controls.current_mode);
		FCGI_JSONPair("control_user_name", g_controls.user_name);
		FCGI_JSONPair("control_experiment_name", g_controls.experiment_name);
		FCGI_EndJSON();
		return;
	} else if (!strcmp(action, "list")) {
		ListExperiments(context);
		return;
	}
	//TODO: Need a "load" action to set data files (but not run) from a past experiment

	//TODO: Need a "delete" action so that people can overwrite experiments (without all this "force" shenanigans)
	
	if (!strcmp(action, "emergency")) {
		desired_mode = CONTROL_EMERGENCY;
	} else if (!strcmp(action, "start")) {
		desired_mode = CONTROL_START;
	} else if (!strcmp(action, "pause")) {
		desired_mode = CONTROL_PAUSE;
	} else if (!strcmp(action, "resume")) {
		desired_mode = CONTROL_RESUME;
	} else if (!strcmp(action, "stop")) {
		desired_mode = CONTROL_STOP; 
	} else {
		FCGI_RejectJSON(context, "Unknown action specified.");
		return;
	}

	if ((*g_controls.user_name) != '\0' && strcmp(g_controls.user_name, context->user_name) != 0)
	{
		if (context->user_type != USER_ADMIN) {
			FCGI_RejectJSON(context, "Another user has an experiment in progress.");
			return;
		}
		
		if (!force) {
			Log(LOGERR, "User %s is currently running an experiment!", g_controls.user_name);
			FCGI_RejectJSON(context, "Pass \"force\" to take control over another user's experiment");
			return;
		}
	}

	void *arg = NULL;
	char experiment_dir[BUFSIZ] = {0};
	if (desired_mode == CONTROL_START) {
		int ret;

		if (*name == '\0') {
			FCGI_RejectJSON(context, "An experiment name must be specified.");
			return;
		} if (strpbrk(name, INVALID_CHARACTERS)) {
			FCGI_RejectJSON(context, 
				"An experiment name must not contain: " INVALID_CHARACTERS_JSON);
			return;
		}

		if (*(context->user_dir) == '\0') {
			FCGI_RejectJSON(context, "Not creating experiment in root dir.");
			return;
		}

		ret = snprintf(experiment_dir, BUFSIZ, "%s/%s.exp", 
						context->user_dir, name);
		if (ret >= BUFSIZ) {
			FCGI_RejectJSON(context, "The experiment name is too long.");
			return;
		} else if (DirExists(experiment_dir) && !force) {
			FCGI_RejectJSON(context, "An experiment with that name already exists.");
			return;
		}

		arg = (void*) experiment_dir;
	}

	const char *ret;
	if ((ret = Control_SetMode(desired_mode, arg)) != NULL) {
		FCGI_RejectJSON(context, ret);
	} else {
		if (desired_mode == CONTROL_STOP) {
			g_controls.user_name[0] = '\0';
			g_controls.experiment_dir[0] = '\0';
			g_controls.experiment_name[0] = '\0';
		} else if (desired_mode == CONTROL_START) {
			snprintf(g_controls.user_name, sizeof(g_controls.user_name), 
						"%s", context->user_name);
			snprintf(g_controls.experiment_dir, sizeof(g_controls.experiment_dir),
						"%s", experiment_dir);
			snprintf(g_controls.experiment_name, sizeof(g_controls.experiment_name),
						"%s", name);
		}

		FCGI_AcceptJSON(context, "Ok", NULL);
	}
}

/**
 * Sets the mode to the desired mode, if possible.
 * @param desired_mode The mode to be set to
 * @param arg An argument specific to the mode to be set. 
 * @return NULL on success, an error message on failure.
 */
const char* Control_SetMode(ControlModes desired_mode, void * arg)
{
	const char *ret = NULL;

	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.current_mode == desired_mode)
		ret = "Already in the desired mode.";
	else if (g_controls.current_mode == CONTROL_EMERGENCY && desired_mode != CONTROL_STOP)
		ret = "In emergency mode. You must stop before continuing.";
	else switch (desired_mode) {
		case CONTROL_START:
			if (g_controls.current_mode == CONTROL_STOP) {
				const char * path = arg;
				if (mkdir(path, 0777) != 0 && errno != EEXIST) {
					Log(LOGERR, "Couldn't create experiment directory %s - %s", 
						path, strerror(errno));
					ret = "Couldn't create experiment directory.";
				} else {
					clock_gettime(CLOCK_MONOTONIC, &(g_controls.start_time));
				}
			} else 
				ret = "Cannot start when not in a stopped state.";
		break;
		case CONTROL_PAUSE:
			if (g_controls.current_mode != CONTROL_START)
				ret = "Cannot pause when not in a running state.";
		break;
		case CONTROL_RESUME:
			if (g_controls.current_mode != CONTROL_PAUSE)
				ret = "Cannot resume when not in a paused state.";
		break;
		case CONTROL_EMERGENCY:
			if (g_controls.current_mode != CONTROL_START) //pfft
				ret = "Not running so how can there be an emergency?";
		break;
		default:
		break;
	}
	
	if (ret == NULL) {
		Actuator_SetModeAll(desired_mode, arg);
		Sensor_SetModeAll(desired_mode, arg);
		if (desired_mode != CONTROL_RESUME)
			g_controls.current_mode = desired_mode;
		else
			g_controls.current_mode = CONTROL_START;
	}
	pthread_mutex_unlock(&(g_controls.mutex));
	return ret;
}

/**
 * Gets a string representation of the current mode
 * @param mode The mode to get a string representation of
 * @return The string representation of the mode
 */
const char * Control_GetModeName() {
	const char * ret = "Unknown";

	switch (g_controls.current_mode) {
		case CONTROL_START: ret = "Running"; break;
		case CONTROL_PAUSE: ret = "Paused"; break;
		case CONTROL_RESUME: ret = "Resumed"; break;
		case CONTROL_STOP: ret = "Stopped"; break;
		case CONTROL_EMERGENCY: ret = "Emergency mode"; break;
	}
	return ret;
}

/**
 * Gets the start time for the current experiment
 * @return the start time
 */
const struct timespec * Control_GetStartTime() {
	return &g_controls.start_time;
}
