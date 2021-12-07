#include "skbuff.h"
#include "eth.h"
#include "arp.h"
#include "ip.h"
#include "udp.h"
#include "tftp.h"
#include "tftpput.h"

#include "osdep.h"
#ifdef BOOT_LZMA_SUPPORT
/* frankliao added 20100511 */
#include "flashhal.h"       		/* for tftp_move_recdata() */
#include "../../../tools/trx/trx.h" /* for trx header size */
#include <linux/string.h>   		/* for tftp_rcv_wrq() */
#endif
#include "common.h"


extern void my_flash_boot(void);
extern void my_flash_kernel(void);
extern void my_flash_rootfs(void);
extern void my_flash_image(void);
/* add by liyao */
unsigned long client_ip;
unsigned short client_port;
unsigned short client_block;

#ifdef BOOT_LZMA_SUPPORT
/* the name of the received data */
static char filename[ FILE_NAME_LENGTH ];
/* the size of the received data */
//static int rcvdata_size = 0;
int rcvdata_size = 0;
/* file type :  TCBOOT : tcboot.bin
 *              TCLINUX : tclinux.bin */
static short filetype = 0;

uint32_t crc32buf(char *buf, size_t len, int flags);
static int tftp_write_flash(void);
int checkfile(short filetype, int rcvdata_size, char *start);
#endif

int tftp_send_ack(struct tftphdr *tftp_hdr, int block)
{
	struct tftphdr *tftp_ack;
	sk_buff *skb;

	skb = alloc_skb(ETH_FRAME_LEN);
	udp_skb_reserve(skb);
	tftp_ack = (struct tftphdr *)skb_put(skb, sizeof(struct tftphdr));
	tftp_ack->th_opcode = htons(ACK);
	tftp_ack->th_block = htons(block);

	udp_send(skb, client_ip, TFTP, client_port);

	return 0;
}

int tftp_rcv_wrq(sk_buff *skb)
{
	struct tftphdr *tftp_hdr;
#ifdef BOOT_LZMA_SUPPORT
	/*
	 * frankliao added 20100511
	 * the pointer is a temporary pointer for
	 * getting filename and mode from skb->data
	 */
    unsigned char* pointer;
#endif

	client_ip = ip_get_source_ip(skb);
	client_port = udp_get_source_port(skb);

	//prom_printf("client_ip=%X\n", client_ip);
	//prom_printf("client_port=%X\n", client_port);
#ifdef BOOT_LZMA_SUPPORT
	/*
	 * frankliao added 20100511
	 * skb->data started in opcode (16 bit),
	 * (skb->data + 2) is the address of filename
	 */
	pointer = skb->data + 2;
	/* get filename */
	strncpy(filename, pointer, sizeof (filename) - 1);

	/*
	 * frankliao added 20100511
	 * set filetype as a identifier when checking file
	 */
	if( !strcmp(filename, TCBOOT_NAME) )
		filetype = TCBOOT;
	else if( !strcmp(filename, TCLINUX_NAME) )
		filetype = TCLINUX;
	else
		filetype = ERRFILE;
#endif
	tftp_hdr = (struct tftphdr *)skb->data;
	tftp_send_ack(tftp_hdr, 0);
	client_block = 1;

	tftp_put_begin();

	return 0;
}

int tftp_rcv_data(sk_buff *skb)
{
	struct tftphdr *tftp_hdr;
	int len;

	if (client_ip != ip_get_source_ip(skb))
		return -1;
	if (client_port != udp_get_source_port(skb))
		return -1;

	tftp_hdr = (struct tftphdr *)skb->data;
	if (client_block == ntohs(tftp_hdr->th_block)) {
		len = skb->len - sizeof(struct tftphdr);
		if (tftp_put(tftp_hdr->th_data, len))
			return -1;
#ifdef BOOT_LZMA_SUPPORT
		/* 
		 * frankliao added 20100511
		 * record received data size
		 */
		rcvdata_size += len;
#endif
		tftp_send_ack(tftp_hdr, client_block);
		client_block++;

		if (len < SEGSIZE) {
			tftp_put_end();
#ifdef BOOT_LZMA_SUPPORT
			/*
			 * frankliao added 20100511
			 * check received data by crc
			 */
			if( !checkfile(filetype, rcvdata_size, TFTP_BUF_BASE) )
				tftp_write_flash();
#endif				
        }

	} else if (client_block > ntohs(tftp_hdr->th_block)) {

		tftp_send_ack(tftp_hdr, ntohs(tftp_hdr->th_block));

	} else {

		tftp_send_ack(tftp_hdr, client_block);
	}

	return 0;
}

int tftp_rcv_packet(sk_buff *skb)
{
	struct tftphdr *tftp_hdr;
	
	tftp_hdr = (struct tftphdr *)skb->data;

	switch (ntohs(tftp_hdr->th_opcode)) {

	case RRQ:
		break;
	case WRQ:
		tftp_rcv_wrq(skb);
#ifdef BOOT_LZMA_SUPPORT		
	   /*
		* frankliao added 20100511
		* reset the variable as a counter for 
		* recording the received data size
		*/
		rcvdata_size = 0;
#endif		
		break;
	case DATA:
		tftp_rcv_data(skb);
		break;
	case ACK:
		break;
	case ERROR:
		break;
	default:
		break;
	}

	return 0;
}

#ifdef BOOT_LZMA_SUPPORT
/**************************************************************
 Function name: 
			checkfile
 Description:
			Use checksum to check if received data is valid
 Parameters: 
			filetype : TCBOOT : tcboot.bin
					   TCLINUX : tclinux.bin
			rcvdata_size : the size of checkfile
			start : the start address of checkfile
 Return value: 
			0: success
			others: fail

 frankliao added 20100511
***************************************************************/
int checkfile(short file_type, int rcvdata_size, char *start)
{
	unsigned int checksum;
	unsigned char * crc;
	unsigned int checkdata_size;

	prom_printf("Received file: %s\n", filename);
	prom_printf("rcvdata_size = %X\r\n", rcvdata_size);
	prom_printf("start = 0x%X\r\n", (unsigned long)start);

	

	if (rcvdata_size == 0x10000)
	{
		filetype = TCBOOT;
	}
	else if ((rcvdata_size > 0xD0000) && (rcvdata_size <= 0x160000))
	{
		filetype = TPKERNEL;
	}
	else if ((rcvdata_size > 0x500000) && (rcvdata_size <= 0x700000))
	{
		filetype = TPROOTFS;
	}
	else if (rcvdata_size == 0x800000)
	{
		filetype = TPFLASH;
	}

	prom_printf("filetype : %d\n", filetype);

	file_type = filetype;
	/* check tcboot.bin */
	if (file_type == TCBOOT) {
		if ((rcvdata_size > TCBOOT_SIZE) || (rcvdata_size < 0)) {
			prom_printf("File %s: format error\n", filename);
			return -1;
		}
		crc = (unsigned char*)(start + rcvdata_size - CKSUM_LENGTH);
		checksum = (*crc)<<24 | (*(crc+1))<<16 | (*(crc+2))<<8 | (*(crc+3))<<0;
		checkdata_size = rcvdata_size - CKSUM_LENGTH;
	/* check tclinux.bin */
	} else if (file_type == TCLINUX) {
		if (rcvdata_size <= TCBOOT_SIZE) {
			prom_printf("File %s: format error\n", filename);
			return -1;
		}
		crc = &(((struct trx_header*)start)->crc32);
		checksum = (*crc)<<24 | (*(crc+1))<<16 | (*(crc+2))<<8 | (*(crc+3))<<0;
		checkdata_size = rcvdata_size - sizeof(struct trx_header);
		start += sizeof(struct trx_header);
	/* Add by Zhao Mengqing, 2016/7/25 */
	} else if ((file_type == TPKERNEL) || (file_type == TPROOTFS)
				|| (file_type == TPFLASH) || (file_type == TPRADIO)) {
		//do CRC check ?
	/* End add */
	/* error file handle */
	} else {
		prom_printf("Invalid filename, upload failed\n\n");
		/* Modified by Zhao Mengqing, 2016/7/25 */
		return -1;
	}

	/* Add by Zhao Mengqing, 2016/7/25 */
	if ((file_type == TPKERNEL) || (file_type == TPROOTFS) 
			|| (file_type == TPLINUX) || (file_type == TPRADIO) || (file_type == TPFLASH))
	{
		prom_printf("TP kernel/rootfs/linux/radio, do not check CRC\n...\n");
		return 0;
	}
	/* End add */
	/* frankliao modify 20100805 */
	prom_printf("Real crc code: %X\n", checksum);
	if (checksum == crc32buf(start, checkdata_size, 1)) {
		
		prom_printf("Check data success, prepare to upload\n...\n");
		return 0;
	} else {
		prom_printf("Check data fail, upload failed\n...\n");
		return -1;
	}

}


/********************************************
 Function name: 
			tftp_write_flash
 Description: 
			write received data to suitable address in flash
 Parameters: 
			None
 Return value:    
			0: success
	   others: fail

 frankliao added 20100511
 *******************************************/

static int tftp_write_flash(void)
{
	unsigned long retlen;

	
	/* move tcboot.bin to the base address of bootloader */
	if (filetype == TCBOOT) {
		my_flash_boot();
	/* move tclinux.bin to the base address of kernel */
	} else if (filetype == TPKERNEL) {
		my_flash_kernel();
	} else if (filetype == TPROOTFS) {
		my_flash_rootfs();	
	} else if (filetype == TPFLASH) {
		my_flash_image();
	} else {
		return -1;
	}

	return 0;
}
#endif
unsigned long tftp_get_buf_base()
{
	return TFTP_BUF_BASE;
}
