/*
 * Copyright (c) 2024 Kelly Helmut Lord
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>
#include <zephyr/device.h>
#include <zephyr/retention/retention.h>

const struct device *retention = DEVICE_DT_GET(DT_NODELABEL(retention));

struct time {
	uint32_t hour;
	uint32_t minute;
	uint32_t centisecond;
};

struct k_timer reset_timer;
extern void reset_timer_handler(struct k_timer *timer_id) {
	sys_reboot(0);
}

int main(void)
{
	int ret;
	struct time current_time = {0};

	k_timer_init(&reset_timer, reset_timer_handler, NULL);
	k_timer_start(&reset_timer, K_SECONDS(10), K_NO_WAIT);

	if (!device_is_ready(retention)) {
		printk("Retention domain device is not ready\n");
		return -ENODEV;
	}


	if (retention_is_valid(retention) == 0) {
		printk("Retention data is not valid\n");
		ret = retention_clear(retention);
		if (ret) {
			printk("Failed to clear retention data\n");
			return ret;
		}
	}

	while (1) {

		ret = retention_read(retention, 0, &current_time, sizeof(current_time));
		if (ret) {
			printk("Failed to read retention data\n");
			return ret;
		}

		printk("Current time: %02d:%02d:%02d\n",
		       current_time.hour, current_time.minute, current_time.centisecond);

		current_time.centisecond++;

		ret = retention_write(retention, 0, &current_time, sizeof(current_time));	
		k_msleep(100);
	}
	return 0;
}
