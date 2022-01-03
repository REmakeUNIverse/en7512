IMAGE_DIR=buildroot/output/images

tput: add-dtb
	cd buildroot/output/images && \
		tftp 192.168.1.1 -m binary -c put vmlinux.bin

full-ct-diff:
	cd linux-5-git && \
		git diff 6f6e3714c17... > ../full-ct.diff

dtb:
	dtc -I dts -O dtb -o innboxe80_pw2.dtb innboxe80_pw2.dtsi

add-dtb: dtb
	cat buildroot/output/images/vmlinux.bin \
		innboxe80_pw2.dtb > vmlinux-dtb.bin

tput-dtb: add-dtb
	tftp 192.168.1.1 -m binary -c put vmlinux-dtb.bin
