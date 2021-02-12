#ifndef DEF_CONSTANTES
#define DEF_CONSTANTES



#define    MAKE_POS(x, y)   ((y << 16) | (x))
#define    BLOC_W           16
#define    BLOC_H           16

/* definition des directions */
enum
{
    LABF_NORD  = 0x01,
    LABF_EST   = 0x02,
    LABF_SUD   = 0x04,
    LABF_OUEST = 0x08,
    LABF_NSOE  = 0x0f,
    LABF_VISIT = 0x10
};
static int opposite[] = {0, LABF_SUD, LABF_OUEST, 0, LABF_NORD, 0, 0, 0, LABF_EST};

#endif
