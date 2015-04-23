/* 
 * File:   port.h
 * Author: karlp
 *
 * port.h is a _required_ include from freemodbus, but is not included in the
 * BSD licensed portion of the distribution.  I feel this is a major
 * complication, and given how google et al feel about licensing of headers,
 * I have no bad feelings about copying the required portions from the "example"
 * port.h file in the demo directory.
 */

#ifndef PORT_H
#define	PORT_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define ENTER_CRITICAL_SECTION( )	vMBPortEnterCritical()
#define EXIT_CRITICAL_SECTION( )	vMBPortExitCritical()

typedef uint8_t BOOL;

typedef unsigned char UCHAR;
typedef char CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

void vMBPortEnterCritical( void );
void vMBPortExitCritical( void );


#ifdef	__cplusplus
}
#endif

#endif	/* PORT_H */

