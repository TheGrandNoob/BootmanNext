.PHONY: all 
all: Bootman.iso

.PHONY: all-iso
all-iso: Bootman.iso

.PHONY: all-img
all-img: Bootman.img

.PHONY: all-clean
all-clean: clean


Bootman.iso: boot-bios boot-cd bootman.SYS
	rm -rf iso_root
	mkdir -p iso_root
	mkdir -p iso_root/system16
	cp -r osRoot/* iso_root/
	mkdir iso_root/boot
	cp -r boot/*.bin iso_root/boot/
	cp Bootman/bootman.SYS iso_root/system16/
	mkisofs -U -J \
		-b boot/cdboot.bin\
		-no-emul-boot -c boot/boot.cat\
		-V "Bootman" -A "Necorupt"\
		-sysid "Bootman"  -iso-level 3 -graft-points\
		-o Bootman.iso iso_root
		
	rm -rf iso_root

Bootman.img: boot-bios boot-cd
	rm -f bootman.img
	dd if=/dev/zero bs=1M count=0 seek=64 of=bootman.img
	parted -s bootman.img mklabel gpt
	parted -s bootman.img mkpart ESP fat32 2048s 100%
	parted -s bootman.img set 1 esp on
	
	sudo losetup -Pf --show bootman.hdd >loopback_dev
	sudo mkfs.fat -F 32 `cat loopback_dev`p1
	mkdir -p img_mount
	sudo mount `cat loopback_dev`p1 img_mount
	sudo mkdir -p img_mount/boot
	sudo cp -v osRoot/* img_mount/
	
	sync
	sudo umount img_mount
	sudo losetup -d `cat loopback_dev`
	rm -rf loopback_dev img_mount

boot-bios:
	fasm boot/mbr.asm
boot-cd:
	fasm boot/cdboot.asm
bootman.SYS:
	fasm Bootman/bootman.asm

clean:
	rm -rf Bootman.iso Bootman.img
	rm -rf *.bin
	rm -rf iso_root
	rm -rf *.SYS
