.PHONY: all 
all: Bootman

.PHONY: all-iso
all-iso: Bootman.iso

.PHONY: all-img
all-img: Bootman.img

.PHONY: all-clean
all-clean: clean

Bootman: boot-bios boot-cd
	fasm Bootman/bootman.asm
	mkdir -p bin
	cp -rf boot/*.bin bin
	cp -rf Bootman/*.sys bin

Bootman.iso: boot-bios boot-cd
	fasm Bootman/bootman.asm
	rm -rf iso_root
	mkdir -p iso_root
	mkdir -p iso_root/system16
	cp Bootman/bootman.sys iso_root/system16/
	mkdir iso_root/boot
	cp -r boot/*.bin iso_root/boot/
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
	parted -s bootman.img mkEart ESP fat32 2048s 100%
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
clean:

	rm -rf Bootman.iso Bootman.img
	find . -name "*.bin" -type f -delete
	find . -name "*.SYS" -type f -delete
	find . -name "*.sys" -type f -delete
	find . -name "*.exe" -type f -delete
	find . -name "*.o" -type f -delete
