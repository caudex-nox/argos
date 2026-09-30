<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/images/argos-dark-theme.svg">
    <source media="(prefers-color-scheme: light)" srcset="assets/images/argos-light-theme.svg">
    <img src="assets/images/argos-light-theme.svg" width="200" alt="Argos">
  </picture>
</p>

# Argos | CRUD

**Argos** es una aplicación de escritorio multiplataforma desarrollada en **C++17**, **Qt 6 (Widgets)** y **SQLite 3**, orientada a la administración de usuarios y al monitoreo de tareas en tiempo real. Permite realizar operaciones CRUD completas, gestionar estados de actividades y proteger información sensible mediante cifrado local, todo desde una interfaz gráfica clara y moderna.

Este proyecto fue desarrollado como trabajo final para la materia **Programación II** de la carrera **Tecnicatura en Desarrollo de Software**.

---

## 🚀 Características Actuales

* **Operaciones CRUD Completas:** Alta, baja, modificación y consulta de registros de usuarios y tareas asociadas en una base de datos local SQLite.
* **Monitoreo y Filtrado de Tareas:** Visualización tabular dinámica con filtro en tiempo real por estado (*Pendiente*, *En Proceso*, *Finalizado* y *Todos*).
* **Seguridad y Cifrado Local:** Protección de datos sensibles (campo de correo electrónico) mediante cifrado simétrico por operación XOR y codificación Base64 en disco.
* **Consultas Parametrizadas Seguras:** Comunicación nativa mediante la C API de SQLite 3 con sentencias preparadas (*Prepared Statements*) para optimizar el rendimiento y prevenir inyecciones SQL.
* **Interfaz Gráfica Nativa:** Construida con Qt 6 Widgets (`QLineEdit`, `QSpinBox`, `QComboBox`, `QPushButton`, `QTableWidget`).
* **Multiplataforma:** Diseñado y optimizado para compilar y ejecutar de forma nativa en entornos Linux y Windows.

---

## 🛠️ Tecnologías Utilizadas

* **Lenguaje:** C++17
* **Interfaz Gráfica:** Qt 6 (Módulo `Widgets`)
* **Base de Datos:** SQLite 3 (API nativa de C)
* **Sistema de Construcción:** CMake (v3.16+)

---

## 📂 Estructura del Proyecto

```text
Argos/
<<<<<<< Updated upstream
├── assets/                 # Recursos estáticos
│   ├── images/             # Imágenes y logo para la app y README
│   ├── icons/              # Iconos de la interfaz (.png, .svg)
│   └── styles/             # Hojas de estilo QSS
├── data/                   # Base de datos local (ignorada en el repositorio)
├── src/
│   ├── controllers/        # Lógica de negocio (Autenticación, Tareas, Roles)
│   ├── database/           # Persistencia y gestión de SQLite
│   ├── models/             # Estructuras de datos (Usuario, Empresa, Tarea)
│   ├── ui/                 # Vistas y componentes de la interfaz en Qt
│   └── main.cpp            # Punto de entrada de la aplicación
├── .gitignore              # Archivos e historial excluidos de Git
=======
├── assets/                 # Recursos gráficos
│   └── images/             # Imágenes y logotipos para la aplicación y README
├── docs/                   # Documentación del sistema
│   ├── CRUD_DOCUMENTACION.md # Guía detallada de arquitectura y ciclo de vida SQLite
│   └── MANUAL_DE_USUARIO.md  # Manual de usuario y guía operativa de la aplicación
├── resources/              # Iconos y recursos para compilación Qt y Windows
│   ├── app.qrc             # Archivo de recursos Qt embebidos
│   ├── app.rc              # Metadatos e icono de aplicación para Windows
│   ├── argos-icon.ico      # Icono en formato .ico para Windows
│   └── argos-icon.png      # Icono en formato .png para ventanas Linux/Qt
├── src/                    # Código fuente de la aplicación
│   ├── MainWindow.h        # Definición de la clase MainWindow, campos y slots
│   ├── MainWindow.cpp      # Lógica de interfaz, operaciones CRUD y acceso a SQLite
│   ├── main.cpp            # Punto de entrada de la aplicación
│   ├── sqlite3.c           # Implementación C incrustada de SQLite 3 (amalgamation)
│   └── sqlite3.h           # Cabecera C API de SQLite 3
├── .gitignore              # Archivos y artefactos de compilación excluidos de Git
>>>>>>> Stashed changes
├── CMakeLists.txt          # Configuración de compilación con CMake
└── README.md               # Documentación general del proyecto
```

---

## 🔮 Ideas a Futuro (Escalabilidad del Proyecto)

Como parte del diseño conceptual inicial y como posibles líneas de evolución para continuar escalando el proyecto más allá del alcance de la materia, se contemplan las siguientes propuestas de mejora:

* **🏢 Gestión de Organizaciones y Multi-empresa:** Creación de estructuras empresariales independientes y unificación de miembros mediante tokens de invitación únicos.
* **👑 Control Jerárquico de Roles y Permisos:** Sistema granular de privilegios (administradores, supervisores, empleados) para restringir el acceso a módulos y operaciones específicas.
* **🔐 Módulo de Autenticación y Cuentas de Usuario:** Flujo de inicio de sesión seguro (`LoginWindow`), gestión de sesiones y paneles de control dedicados (`DashboardWindow`).
* **🛡️ Cifrado Avanzado con SQLCipher:** Migración del cifrado XOR a nivel de campo hacia una protección criptográfica robusta a nivel de archivo completo utilizando el estándar AES-256 de SQLCipher.
* **🧩 Arquitectura Modular (MVC):** Desacoplar la lógica en controladores dedicados (`controllers/`), modelos de dominio (`models/`), vistas UI (`.ui`) y un administrador centralizado de persistencia (`DatabaseManager`).

---

## 📋 Requisitos Previos e Instalación de Dependencias

> [!NOTE]
> SQLite 3 se incluye directamente como código fuente embebido en el proyecto (`src/sqlite3.c` y `src/sqlite3.h`), por lo que **no es necesario** instalar paquetes de desarrollo de SQLite en el sistema operativo.

### 🐧 En Linux

#### Debian / Ubuntu / Linux Mint
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev
```

#### Fedora / RHEL / CentOS
```bash
sudo dnf install -y @development-tools cmake qt6-qtbase-devel
```

#### Arch Linux / Manjaro
```bash
sudo pacman -S --needed base-devel cmake qt6-base
```

---

### 🪟 En Windows

#### Opción A: Con MSYS2 (Entorno UCRT64 recomendado)
Abrir la terminal **MSYS2 UCRT64** e instalar el compilador, CMake y Qt 6:
```bash
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-qt6-base
```

#### Opción B: Con Qt Online Installer y MinGW
1. Instalar **Qt 6 (con MinGW 64-bit)** y **CMake** desde sus instaladores oficiales.
2. Indicar la ruta de Qt a CMake con `-DCMAKE_PREFIX_PATH`:
   ```powershell
   cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
   ```

---

## ⚙️ Compilación y Ejecución

### 1. Clonar el repositorio

```bash
git clone https://github.com/caudex-nox/argos.git
cd argos
```

### 2. Compilar con CMake

#### **En Linux:**

```bash
mkdir build && cd build
cmake ..
make
./Argos-CRUD
```

#### **En Windows (PowerShell / CMD con MinGW):**

```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
.\CrudQtSqlite.exe
```

<<<<<<< Updated upstream
=======
---

## 📖 Documentación y Manuales

* 📘 **Manual de Usuario:** Consulta el [MANUAL_DE_USUARIO.md](docs/MANUAL_DE_USUARIO.md) para aprender a utilizar la interfaz, dar de alta registros, filtrar actividades y comprender las validaciones del sistema.
* 🛠️ **Documentación Técnica:** Para conocer en detalle la arquitectura de la base de datos, el ciclo de vida de las consultas SQLite3 con la API C y el mapeo de Señales y Slots en Qt 6, consulta el archivo [CRUD_DOCUMENTACION.md](docs/CRUD_DOCUMENTACION.md).

---

>>>>>>> Stashed changes
## 👥 Créditos y Organización

Proyecto desarrollado por el equipo de **Caudex**.

* **Materia:** Programación II
* **Carrera:** Tecnicatura en Desarrollo de Software
* **Año:** 2026
