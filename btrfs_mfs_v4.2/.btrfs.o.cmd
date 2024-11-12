savedcmd_/home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.o := ld -m elf_x86_64 -z noexecstack   -r -o /home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.o @/home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.mod  ; ./tools/objtool/objtool --hacks=jump_label --hacks=noinstr --hacks=skylake --ibt --retpoline --rethunk --sls --stackval --static-call --uaccess --prefix=16  --link  --module /home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.o

/home/syslab/workspace_la/btrfs_mfs_v4.2/btrfs.o: $(wildcard ./tools/objtool/objtool)
