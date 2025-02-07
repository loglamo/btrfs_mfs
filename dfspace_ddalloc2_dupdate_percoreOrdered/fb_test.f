set $dir=/mnt/test
#set $nfiles=1000000
set $nfiles=16
#set $meandirwidth=10000
set $meandirwidth=16
#set $filesize=cvar(type=cvar-gamma,parameters=mean:131072;gamma:1.5)
#set $filesize=cvar(type=cvar-gamma,parameters=mean:19660800;gamma:1.5)
#set $filesize=cvar(type=cvar-gamma,parameters=mean:3221225472;gamma:1.5)
#set $filesize=cvar(type=cvar-uniform,parameters=mean:5368709120)
set $filesize=12g
set $nthreads=16
set $iosize=4k
set $meanappendsize=1m
#set $meanappendsize=16k
set $runtime=120


#define fileset name=bigfileset,path=$dir,size=$filesize,entries=$nfiles,dirwidth=$meandirwidth,prealloc=80
define fileset name=bigfileset,path=$dir,size=$filesize,entries=$nfiles,dirwidth=$meandirwidth,prealloc=100

define process name=filereader,instances=16
{
    thread name=filereaderthread,instances=1
    {
    # flowop openfile name="openfile1",filesetname="bigfileset"
    flowop write name="rand-write1",filesetname="bigfileset",iosize=$iosize,random
    # flowop closefile name="closefile1"
   }
}

run $runtime
