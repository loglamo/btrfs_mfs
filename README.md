## BTRFS with multiple filesystems/ partitions:

BTRFS is splitted into multiple BTRFS on corresponding partitions on a NVMe device. Multiple BTRFS use a common mountpoint for users' operations.

### FIO testing:

Primary steps in running FIO tests:

        $sudo make //Make BTRFS 
        $sudo insmod btrfs.ko
        $sudo ./mkfs.btrfs /dev/nvme0n1 //Make filesystems on the device with modified progs-btrfs
        $sudo mount /dev/nvme0n1 /mnt/test  //Mount the device 
        $sudo ./fio r.fio 
1. btrfs_mfs_v3: Run with FIO (buffered writes, multiple jobs) without "verify=meta" => Corrupted with "verify=meta".
2. btrfs_mfs_v4.1: Run with FIO multiple jobs (e.g., 64) with smaller file size of 16MB and "verify=meta" without error, but larger file size case gets error with wrong numberio of FIO.
3. btrfs_mfs_v4.2: Pass wrong numberio of FIO verify=meta, with 32M, 64M, 128M, 256M, except 1G.
4. Current btrfs_mfs_v4.2: Pass FIO verify=meta, with all filesizes.
