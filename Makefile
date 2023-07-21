.PHONY: all
all: iso

.PHONY: all-hdd
all-hdd: barebones.hdd

.PHONY: run
run: iso
	qemu-system-x86_64 -M pc-i440fx-4.2 -m 2G -cdrom elysia.iso -boot d -enable-kvm -s  -monitor stdio

.PHONY: run-gdb
run-gdb: iso
	qemu-system-x86_64 -M pc-i440fx-4.2 -m 4G -cdrom elysia.iso -boot d -enable-kvm -s -parallel stdio -d int,guest_errors -S

.PHONY: run-uefi
run-uefi: ovmf-x64 iso
	qemu-system-x86_64 -M q35 -m 2G -bios ovmf-x64/OVMF.fd -cdrom iso -boot d

.PHONY: run-hdd
run-hdd: elysia.hdd
	qemu-system-x86_64 -M q35 -m 2G -accel kvm -hda elysia.hdd

.PHONY: run-hdd-uefi
run-hdd-uefi: ovmf-x64 barebones.hdd
	qemu-system-x86_64 -M q35 -m 2G -bios ovmf-x64/OVMF.fd -hda barebones.hdd

ovmf-x64:
	mkdir -p ovmf-x64
	cd ovmf-x64 && curl -o OVMF-X64.zip https://efi.akeo.ie/OVMF/OVMF-X64.zip && 7z x OVMF-X64.zip

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v4.x-branch-binary --depth=1
	make -C limine

.PHONY: kernel
kernel:
	mkdir -p build/kernel
	$(MAKE) -C kernel

iso: limine kernel
	rm -rf iso_root
	mkdir -p iso_root
	cp build/kernel.elf \
		limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
	@xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o elysia.iso > /dev/null 2>&1
	@limine/limine-deploy elysia.iso > /dev/null 2>&1
	@echo "\033[0;32m[Ok] ISO image generated successfully!\033[0;37m"		

barebones.hdd: limine kernel
	rm -f elysia.hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=elysia.hdd
	parted -s elysia.hdd mklabel gpt
	parted -s elysia.hdd mkpart ESP fat32 2048s 100%
	parted -s elysia.hdd set 1 esp on
	limine/limine-deploy elysia.hdd
	sudo losetup -Pf --show elysia.hdd >loopback_dev
	sudo mkfs.fat -F 32 `cat loopback_dev`p1
	mkdir -p img_mount
	sudo mount `cat loopback_dev`p1 img_mount
	sudo mkdir -p img_mount/EFI/BOOT
	sudo cp -v build/kernel.elf limine.cfg limine/limine.sys img_mount/
	sudo cp -v limine/BOOTX64.EFI img_mount/EFI/BOOT/
	sync
	sudo umount img_mount
	sudo losetup -d `cat loopback_dev`
	sudo rm -rf loopback_dev img_mount

.PHONY: clean
clean:
	rm -rf iso_root iso elysia.hdd build
	$(MAKE) -C kernel clean	

.PHONY: distclean
distclean: clean
	rm -rf limine ovmf-x64
	$(MAKE) -C souce distclean
