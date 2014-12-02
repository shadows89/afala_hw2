echo Recompiling...
cd /usr/src/linux-2.4.18-14custom
make bzImage
echo Replacing boot
cd arch/i386/boot
cp bzImage /boot/vmlinuz-2.4.18-14custom
echo Done. You can reboot now.