#include <obstack.h>
#include <signal.h>

extern int suppressint;
extern volatile int intpending;
extern int intrnewline;
extern struct obstack mainobstack;
extern struct obstack lineobstack;

#ifdef __GNUC__
void intr(int) __attribute__ ((noreturn));
#else
void intr(int);
#endif

#define INTOFF suppressint++;
#define INTON do { if (--suppressint == 0 && intpending) intr(SIGINT); } \
	      while(0)
#define CHECKINT do { if (suppressint == 1 && intpending) { \
		 suppressint = 0; intr(SIGINT); }} while(0)
