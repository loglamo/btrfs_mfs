savedcmd_/home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.ko := ld -r -m elf_x86_64 -z noexecstack --build-id=sha1  -T scripts/module.lds -o /home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.ko /home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.o /home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.mod.o;  make -f ./arch/x86/Makefile.postlink /home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.ko