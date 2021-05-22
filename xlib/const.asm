; file descriptors

#define STDIN       #0x00
#define STDOUT      #0x01
#define STDERR      #0x02

; syscalls
; printf syscall removed

#define SYS_READ    #0x00
#define SYS_WRITE   #0x01
#define SYS_MAP     #0x02
#define SYS_UNMAP   #0x03
#define SYS_EXEC    #0x04
#define SYS_OPEN    #0x05
#define SYS_CLOSE   #0x06
#define SYS_BIND    #0x07
#define SYS_ACCEPT  #0x08
#define SYS_LISTEN  #0x09
#define SYS_RECV    #0x0A
#define SYS_SEND    #0x0B
#define SYS_SOCKET  #0x0C
#define SYS_CONNECT #0x0D
#define SYS_DUP2    #0x0E

; perms
#define PERM_READ   #0x1
#define PERM_WRITE  #0x2
#define PERM_EXEC   #0x4
#define PERM_RW     #0x3
#define PERM_RX     #0x5
#define PERM_RWX    #0x7

; flags
#define O_CREAT     #0x40
#define O_EXCL      #0x80
#define O_TRUNC     #0x200
#define O_APPEND    #0x400
#define O_RDONLY    #0x0
#define O_WRONLY    #0x1
#define O_RDWR      #0x2
#define SEEK_SET    #0x00
#define SEEK_CUR    #0x01
#define SEEK_END    #0x02

; modes
#define S_WORKS     #0x1a4
#define S_IRWXU     #0x1c0
#define S_IRUSR     #0x100
#define S_IWUSR     #0x80
#define S_IXUSR     #0x40
#define S_IRWXG     #0x38
#define S_IRGRP     #0x20
#define S_IWGRP     #0x10
#define S_IXGRP     #0x8
#define S_IRWXO     #0x7
#define S_IROTH     #0x4
#define S_IWOTH     #0x2
#define S_IXOTH     #0x1


; Sockets

#define SOCK_STREAM     #0x01
#define SOCK_DGRAM      #0x02

#define AF_UNSPEC       #0x0
#define AF_UNIX         #0x1
#define AF_INET         #0x2
#define AF_AX25         #0x3
#define AF_IPX          #0x4
#define AF_APPLETALK    #0x5
#define AF_NETROM       #0x6
#define AF_BRIDGE       #0x7
#define AF_AAL5         #0x8
#define AF_X25          #0x9
#define AF_INET6        #0xa
#define AF_MAX          #0xc

; RAND CONSTS
#define PRNG_A          #0x4212
#define PRNG_B          #0x9837
#define PRNG_C          #0x117


.section .data
PRVS_N:
    .dd #0xCCCCCCCC