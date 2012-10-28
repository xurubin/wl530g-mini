mount -t proc proc /proc
mount -t ramfs none /var
mkdir /var/tmp
mkdir /var/tmp/ppp
mkdir /var/log
mkdir /var/run
mkdir /var/lock

#Initialise /etc directory
mount -t ramfs none /etc
loadetc
## skip existing file when copying
cp -se /etc0/* /etc/

source /etc/rc
