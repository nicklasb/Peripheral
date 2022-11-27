

#ifdef __cplusplus
extern "C"
{
#endif

#include <sdkconfig.h>

#if CONFIG_ROBUSTO_LOAD_DS1603L
    /*********************
     *      DEFINES
     *********************/

    /*********************
     *      INCLUDES
     *********************/

    /**********************
     *      TYPEDEFS
     **********************/

    /**********************
     * GLOBAL PROTOTYPES
     **********************/

    int ds1603l_read();
    int ds1603l_init(char *log_prefix);
    /**********************
     *      MACROS
     **********************/

#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

