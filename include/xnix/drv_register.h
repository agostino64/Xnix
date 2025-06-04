#ifndef DRV_MGR_REGISTER_H
#define DRV_MGR_REGISTER_H

typedef int (*driver_init_func_t)(void);

struct driver_registration {
    driver_init_func_t init;
    int driver_id;
};

#define REGISTER_DRIVER(func, id) \
    static const struct driver_registration drv_##id \
    __attribute__((section(".drivers"), used)) = { \
        .init = func, \
        .driver_id = id \
    }

#endif
