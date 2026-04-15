/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

void itoa(int a, char *b);

int strlen(char *a);

int gettime(void);
int getpid(void);
int write(int fd, char *buffer, int size);

extern int errno;

void perror(const char *s);

#endif  /* __LIBC_H__ */
