#ifndef __ALSA_TOOLS_ALSA_CONTEXT__
#define __ALSA_TOOLS_ALSA_CONTEXT__

#include <stdint.h>

typedef struct alsa_context_reactor ALSAContextReactor;
ALSAContextReactor *alsa_context_reactor_create(unsigned int id, int *err);
void alsa_context_reactor_start(ALSAContextReactor *reactor, int *err);
void alsa_context_reactor_stop(ALSAContextReactor *reactor);
void alsa_context_reactor_destroy(ALSAContextReactor *reactor);

typedef struct alsa_context_reactant ALSAContextReactant;
typedef enum {
	ALSAContextReactantStateReadable,
	ALSAContextReactantStateWritable,
	ALSAContextReactantStateError,
} ALSAContextReactantState;
typedef void (*ALSAContextReactantCallback)(ALSAContextReactantState state,
					    void *private_data,
					    int *err);

ALSAContextReactant *alsa_context_reactant_create(unsigned int id, int fd,
				       ALSAContextReactantCallback callback,
				       void *private_data, int *err);
void alsa_context_reactant_add(ALSAContextReactant *reactant, uint32_t events,
			       int *err);
void alsa_context_reactant_remove(ALSAContextReactant *reactant);
void alsa_context_reactant_destroy(ALSAContextReactant *reactant);

#endif
