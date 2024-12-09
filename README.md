## BTRFS with multiple filesystems/ partitions:

BTRFS is splitted into multiple BTRFS on corresponding partitions on a NVMe device. Multiple BTRFS use a common mountpoint for users' operations.

### FIO testing:

Primary steps for running FIO tests:

        $sudo make //Make BTRFS 
        $sudo insmod btrfs.ko
        $sudo wipefs -af /dev/nvme0n1 //If in need
        $sudo ./mkfs.btrfs /dev/nvme0n1 //Make filesystems on the device with modified progs-btrfs
        $sudo mount /dev/nvme0n1 /mnt/test  //Mount the device 
        $sudo ./fio r.fio // Run FIO tests

### Filebench testing:

Primary steps for running Filebench-varmail tests:

        $sudo make
        $sudo insmod btrfs.ko
        $sudo wipefs -af /dev/nvme0n1
        $sudo ./mkfs.btrfs /dev/nvme0n1 
        $sudo mount /dev/nvme0n1 /mnt/test
        $sudo echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
        $sudo filebench -f ./varmail.f


1. btrfs_mfs_v3: Run with FIO (buffered writes, multiple jobs) without "verify=meta" => Corrupted with "verify=meta".
2. btrfs_mfs_v4.1: Run with FIO multiple jobs (e.g., 64) with smaller file size of 16MB and "verify=meta" without error, but larger file size case gets error with wrong numberio of FIO.
3. btrfs_mfs_v4.2: Pass wrong numberio of FIO verify=meta, with 32M, 64M, 128M, 256M, except 1G.
4. Current btrfs_mfs_v4.2: Pass FIO verify=meta, with all filesizes, via modifying create(), write(), and read() paths.
5. btrfs_mfs_v4.3: Modifications for other operations (close(), delete(), fsync() in Filebench-varmail).

