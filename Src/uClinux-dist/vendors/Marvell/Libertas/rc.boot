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
cd /etc0/
cp -se * /etc/

source /etc/rc
