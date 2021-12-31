tftp-put:
	cd buildroot/output/images && \
		tftp 192.168.1.1 -m binary -c put vmlinux.bin

full-ct-diff:
	cd linux-5-git && \
		git diff 6f6e3714c17... > ../full-ct.diff
