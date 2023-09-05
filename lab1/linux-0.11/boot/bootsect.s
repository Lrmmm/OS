SETUPLEN=2
SETUPSEG=0x07e0
entry _start
_start:
	mov ah,#0x03
	xor bh,bh
	int 0x10
	mov cx,#36
	mov bx,#0x0007
	mov bp,#msg1
	mov ax,#0x07c0
	mov es,ax 
	mov ax,#0x1301
	int 0x10
load_setup:
	! 设置驱动器、磁头、扇区号和磁道
	mov dx,#0x0000
	mov cx,#0x0002
	! 设置读入的内存地址:BOOTSEG+address = 512
	mov bx,#0x0200
	! 设置读入扇区的个数,这里是2 AL:读的扇区数目 AH:0x02表示功能为将磁盘读到内存
	mov ax,#0x0200+SETUPLEN
	int 0x13
	! 如果出错就改变CF寄存器的值,如果不出错跳转到ok_load_setup
	jnc ok_load_setup 
	mov dx,#0x0000
	mov ax,#0x0000
	int 0x13
	jmp load_setup
ok_load_setup:
	jmpi	0,SETUPSEG
msg1:
	.byte	13,10
	.ascii	"Hello OS world, my name is LRM"
	.byte	13,10,13,10
.org 510
boot_flag:
	.word	0xAA55
