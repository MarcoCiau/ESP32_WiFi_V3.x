#ifndef NOFOS_NETWORK_H
#define NOFOS_NETWORK_H
#ifndef ENABLE_NOFOS_GTWY
#undef ENABLE_NOFOS_GTWY
#endif
#ifdef ENABLE_NOFOS_GTWY
#include <Arduino.h>
extern void nofos_network_begin();
extern void nofos_network_loop();
extern void nofos_network_set_profile(int profile);
#endif // ENABLE_NOFOS_GTWY
#endif // NOFOS_NETWORK_H