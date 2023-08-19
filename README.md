# Responsive

A project that aims to improve the responsiveness of your typical desktop
linux system using sysctl options, zram and adjustments to the swap.

## But why?

Defaults in the linux eco system are outdated. For example, `swapiness` is set
to 60 by default which is unideal number for modern systems. Swap priority is
also bad by default.

## How does it work?

It detects the type of hard disk that root is mounted on, and its r/w speed,
then sets many sysctl values accordingly.

## TODO

**Warning: Response is in very early alpha state**

* ~~Detect SATA HDD and SSD from whole drives (sda) or partitions (sda1).~~
* Detect NVMe SSD from whole drives (nvme0n1) or partitions (nvme0n1p1).
* Detect loop devices.
* Detect IDE drives.
* Detect floppy disks.
* Detect USB disks.
* Detect underlying hard disk from lvm or encrypted volume.
* Come up with a formula for swappiness and swap priority.
* Add zram support.

