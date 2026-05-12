# 🌱 MODO HEFINSITO – Local Privilege Escalation ROOT - PoC

<img width="5072" height="1536" alt="bannerMF" src="https://github.com/user-attachments/assets/4958e704-80b5-4f7b-94df-9984e0682f98" />

Bienvenido a **Modo Hefinsito**, un script de automatización desarrollado por **UnHackerEnCapital** diseñado para la validación de vectores de ataque de escalada de privilegios local (LPE) en entornos Linux.

Esta herramienta integra una interfaz visual retro con un motor de explotación basado en la vulnerabilidad **CVE-2026-31431**, permitiendo transformar una sesión de usuario estándar en una consola con privilegios de **Root**.

> ⚖️ **Aviso de Auditoría:** Este software se proporciona con fines estrictamente educativos y de investigación en ciberseguridad. El usuario es el único responsable de asegurar que el análisis se realice sobre sistemas donde tenga autorización explícita.

---

## 🚀 Características Principales

* **Auditoría de Privilegios:** Módulo integrado para la detección rápida de UID, GID y pertenencia a grupos críticos (sudo, wheel, root).
* **Explotación de Socket AF_ALG:** Implementa una PoC que manipula la interfaz de algoritmos del kernel para forzar la elevación a través del binario `su`.
* **Control de Flujo por Teclado:** Sistema de validación de términos de uso mediante captura de interrupciones (`ESC` para abortar).

---

## 🔍 ¿Cómo funciona la PoC?

El sistema opera en tres fases críticas de análisis:

1.  **Validación de Entorno:** El script verifica la identidad del usuario actual y analiza si existen permisos administrativos previos o vectores SUID.
2.  **Inyección de Payload:** Utiliza la biblioteca `zlib` para descomprimir la carga útil y la inyecta mediante `os.splice` en el flujo del proceso objetivo.
3.  **Spawn de Shell:** Si la vulnerabilidad es exitosa, el script realiza un bridge directo hacia una shell de root persistente.

---

## ⚙️ Configuración y Uso

### 1️⃣ Requisitos del Sistema
Debido al uso de syscalls específicas como `splice`, este script requiere:
* **Sistema Operativo:** Linux (Kernel vulnerable a la rama 2026-31431).
* **Python:** v3.8 o superior.
* **Permisos:** El script debe ejecutarse con permisos de lectura sobre `/usr/bin/su`.

### 2️⃣ Ejecución
```bash
python3 modo_hefinsito.py
```

---
---

## ⚙️ Compilación y Ejecución del script en C

```bash
gcc -static ModoHefinsito.c -o ModoHefinsito
chmod +x ModoHefinsito
./ModoHefinsito
```

---

> ⚖️ **Creditos de investigacion y desarrollo del script: copy.fail**
