# Zephyr Retention System - Retained RAM with the nRF5340

Since Zephyr handles all RAM and Flash somewhere in its depths, to create a section of RAM that will be retained across reboots, we must use the [retention system](https://docs.zephyrproject.org/latest/services/retention/index.html)

To begin, refer to the RAM setup for your specific SoC. Since the nRF5340 has secure domain, we should specifically use the non-secure RAM for this purpose.

*Excerpt from nrf5340_cpuapp_partition_conf.dtsi*
```dts
/* Default SRAM planning when building for nRF5340 with
 * ARM TrustZone-M support
 * - Lowest 256 kB SRAM allocated to Secure image (sram0_s)
 * - Middle 192 kB allocated to Non-Secure image (sram0_ns)
 * - Upper 64 kB SRAM allocated as Shared memory (sram0_shared)
 *   (see nrf5340_shared_sram_planning_conf.dtsi)
 */
&sram0_image {
	reg = <0x20000000 DT_SIZE_K(448)>;
};

&sram0_s {
	reg = <0x20000000 0x40000>;
};

&sram0_ns {
	reg = <0x20040000 0x30000>;
};
```

To make this work properly and not break your application, we must:
- Define a section of RAM to be retained
- Shrink the existing RAM section to make room for the retained section

*Excerpt from nrf5340dk_nrf5340_cpuapp.overlay*
```dts
/ {
    sram@20040000 {
        compatible = "zephyr,memory-region";
        reg = <0x20040000 0x1000>;
        zephyr,memory-region = "RetainedMemory";
        status = "okay";

        retainedmemory {
            compatible = "zephyr,retained-ram";
            status = "okay";
            #address-cells = <1>;
            #size-cells = <1>;

            retention: retention@0 {
                compatible = "zephyr,retention";
                status = "okay";

                reg = <0x0 0x1000>;
                prefix = [08 04];
                checksum = <1>;
            };
        };
    };
};

&sram0_ns{
    reg = <0x20041000 0x2E000>;
};
```
Here, we have taken the address of the beginning of non-secure RAM and created a 1kB section to be retained. We then shrink the non-secure RAM to make room for the retained section. Also note that we add a header and one byte checksum so that the application can check on boot if the retained area needs to be erased.

The sample itself is a counter that increments a value every 100ms. It also sets up a timer that reboots the SoC every 10 seconds. All data is printed over RTT for easier power profiling. Note: since the SoC wakes up so frequently (and must start its high frequency oscillator, a heavy power draw), the power consumption will be necessarily higher.

*Expected output:*
```
*** Booting nRF Connect SDK v2.5.2 ***
Current time: 00:00:3478
Current time: 00:00:3479
Current time: 00:00:3480
Current time: 00:00:3481
Current time: 00:00:3482
Current time: 00:00:3483
Current time: 00:00:3484
Current time: 00:00:3485
Current time: 00:00:3486
Current time: 00:00:3487
Current time: 00:00:3488
Current time: 00:00:3489
Current time: 00:00:3490
Current time: 00:00:3491
Current time: 00:00:3492
Current time: 00:00:3493
Current time: 00:00:3494
Current time: 00:00:3495
Current time: 00:00:3496
Current time: 00:00:3497
Current time: 00:00:3498
Current time: 00:00:3499
Current time: 00:00:3500
Current time: 00:00:3501
Current time: 00:00:3502
Current time: 00:00:3503
Current time: 00:00:3504
Current time: 00:00:3505
Current time: 00:00:3506
Current time: 00:00:3507
Current time: 00:00:3508
Current time: 00:00:3509
Current time: 00:00:3510
Current time: 00:00:3511
Current time: 00:00:3512
Current time: 00:00:3513
Current time: 00:00:3514
Current time: 00:00:3515
Current time: 00:00:3516
Current time: 00:00:3517
Current time: 00:00:3518
Current time: 00:00:3519
Current time: 00:00:3520
Current time: 00:00:3521
Current time: 00:00:3522
Current time: 00:00:3523
Current time: 00:00:3524
Current time: 00:00:3525
Current time: 00:00:3526
Current time: 00:00:3527
Current time: 00:00:3528
Current time: 00:00:3529
Current time: 00:00:3530
Current time: 00:00:3531
Current time: 00:00:3532
Current time: 00:00:3533
Current time: 00:00:3534
Current time: 00:00:3535
Current time: 00:00:3536
Current time: 00:00:3537
Current time: 00:00:3538
Current time: 00:00:3539
Current time: 00:00:3540
Current time: 00:00:3541
Current time: 00:00:3542
Current time: 00:00:3543
Current time: 00:00:3544
Current time: 00:00:3545
Current time: 00:00:3546
Current time: 00:00:3547
Current time: 00:00:3548
Current time: 00:00:3549
Current time: 00:00:3550
Current time: 00:00:3551
Current time: 00:00:3552
Current time: 00:00:3553
Current time: 00:00:3554
Current time: 00:00:3555
Current time: 00:00:3556
Current time: 00:00:3557
Current time: 00:00:3558
Current time: 00:00:3559
Current time: 00:00:3560
Current time: 00:00:3561
Current time: 00:00:3562
Current time: 00:00:3563
Current time: 00:00:3564
Current time: 00:00:3565
Current time: 00:00:3566
Current time: 00:00:3567
Current time: 00:00:3568
*** Booting nRF Connect SDK v2.5.2 ***
Current time: 00:00:3572
Current time: 00:00:3573
Current time: 00:00:3574
Current time: 00:00:3575
Current time: 00:00:3576
Current time: 00:00:3577
```
