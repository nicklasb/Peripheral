
#ifdef __cplusplus
extern "C"
{
#endif

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

    /* Tasks for handling work items */
    int init_ds1603l(char *log_prefix);
    int read_sd1603l();
    /**********************
     *      MACROS
     **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif
