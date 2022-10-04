

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * BMV700 series library.
 * 
 * Copyright Nicklas Börjesson, 2022
 */
#ifndef __BMV700__
#define __BMV700__


//#include <Stream.h>                                             // Generic Stream library, upon which a.o. Serial and SoftwareSerial are built.

int init_bmv700(char *log_prefix);


char* read_bmv700();

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif