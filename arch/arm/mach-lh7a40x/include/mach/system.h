

static inline void arch_idle(void)
{
	cpu_do_idle ();
}

static inline void arch_reset(char mode, const char *cmd)
{
	cpu_reset (0);
}
