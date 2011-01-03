

#ifdef CONFIG_HAVE_TCM
void __init tcm_init(void);
#else

inline void tcm_init(void)
{
}
#endif
