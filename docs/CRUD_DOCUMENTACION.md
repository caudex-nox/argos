# Documentación Técnica: Implementación de CRUD con C++17, Qt 6 y SQLite 3

**Proyecto:** Argos | CRUD  
**Organización:** Caudex  
**Materia:** Programación II  
**Carrera:** Tecnicatura en Desarrollo de Software  
**Año:** 2026  
**Tecnologías:** C++17, Qt 6 (Widgets), SQLite 3 (C API), CMake  

---

## 📑 Tabla de Contenidos

1. [Visión General y Arquitectura](#1-visión-general-y-arquitectura)
2. [Estructura del Proyecto](#2-estructura-del-proyecto)
3. [Estructura de la Base de Datos](#3-estructura-de-la-base-de-datos)
4. [Ciclo de Vida de Consultas en SQLite 3](#4-ciclo-de-vida-de-consultas-en-sqlite-3)
5. [Operaciones CRUD](#5-operaciones-crud)
   - [5.1 Create (Insertar Registro)](#51-create-insertar-registro)
   - [5.2 Read (Consultar y Monitorear Registros)](#52-read-consultar-y-monitorear-registros)
   - [5.3 Update (Actualizar Registro)](#53-update-actualizar-registro)
   - [5.4 Delete (Eliminar Registro)](#54-delete-eliminar-registro)
6. [Cifrado y Seguridad de Datos Sensibles](#6-cifrado-y-seguridad-de-datos-sensibles)
7. [Mapeo de Señales y Slots (Qt 6 UI ↔ Lógica)](#7-mapeo-de-señales-y-slots-qt-6-ui--lógica)
8. [Comparativa Técnica: SQLite 3 C API vs QtSql](#8-comparativa-técnica-sqlite-3-c-api-vs-qtsql)
9. [Compilación y Ejecución](#9-compilación-y-ejecución)
   - [9.1 Requisitos Previos e Instalación](#91-requisitos-previos-e-instalación)
   - [9.2 Pasos de Compilación con CMake](#92-pasos-de-compilación-con-cmake)
10. [Escalabilidad y Líneas de Evolución Futura](#10-escalabilidad-y-líneas-de-evolución-futura)

---

## 1. Visión General y Arquitectura

**Argos** es una aplicación de escritorio multiplataforma desarrollada en **C++17**, **Qt 6 (Widgets)** y **SQLite 3**, orientada a la administración de usuarios y al monitoreo de tareas en tiempo real. Permite realizar operaciones CRUD completas, gestionar estados de actividades y proteger información sensible mediante cifrado local, todo desde una interfaz gráfica clara y moderna.

Este proyecto fue desarrollado como trabajo final para la materia **Programación II** de la carrera **Tecnicatura en Desarrollo de Software** por la organización **Caudex**.

### Diagrama de Flujo de Datos

```
┌──────────────────────────────────────────────────────────────┐
│                    Capa de Presentación                      │
│     (Qt 6 Widgets: QLineEdit, QComboBox, QTableWidget)       │
└──────────────────────────────┬───────────────────────────────┘
                               │ Eventos de usuario (Signals)
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                    Capa de Control y Lógica                  │
│   (MainWindow: validación, cifrado XOR y binding de datos)   │
└──────────────────────────────┬───────────────────────────────┘
                               │ Prepared Statements (C API)
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                    Capa de Persistencia                      │
│      (SQLite 3 Engine: Base local usuarios_tareas.db)        │
└──────────────────────────────────────────────────────────────┘
```

---

## 2. Estructura del Proyecto

La organización del repositorio separa claramente el código fuente, la documentación técnica, los recursos gráficos del sistema y la configuración de construcción con CMake:

```text
Argos/
├── assets/                 # Recursos gráficos
│   └── images/             # Logotipos e imágenes para la aplicación y documentación
├── docs/                   # Documentación técnica extendida
│   └── CRUD_DOCUMENTACION.md # Guía detallada de arquitectura y ciclo de vida SQLite 3
├── resources/              # Iconos y recursos para compilación Qt y Windows
│   ├── app.qrc             # Archivo de recursos Qt embebidos
│   ├── app.rc              # Metadatos e icono de aplicación para Windows
│   ├── argos-icon.ico      # Icono en formato .ico para Windows
│   └── argos-icon.png      # Icono en formato .png para ventanas Linux/Qt
├── src/                    # Código fuente de la aplicación
│   ├── MainWindow.h        # Definición de la clase MainWindow, campos y slots
│   ├── MainWindow.cpp      # Lógica de interfaz, operaciones CRUD y acceso a SQLite
│   ├── main.cpp            # Punto de entrada de la aplicación
│   ├── sqlite3.c           # Implementación C incrustada del motor SQLite
│   └── sqlite3.h           # Cabecera de la C API de SQLite
├── .gitignore              # Archivos y artefactos de compilación excluidos de Git
├── CMakeLists.txt          # Configuración de compilación con CMake
└── README.md               # Documentación general del proyecto
```

---

## 3. Estructura de la Base de Datos

La base de datos se inicializa en el método `MainWindow::initDatabase()` y se almacena en el directorio local de datos de usuario provisto por el sistema mediante `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)`. El archivo de persistencia generado es `usuarios_tareas.db`.

### Esquema de la Tabla e Índices

```sql
CREATE TABLE IF NOT EXISTS usuarios (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nombre TEXT NOT NULL,
    email TEXT NOT NULL,
    edad INTEGER,
    tarea TEXT,
    estado TEXT DEFAULT 'Pendiente'
);

CREATE INDEX IF NOT EXISTS idx_usuarios_estado ON usuarios(estado);
```

### Diccionario de Datos (`usuarios`)

| Campo | Tipo SQL | Descripción | Restricciones |
| :--- | :--- | :--- | :--- |
| `id` | `INTEGER` | Identificador único autoincremental | Clave primaria (`PRIMARY KEY AUTOINCREMENT`) |
| `nombre` | `TEXT` | Nombre del usuario/empleado | `NOT NULL` |
| `email` | `TEXT` | Correo electrónico almacenado en formato cifrado (XOR + Base64) | `NOT NULL` |
| `edad` | `INTEGER` | Edad del usuario | Rango validado (18-60), ingreso manual requerido |
| `tarea` | `TEXT` | Descripción de la actividad asignada | Opcional |
| `estado` | `TEXT` | Estado de progreso de la tarea asignada | Por defecto `'Pendiente'` (`'Pendiente'`, `'En Proceso'`, `'Finalizado'`) |

> **Optimización de Índices:** El índice `idx_usuarios_estado` acelera las consultas de lectura y monitoreo en tiempo real cuando el usuario filtra por el estado de las tareas (`WHERE estado = ?`).

---

## 4. Ciclo de Vida de Consultas en SQLite 3

Para garantizar el máximo rendimiento y prevenir vulnerabilidades de **Inyección SQL**, todas las consultas que reciben parámetros del usuario se gestionan mediante **sentencias preparadas** (*Prepared Statements*) de la API nativa de C de SQLite 3:

```
[SQL String con '?'] ──► sqlite3_prepare_v2() ──► sqlite3_stmt* compilado
                                                          │
                                                          ▼
                                                  sqlite3_bind_*()
                                                          │
                                                          ▼
                                                   sqlite3_step()
                                                    /         \
                                     (SQLITE_DONE) /           \ (SQLITE_ROW)
                                                  ▼             ▼
                                         Escritura OK     Leer columnas (SELECT)
                                                  \             /
                                                   ▼           ▼
                                                 sqlite3_finalize()
```

1. **`sqlite3_prepare_v2()`**: Compila la consulta SQL en código binario (bytecode) y genera un puntero opaco `sqlite3_stmt*`.
2. **`sqlite3_bind_*()`**: Vincula de forma segura los valores de las variables de C++/Qt a los comodines posicionales `?` de la consulta.
3. **`sqlite3_step()`**:
   - `SQLITE_DONE`: Indica que la operación de escritura (`INSERT`, `UPDATE`, `DELETE`) se ejecutó exitosamente.
   - `SQLITE_ROW`: Indica que hay una fila disponible para lectura (`SELECT`), permitiendo extraer columnas mediante `sqlite3_column_*()`.
4. **`sqlite3_finalize()`**: Destruye la sentencia preparada y libera de inmediato los recursos de memoria asignados.

---

## 5. Operaciones CRUD

Para evitar duplicación de código en la preparación de parámetros, el enlace de atributos compartidos entre inserciones y actualizaciones se centralizó en la función auxiliar estática `MainWindow::bindCamposUsuario`:

```cpp
void MainWindow::bindCamposUsuario(sqlite3_stmt* stmt, const QString& nombre, const QString& emailCifrado, int edad, const QString& tarea, const QString& estado) {
    sqlite3_bind_text(stmt, 1, nombre.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, emailCifrado.toLatin1().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, edad);
    sqlite3_bind_text(stmt, 4, tarea.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, estado.toUtf8().constData(), -1, SQLITE_TRANSIENT);
}
```

> **Nota Técnica:** La constante `SQLITE_TRANSIENT` le indica a SQLite que realice una copia interna del buffer en memoria antes de que los objetos temporales de Qt salgan de ámbito.

---

### 5.1 Create (Insertar Registro)

* **Método:** `MainWindow::handleInsertar()`
* **Disparador:** Señal `clicked()` de `btnInsertar`.

#### Flujo paso a paso:
<<<<<<< Updated upstream
1. **Validación:** Se comprueba que `nombre` y `email` contengan texto válido.
2. **Cifrado:** El correo se cifra mediante la función `cifrar()`.
3. **Preparación y Enlace de Parámetros:**
=======
1. **Validación de Campos Requeridos:** Se comprueba que `nombre` y `email` no estén vacíos.
2. **Validación de Formato de Nombre:** Se restringe la entrada tanto en tiempo real mediante `QRegularExpressionValidator` como al enviar el formulario mediante la expresión regular `QRegularExpression(R"(^[\p{L}]+(?:\s+[\p{L}]+)*$)")`. Esto garantiza que el nombre únicamente contenga caracteres alfabéticos (incluyendo tildes, diéresis y la letra ñ) y espacios individuales, prohibiendo números o caracteres especiales.
3. **Validación de Formato de Email:** Se utiliza una expresión regular estricta (`QRegularExpression regex(R"(^[^@\s]+@[^@\s]+\.[^@\s]{2,}$)")`) para verificar que el correo cuente con estructura de usuario, dominio y extensión válidos.
4. **Cifrado del Email:** Se transforma el correo en texto cifrado seguro mediante `cifrar(email)`.
5. **Preparación y Enlace:**
>>>>>>> Stashed changes
   ```cpp
   const char* sql = "INSERT INTO usuarios (nombre, email, edad, tarea, estado) VALUES (?, ?, ?, ?, ?);";
   sqlite3_stmt* stmt;

   if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
       QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
       return;
   }

   bindCamposUsuario(stmt, nombre, emailCifrado, edad, tarea, estado);
   ```
5. **Ejecución y Limpieza:** Al retornar `SQLITE_DONE`, se vacían los campos del formulario (`handleLimpiar()`) y se recarga la tabla de datos (`cargarTabla()`).

---

### 5.2 Read (Consultar y Monitorear Registros)

* **Método:** `MainWindow::cargarTabla(const QString& filtroEstado)`
* **Disparadores:** Inicialización del programa, tras cambios en la BD, o al cambiar el valor del combo de filtro `comboFiltroEstado`.

#### Flujo paso a paso:
1. **Optimización de Renderizado:** Se invoca `tablaUsuarios->setUpdatesEnabled(false)` para deshabilitar temporalmente el repintado de la interfaz y evitar parpadeos durante la carga masiva.
2. **Construcción del Query Dinámico:**
   ```cpp
   QString query = "SELECT id, nombre, email, edad, tarea, estado FROM usuarios";
   if (filtroEstado != "Todos") {
       query += " WHERE estado = ?";
   }
   query += " ORDER BY id DESC;";
   ```
3. **Recorrido del Cursor y Descifrado:**
   ```cpp
   int row = 0;
   while (sqlite3_step(stmt) == SQLITE_ROW) {
       tablaUsuarios->insertRow(row);

       int id = sqlite3_column_int(stmt, 0);
       QString nombre = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
       QString emailCifrado = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
       int edad = sqlite3_column_int(stmt, 3);
       const char* tareaTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
       const char* estadoTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

       // Descifrado para presentación en la vista
       QString emailClaro = descifrar(emailCifrado);

       tablaUsuarios->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
       tablaUsuarios->setItem(row, 1, new QTableWidgetItem(nombre));
       tablaUsuarios->setItem(row, 2, new QTableWidgetItem(emailClaro));
       tablaUsuarios->setItem(row, 3, new QTableWidgetItem(QString::number(edad)));
       tablaUsuarios->setItem(row, 4, new QTableWidgetItem(tareaTxt ? QString::fromUtf8(tareaTxt) : ""));
       tablaUsuarios->setItem(row, 5, new QTableWidgetItem(estadoTxt ? QString::fromUtf8(estadoTxt) : "Pendiente"));

       row++;
   }
   sqlite3_finalize(stmt);
   tablaUsuarios->setUpdatesEnabled(true);
   ```

---

### 5.3 Update (Actualizar Registro)

* **Método:** `MainWindow::handleActualizar()`
* **Disparador:** Señal `clicked()` de `btnActualizar`.

#### Flujo paso a paso:
<<<<<<< Updated upstream
1. **Comprobación de ID:** Requiere que el campo de solo lectura `txtId` tenga un ID cargado (seleccionado previamente de la tabla).
2. **Consulta Parametrizada:**
=======
1. **Comprobación de Selección:** Requiere que exista un ID cargado en memoria (`txtId`) tras seleccionar una fila de la tabla.
2. **Validación de Datos:** Se aplican los mismos filtros que en la inserción: presencia de campos obligatorios, validación de caracteres del nombre (solo letras y espacios individuales) y validez del correo electrónico.
3. **Consulta Parametrizada:**
>>>>>>> Stashed changes
   ```sql
   UPDATE usuarios SET nombre = ?, email = ?, edad = ?, tarea = ?, estado = ? WHERE id = ?;
   ```
3. **Binding y Ejecución:**
   ```cpp
   bindCamposUsuario(stmt, nombre, emailCifrado, edad, tarea, estado);
   sqlite3_bind_int(stmt, 6, id);

   if (sqlite3_step(stmt) == SQLITE_DONE) {
       handleLimpiar();
       cargarTabla(comboFiltroEstado->currentText());
   }
   sqlite3_finalize(stmt);
   ```

---

### 5.4 Delete (Eliminar Registro)

* **Método:** `MainWindow::handleEliminar()`
* **Disparador:** Señal `clicked()` de `btnEliminar`.

#### Flujo paso a paso:
1. **Validación:** Comprueba la existencia de un ID seleccionado en `txtId`.
2. **Ejecución del Borrado:**
   ```cpp
   const char* sql = "DELETE FROM usuarios WHERE id = ?;";
   sqlite3_stmt* stmt;
   sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
   sqlite3_bind_int(stmt, 1, id);

   if (sqlite3_step(stmt) == SQLITE_DONE) {
       handleLimpiar();
       cargarTabla(comboFiltroEstado->currentText());
   }
   sqlite3_finalize(stmt);
   ```

---

## 6. Cifrado y Seguridad de Datos Sensibles

Para mitigar el riesgo de filtraciones al inspeccionar directamente el archivo físico `.db` en disco, la aplicación implementa una capa de protección criptográfica simétrica para datos sensibles (como la columna `email`):

```cpp
const quint8 CLAVE_ENCRIPTACION = 0x6C;

QString MainWindow::cifrar(const QString& texto) {
    QByteArray bytes = texto.toUtf8();
    for (int i = 0; i < bytes.size(); ++i) {
        bytes[i] = bytes[i] ^ CLAVE_ENCRIPTACION;
    }
    return QString::fromLatin1(bytes.toBase64());
}

QString MainWindow::descifrar(const QString& texto) {
    QByteArray bytes = QByteArray::fromBase64(texto.toLatin1());
    for (int i = 0; i < bytes.size(); ++i) {
        bytes[i] = bytes[i] ^ CLAVE_ENCRIPTACION;
    }
    return QString::fromUtf8(bytes);
}
```

* **Operación XOR:** Modifica bit a bit cada byte del texto original mediante una clave simétrica (`0x6C`).
* **Codificación Base64:** Convierte los bytes resultantes en una cadena de caracteres ASCII seguros, asegurando que se guarden sin problemas de codificación o caracteres no imprimibles en SQLite.

---

## 7. Mapeo de Señales y Slots (Qt 6 UI ↔ Lógica)

En Qt 6, los eventos de la interfaz gráfica se comunican con el código C++ mediante el mecanismo tipado de **Signals & Slots**:

```cpp
connect(btnInsertar, &QPushButton::clicked, this, &MainWindow::handleInsertar);
connect(btnActualizar, &QPushButton::clicked, this, &MainWindow::handleActualizar);
connect(btnEliminar, &QPushButton::clicked, this, &MainWindow::handleEliminar);
connect(btnLimpiar, &QPushButton::clicked, this, &MainWindow::handleLimpiar);
connect(tablaUsuarios, &QTableWidget::cellClicked, this, &MainWindow::handleRowSelected);
connect(comboFiltroEstado, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::handleFiltrarPorEstado);
```

<<<<<<< Updated upstream
* **Selección de Fila:** Al hacer clic en cualquier celda de `QTableWidget`, el slot `handleRowSelected(int row, int col)` rellena automáticamente los campos del formulario con los valores de la fila elegida para facilitar la edición o eliminación.
=======
* **Carga de Registro al Formulario:** Al hacer clic en cualquier celda de `tablaUsuarios`, el slot `handleRowSelected(int row, int col)` extrae los datos de la fila y los coloca en los campos del formulario (`txtNombre`, `txtEmailSensible`, `spinEdad`, `txtTarea`, `comboEstado`) y almacena internamente el ID en `txtId` para habilitar operaciones de actualización o eliminación.
* **Ocultación de la Clave Primaria:** Por motivos de diseño y seguridad de la información, el ID autoincremental (`PRIMARY KEY`) no es visible en la interfaz: la primera columna de `tablaUsuarios` se mantiene oculta mediante `tablaUsuarios->setColumnHidden(0, true)` y el campo `txtId` no se incluye en el formulario visual.
>>>>>>> Stashed changes

---

## 8. Comparativa Técnica: SQLite 3 C API vs QtSql

| Característica | API nativa de C (`sqlite3.h`) *(Utilizada en este proyecto)* | Módulo `Qt6::Sql` (`QSqlDatabase`, `QSqlQuery`) |
| :--- | :--- | :--- |
| **Dependencias** | Ninguna externa (código fuente embebido `sqlite3.c` / `sqlite3.h`) | Requiere el módulo `Qt6::Sql` |
| **Control de bajo nivel** | Máximo control sobre punteros, errores, sentencias y memoria | Mayor abstracción orientada a objetos en C++ |
| **Modelos de Vistas** | Población manual (`QTableWidget` celda a celda) | Compatible directamente con Model/View (`QSqlTableModel` + `QTableView`) |
| **Curva de Aprendizaje** | Fundamental para comprender el funcionamiento interno de motores SQL | Sintaxis más integrada con `QString` y clases Qt |

---

## 9. Compilación y Ejecución

### 9.1 Requisitos Previos e Instalación

> [!NOTE]
> SQLite 3 se incluye directamente como código fuente embebido en el proyecto (`src/sqlite3.c` y `src/sqlite3.h`), por lo que **no es necesario** instalar paquetes de desarrollo de SQLite en el sistema operativo.

#### 🐧 En Linux

##### Debian / Ubuntu / Linux Mint
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev
```

##### Fedora / RHEL / CentOS
```bash
sudo dnf install -y @development-tools cmake qt6-qtbase-devel
```

##### Arch Linux / Manjaro
```bash
sudo pacman -S --needed base-devel cmake qt6-base
```

---

#### 🪟 En Windows

##### Opción A: Con MSYS2 (Entorno UCRT64 recomendado)
Abrir la terminal **MSYS2 UCRT64** e instalar las herramientas necesarias (compilador, CMake y Qt 6):
```bash
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-qt6-base
```

##### Opción B: Con Qt Online Installer y MinGW
1. Instalar **Qt 6 (con MinGW 64-bit)** y **CMake** desde sus instaladores oficiales.
2. Indicar la ruta de Qt a CMake con `-DCMAKE_PREFIX_PATH`:
   ```powershell
   cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
   ```

---

### 9.2 Pasos de Compilación con CMake

#### 1. Clonar el repositorio

```bash
git clone https://github.com/caudex-nox/argos.git
cd argos
```

#### 2. Compilar y ejecutar

##### **En Linux:**
```bash
mkdir build && cd build
cmake ..
make
./Argos-CRUD
```

##### **En Windows (PowerShell / CMD con MinGW):**
```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
.\Argos-CRUD.exe
```

---

## 10. Escalabilidad y Líneas de Evolución Futura

Como parte del diseño conceptual inicial y como posibles líneas de evolución para continuar escalando el proyecto más allá del alcance académico actual, se contemplan las siguientes propuestas arquitectónicas:

* **🏢 Gestión de Organizaciones y Multi-empresa:** Creación de estructuras empresariales independientes y unificación de miembros mediante tokens de invitación únicos.
* **👑 Control Jerárquico de Roles y Permisos:** Sistema granular de privilegios (administradores, supervisores, empleados) para restringir el acceso a módulos y operaciones específicas.
* **🔐 Módulo de Autenticación y Cuentas de Usuario:** Flujo de inicio de sesión seguro (`LoginWindow`), gestión de sesiones y paneles de control dedicados (`DashboardWindow`).
* **🛡️ Cifrado Avanzado con SQLCipher:** Migración del cifrado XOR a nivel de campo hacia una protección criptográfica robusta a nivel de archivo completo utilizando el estándar AES-256 de SQLCipher.
* **🧩 Arquitectura Modular (MVC):** Desacoplar la lógica en controladores dedicados (`controllers/`), modelos de dominio (`models/`), vistas UI (`.ui`) y un administrador centralizado de persistencia (`DatabaseManager`).
