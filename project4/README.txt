Efe Acer - 21602217
Yusuf Dalva - 21602867

In order to compile the kernel module type
-> make
in this directory. If you want to recompile type:
-> make clean
-> make
again in this directory.

To run kernel modeule type:
-> sudo insmod project4_EA_YD.ko processid=<your_process_id>
-> sudo rmmod project4_EA_YD.ko
-> cat /var/log/syslog

To compile the app.c program type:
-> gcc -o app app.c

To run the app.c program type:
-> ./app


 
