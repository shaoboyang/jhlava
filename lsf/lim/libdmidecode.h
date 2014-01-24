#ifndef _LIBDMIDECODE_H_
#define _LIBDMIDECODE_H_
#include <unistd.h>
/** 
 * @Description   : this function get the system product name form BIOS according to the SMBIOS/DMI standard.
 * 
 * @param p [out] : give a buff address ,return the string of system-product-name 
 * 
 * @return        : 0 success, -1 failed
 */
int dmidecode(char *p, const size_t len);

#endif /* _LIBDMIDECODE_H_ */

