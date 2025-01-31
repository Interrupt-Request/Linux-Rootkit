#!/usr/bin/env bash
hardDisk="/VirtualMachines/Linux/archlinux"
memory="8G"
smp="4"
uuid="161f80e2-a775-4431-97c0-fee2bf2293f2"
shared="./qemu/shared"
efivars="./qemu/OVMF_VARS.4m.fd"

echo "config loaded"
mkdir -p ./qemu/shared

alias runqemuGTK="
	qemu-system-x86_64 \
	-enable-kvm \
	-machine q35 \
	-uuid $uuid \
	-m $memory \
	-cpu host -smp $smp \
	-device intel-iommu \
	-display gtk,gl=on \
	-vga none \
	-device virtio-vga-gl,hostmem=4G  \
	-device virtio-blk-pci,drive=drive0,id=virtblk0,num-queues=4,serial=MYDISK-1 \
	-drive file=$hardDisk,if=none,format=qcow2,id=drive0 \
	-virtfs local,path=$shared,mount_tag=shared,security_model=mapped-xattr \
	-boot d \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd -drive if=pflash,format=raw,file=$efivars \
	-name 'Arch Linux SDL GTK' \
	-monitor stdio \
	-parallel none \
	-serial vc \
	-k es \
	-usbdevice tablet \
-device virtio-net,netdev=vmnic -netdev user,id=vmnic,net=192.168.50.0/24,dhcpstart=192.168.50.20 \
	-audiodev pipewire,id=snd0 -device ich9-intel-hda"

## Nvidia
alias runqemuSDL="
	qemu-system-x86_64 \
	-enable-kvm \
	-machine q35 \
	-uuid $uuid \
	-m $memory \
	-cpu host -smp $smp \
	-device intel-iommu \
	-display sdl,gl=on \
	-vga none \
	-device virtio-vga-gl,hostmem=4G  \
	-device virtio-blk-pci,drive=drive0,id=virtblk0,num-queues=4,serial=MYDISK-1 \
	-drive file=$hardDisk,if=none,format=qcow2,id=drive0 \
	-virtfs local,path=$shared,mount_tag=shared,security_model=mapped-xattr \
	-boot d \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd -drive if=pflash,format=raw,file=$efivars \
	-name 'Arch Linux SDL' \
	-monitor stdio \
	-parallel none \
	-serial none \
	-k es \
	-usbdevice tablet \
	-device virtio-net,netdev=vmnic -netdev user,id=vmnic,net=192.168.50.0/24,dhcpstart=192.168.50.20 \
	-audiodev pipewire,id=snd0 -device ich9-intel-hda"

alias runqemu="
	qemu-system-x86_64 \
	-enable-kvm \
	-machine q35 \
	-uuid $uuid \
	-m $memory \
	-cpu host -smp $smp \
	-device intel-iommu \
	-device virtio-blk-pci,drive=drive0,id=virtblk0,num-queues=4,serial=MYDISK-1 \
	-drive file=$hardDisk,if=none,format=qcow2,id=drive0 \
	-virtfs local,path=$shared,mount_tag=shared,security_model=mapped-xattr \
	-boot d \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd -drive if=pflash,format=raw,file=$efivars \
	-name 'Arch Linux SDL' \
	-monitor stdio \
	-parallel none \
	-serial none \
	-k es \
	-usbdevice tablet \
	-device virtio-net,netdev=vmnic -netdev user,id=vmnic,net=192.168.50.0/24,dhcpstart=192.168.50.20 \
	-audiodev pipewire,id=snd0 -device ich9-intel-hda"