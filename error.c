#include "error.h"
#include "platform.h"

void error(cansw_logger_error_t error)
{
    //put a breakpoint here to see what broke
    LED_1_OFF();
    while (1);
}
