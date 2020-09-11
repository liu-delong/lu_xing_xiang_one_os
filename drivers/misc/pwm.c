/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        pwm.c
 *
 * @brief       this file implements pwm related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <misc/pwm.h>

os_err_t os_pwm_enable(struct os_pwm_device *pwm, os_uint32_t channel)
{
    os_err_t result = OS_EOK;

    OS_ASSERT(pwm != OS_NULL);

    result = pwm->ops->enabled(pwm, channel, OS_TRUE);

    return result;
}

os_err_t os_pwm_disable(struct os_pwm_device *pwm, os_uint32_t channel)
{
    os_err_t result = OS_EOK;

    OS_ASSERT(pwm != OS_NULL);

    result = pwm->ops->enabled(pwm, channel, OS_TRUE);

    return result;
}

os_err_t os_pwm_set_period(struct os_pwm_device *pwm, os_uint32_t channel, os_uint32_t period)
{
    pwm->period = period;
    pwm->ops->enabled(pwm, channel, OS_FALSE);
    return pwm->ops->set_period(pwm, channel, period);
}

os_err_t os_pwm_set_pulse(struct os_pwm_device *pwm, os_uint32_t channel, os_uint32_t pulse)
{   
    return pwm->ops->set_pulse(pwm, channel, pulse);
}

os_size_t _pwm_set(struct os_device *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    return os_pwm_set_pulse((struct os_pwm_device *)dev, pos, *(os_int32_t *)buffer);
}

static os_err_t _pwm_control(struct os_device *dev, int cmd, void *args)
{
    struct os_pwm_device *pwm    = (struct os_pwm_device *)dev;

    switch (cmd)
    {
    case OS_PWM_CMD_ENABLE:
        return pwm->ops->enabled(pwm, *(os_uint32_t *)args, OS_TRUE);
    case OS_PWM_CMD_DISABLE:
        return pwm->ops->enabled(pwm, *(os_uint32_t *)args, OS_FALSE);
    case OS_PWM_CMD_SET_PERIOD:
        return os_pwm_set_period((struct os_pwm_device *)dev, 0, *(os_uint32_t *)args);
    default:
        return OS_ENOSYS;
    }
}

#ifdef OS_USING_DEVICE_OPS
static const struct os_device_ops pwm_device_ops = {
    OS_NULL,
    OS_NULL,
    OS_NULL,
    _pwm_set,
    _pwm_control
};
#endif /* OS_USING_DEVICE_OPS */

/**
 ***********************************************************************************************************************
 * @brief           register pwm device
 *
 * @param[in]       device          pointer of pwm device
 * @param[in]       name            pointer of pwm name
 * @param[in]       ops             pointer of pwm operation function set
 * @param[in]       user_data       pointer of tim_handle
 *
 * @return          os_err_t
 * @retval          OS_EOK          run successfully
 * @retval          OS_EINVAL       pointer of device or name is NULL or device not exist
 ***********************************************************************************************************************
 */
os_err_t os_device_pwm_register(struct os_pwm_device       *device,
                                const char              *name)
{
    os_err_t result = OS_EOK;

#ifdef OS_USING_DEVICE_OPS
    device->parent.ops = &pwm_device_ops;
#else
    device->parent.write   = _pwm_set;
    device->parent.control = _pwm_control;
#endif /* OS_USING_DEVICE_OPS */

    device->parent.type      = OS_DEVICE_TYPE_MISCELLANEOUS;

    result = os_device_register(&device->parent, name, OS_DEVICE_FLAG_RDWR);

    return result;
}

