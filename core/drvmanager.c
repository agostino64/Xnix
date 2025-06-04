#include <xnix/drv_manager.h>
#include <xnix/drv_register.h>

extern struct driver_registration __start_drivers[];
extern struct driver_registration __stop_drivers[];

int drv_load(int driver_id) {
    for (struct driver_registration *drv = __start_drivers; 
         drv < __stop_drivers; 
         drv++) {
        if (drv->driver_id == driver_id) {
            return drv->init();
        }
    }
    return -1;
}
