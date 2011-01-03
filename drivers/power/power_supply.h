

#ifdef CONFIG_SYSFS

extern int power_supply_create_attrs(struct power_supply *psy);
extern void power_supply_remove_attrs(struct power_supply *psy);
extern int power_supply_uevent(struct device *dev, struct kobj_uevent_env *env);

#else

static inline int power_supply_create_attrs(struct power_supply *psy)
{ return 0; }
static inline void power_supply_remove_attrs(struct power_supply *psy) {}
#define power_supply_uevent NULL

#endif 

#ifdef CONFIG_LEDS_TRIGGERS

extern void power_supply_update_leds(struct power_supply *psy);
extern int power_supply_create_triggers(struct power_supply *psy);
extern void power_supply_remove_triggers(struct power_supply *psy);

#else

static inline void power_supply_update_leds(struct power_supply *psy) {}
static inline int power_supply_create_triggers(struct power_supply *psy)
{ return 0; }
static inline void power_supply_remove_triggers(struct power_supply *psy) {}

#endif 
