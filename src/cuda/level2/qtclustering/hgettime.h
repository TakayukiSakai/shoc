#ifndef NESTED_GETTIME_H_INC
#define NESTED_GETTIME_H_INC

/**
 *  Measuring the time of code regions. 
 *  The regions can be nested.
 *
 *  You have to Specify "GETTIME_HAVE_INSTANCE" before #include statement
 *  in a source file. All the other source files must include this header 
 *  without * specifying "GETTIME_HAVE_INSTANCE"
 *
 *  Warning.
 *  Never include this header in any header files.
 *
 *  Written by Shingo Hayashi. Refactored By Toshiya Komoda.
 *  2013 0402
 */

#include <string.h>
#include <sys/time.h>
#include <time.h>

#define GETTIME_STACK_SIZE 32
#define GETTIME_RECORD_SIZE 64
#define GETTIME_STRLENGTH 64

typedef struct {
    char name[GETTIME_STRLENGTH];
    __int64_t time;
} gettime_record_t;

typedef struct {
    __int64_t starttime;
    gettime_record_t *record;
} gettime_stack_t;

typedef struct {
    struct tm start_lt;
    struct tm end_lt;
} gettime_lt_t;

#ifdef GETTIME_HAVE_INSTANCE
gettime_stack_t gettime_stack[GETTIME_STACK_SIZE];
int gettime_stack_u = 0;
gettime_record_t gettime_record[GETTIME_RECORD_SIZE];
int gettime_record_u = 0;
gettime_lt_t gettime_lt_stamp;


#ifdef __cplusplus
extern "C" {
#endif
void gettime_fatal( const char *msg )
{
    int *p = NULL;
    fprintf( stderr, "%s\n", msg );
    *p = 100;
}

void gettime_dump()
{
    int i;
    if( gettime_stack_u != 0 ) {
        gettime_fatal( "gettime: stack is not empty" );
    }
    printf("\n== Dump Timing Stat ==\n");
    printf("core_start, [%02d:%02d:%02d:%02d]\n",
        gettime_lt_stamp.start_lt.tm_mday,
        gettime_lt_stamp.start_lt.tm_hour,
        gettime_lt_stamp.start_lt.tm_min,
        gettime_lt_stamp.start_lt.tm_sec);
    printf("core_end, [%02d:%02d:%02d:%02d]\n",
        gettime_lt_stamp.end_lt.tm_mday,
        gettime_lt_stamp.end_lt.tm_hour,
        gettime_lt_stamp.end_lt.tm_min,
        gettime_lt_stamp.end_lt.tm_sec);
    for( i=0; i < gettime_record_u; i++ ) {
        printf("%s, %d\n", gettime_record[i].name, (int)gettime_record[i].time );
    }
}

#ifdef __cplusplus
}
#endif

#else
extern gettime_stack_t gettime_stack[GETTIME_STACK_SIZE];
extern int gettime_stack_u;
extern gettime_record_t gettime_record[GETTIME_RECORD_SIZE];
extern int gettime_record_u;
extern gettime_lt_t gettime_lt_stamp;

#ifdef __cplusplus
extern "C" {
#endif
extern void gettime_fatal( const char *msg );
void gettime_dump();
#ifdef __cplusplus
}
#endif

#endif

static inline void gettime_set_startlocaltime()
{
  time_t t;
  time(&t);
  memcpy(&gettime_lt_stamp.start_lt, localtime(&t), sizeof(struct tm));
}

static inline void gettime_set_endlocaltime()
{
  time_t t;
  time(&t);
  memcpy(&gettime_lt_stamp.end_lt, localtime(&t), sizeof(struct tm));
}

static inline gettime_record_t* gettime_getrecord( const char *name )
{
    int i;
    gettime_record_t *p;
    for( i=0; i < gettime_record_u; i++ ) {
        if( strcmp( name, gettime_record[i].name ) == 0 ) {
            return &gettime_record[i];
        }
    }
    if( gettime_record_u < GETTIME_RECORD_SIZE ) {
        p = &gettime_record[gettime_record_u];
        strcpy( p->name, name );
        p->time = 0;
        gettime_record_u++;
        return p;
    }
    gettime_fatal( "gettime: record overflow" );
    return NULL;
}

static inline __int64_t gettime_nowtime()
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return tv.tv_sec*1000000 + tv.tv_usec;
}

static inline void gettime_start( const char *name )
{
    if( gettime_stack_u >= GETTIME_STACK_SIZE ) {
        gettime_fatal( "gettime: stack oveflow" );
    }
    gettime_stack[gettime_stack_u].starttime = gettime_nowtime();
    gettime_stack[gettime_stack_u].record = gettime_getrecord( name );
    gettime_stack_u++;
}

static inline int gettime_end()
{
    __int64_t nowtime, diff;
    if( gettime_stack_u <= 0 ) {
        gettime_fatal( "gettime: stack underflow" );
    }
    nowtime = gettime_nowtime();
    diff = nowtime - gettime_stack[gettime_stack_u-1].starttime;
    gettime_stack[gettime_stack_u-1].record->time += diff;
    ;
    gettime_stack_u--;
    return (int)diff;
}

static inline void gettime_start_n( gettime_stack_t *t, const char *name )
{
    t->starttime = gettime_nowtime();
    t->record = gettime_getrecord( name );
}

static inline int gettime_end_n( gettime_stack_t *t )
{
    __int64_t nowtime, diff;
    nowtime = gettime_nowtime();
    diff = nowtime - t->starttime;
    t->record->time += diff;
    return (int)diff;
}

static inline void gettime_add_time( int t, const char *name ) {
    gettime_record_t *p = gettime_getrecord( name );
    p->time += t;
}

#endif
