#ifndef SURENV_HEADER_FILE
#define SURENV_HEADER_FILE

#ifdef __unix__
#define ENV_ADV_UNIX
#endif

/* Fuck all Windows users and their shitty fucking operating system that can't do anything right */
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#define ENV_ADV_WIN
#endif

#endif
