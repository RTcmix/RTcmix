#ifndef _PROTOTYPES_H_ 
#define PROTOTYPES_H_ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void rtsendzeros(int);
void rtreportstats(void);
int rtcloseout(void);
void close_audio_ports(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _PROTOTYPES_H_ */
