#! /bin/bash

sudo umount -v /mnt/test
sudo rmmod btrfs
sudo wipefs -af /dev/nvme0n1 /dev/nvme1n1 /dev/nvme2n1 /dev/nvme3n1 /dev/nvme4n1 /dev/nvme5n1 /dev/nvme6n1 /dev/nvme7n1
make -j64
retval=$?
if [ $retval -ne 0 ]; then
    echo "Make Failed with return $retval"
    exit $retval
fi
sudo insmod btrfs.ko
sudo mkfs.btrfs -vf -d raid0 /dev/nvme0n1 /dev/nvme1n1 /dev/nvme2n1 /dev/nvme3n1 /dev/nvme4n1 /dev/nvme5n1 /dev/nvme6n1 /dev/nvme7n1
sudo mount -v /dev/nvme0n1 /mnt/test

