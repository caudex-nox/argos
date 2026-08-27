<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/images/argos-dark-theme.svg">
    <source media="(prefers-color-scheme: light)" srcset="assets/images/argos-light-theme.svg">
    <img src="assets/images/argos-light-theme.svg" width="200" alt="Argos">
  </picture>
</p>

**Argos** es una aplicación de escritorio multiplataforma diseñada para la administración, control jerárquico y seguimiento de tareas en organizaciones. Permite gestionar estructuras corporativas, asignar roles con distintos niveles de acceso y monitorear el trabajo de los miembros en tiempo real desde un entorno local seguro.

Este proyecto fue desarrollado como trabajo final para la materia **Programación II** de la carrera **Tecnicatura en Desarrollo de Software**.

---

## 🚀 Características Principales

* **Gestión de Organizaciones:** Creación de empresas y unificación de miembros mediante tokens únicos de invitación.
* **Control Jerárquico de Roles:** Definición de rangos, permisos y estructuras organizativas personalizadas por el administrador.
* **Administración de Tareas:** Asignación, seguimiento de estado (*Pendiente*, *En Proceso*, *Finalizado*) y monitoreo de actividades por usuario.
* **Seguridad Local:** Encriptación de datos sensibles y almacenamiento en base de datos local protegida.
* **Multiplataforma:** Diseñado y optimizado para ejecutar de forma nativa en entornos Linux y Windows.

---

## 🛠️ Tecnologías Utilizadas

* **Lenguaje:** C++17
* **Interfaz Gráfica:** Qt 6 (Módulos `Core`, `Widgets` y `Sql`)
* **Base de Datos:** SQLite 3 / SQLCipher (Encriptación AES-256)
* **Sistema de Construcción:** CMake (v3.16+)

---

## 📂 Estructura del Proyecto

```text
Argos/
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
├── CMakeLists.txt          # Configuración de compilación con CMake
└── README.md               # Documentación del proyecto
```

## 📋 Requisitos Previos

* **Compilador C++:** GCC/Clang (Linux) o MinGW/MSVC (Windows) con soporte para C++17.
* **CMake:** Versión 3.16 o superior.
* **Qt 6:** Módulos `Qt6::Core`, `Qt6::Widgets` y `Qt6::Sql`.

## ⚙️ Instalación y Compilación

### 1. Clonar el repositorio

```bash
git clone https://github.com/Caudex/Argos.git
cd Argos
```

### 2. Compilar con CMake

#### **En Linux:**

```bash
mkdir build && cd build
cmake ..
make
./Argos
```

#### **En Windows (PowerShell / CMD con MinGW):**

```dos
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
.\Argos.exe
```

## 👥 Créditos y Organización

Proyecto desarrollado por el equipo de **Caudex**.

* **Materia:** Programación II
* **Carrera:** Tecnicatura en Desarrollo de Software
* **Año:** 2026
