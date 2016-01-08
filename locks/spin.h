#ifdef __cplusplus
extern "C" {
#endif

#define SPIN_LOCK_INIT 1

extern void spin_init_hle(int *);
extern void spin_lock_hle(int *);
extern void spin_unlock_hle(int *);

extern void spin_init_rtm(int *);
extern void spin_lock_rtm(int *);
extern void spin_unlock_rtm(int *);

#ifdef __cplusplus
}
#endif
