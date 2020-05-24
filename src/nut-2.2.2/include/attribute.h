/* portability hacks for __attribute__ usage in other header files */

#ifndef ATTRIBUTE_H_SEEN
#define ATTRIBUTE_H_SEEN 1

#ifndef __attribute__
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 8) || __STRICT_ANSI__
#  define __attribute__(x)
# endif
#endif

#endif /* ATTRIBUTE_H_SEEN */
