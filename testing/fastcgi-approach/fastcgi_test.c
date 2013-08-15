#include "fcgi_stdio.h" /* fcgi library; put it first*/
#include <stdlib.h>

/*
	But the suggestion was: FunctionName, variable_name (local or member),
    Structure, ENUMVALUE, Extern_FunctionName, g_global
*/

typedef struct Data Data;

typedef void (*ModuleHandler) (Data *data, const char *params);

static void SensorsHandler(Data *data, const char *params) {
    printf("Sensors module!<br>");
}

/*
   API Schema:
   Sensors:
   /cgi/sensors?get=x
   *get=x is optional. Retrieves info for sensor with id x
   Devices:
   /cgi/devices?status=x&power=y&id=z
   *status and power is optional
   *status retrieves whether device with id x is operational
   *power tells whether or not to power on/off the device with id z
   
   Response format:
   200 OK if request was ok
   400 bad request for malformed request
      
*/
int main (int argc, char *argv[])
{
  Data *data = NULL;
  int count = 0;

  //FCGI Accept loop
  while (FCGI_Accept() >= 0)   {
    ModuleHandler module_handler = NULL;
    const char *module = getenv("DOCUMENT_URI_LOCAL");
    const char *params = getenv("QUERY_STRING");

    if (!strcmp("sensors", module)) {
        module_handler = SensorsHandler; //Replace with pointer to sensors handler
    } else if (!strcmp("admin"), module) {
        module_handler = NULL; //Replace with pointer to admin handler
        printf("Admin module selected!\n");
    }
    
    if (module_handler) {
        printf("Content-type: text/html\r\n\r\n"); //Replace with actual type
        module_handler(data, params);
    } else {
        printf("Status: 400 Bad Request\r\n"
               "Content-type: text/html\r\n\r\n"
               "<title>400 Bad Request</title>\n"
               "Unknown module '%s' selected.<br>\n",
               module);   
    }
    
    //Debgging:
    printf("Module: %s, Params: %s<br>\n", module, params);
    printf("Request number %d, host <i>%s</i>\n",
        count++, getenv("SERVER_HOSTNAME"));
  }
}
