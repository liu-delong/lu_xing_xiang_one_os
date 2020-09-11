#ifndef __USER_HUMITURE_H__
#define __USER_HUMITURE_H__

#ifndef IOCTL_lightsensor_READ
#define IOCTL_lightsensor_READ             (15)
#endif
#define LIGHTSENSOR_INIT_FLAG (1)
#define LIGHTSENSOR_DEINIT_FLAG (0)

struct lightsensordata
{
    float light;
    int proximitys;
};

#endif
