tftp-put:
	cd buildroot/output/build/linux-custom/arch/mips/boot && \
		tftp 192.168.1.1 -m binary -c put vmlinux.bin
