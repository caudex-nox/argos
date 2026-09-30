# Documentación Técnica: Implementación de CRUD con C++17, Qt 6 y SQLite3

**Proyecto:** Argos / CrudQtSqlite  
**Materia:** Programación II  
**Tecnologías:** C++17, Qt 6 (Widgets), SQLite3 (C API), CMake  

---

## 📑 Tabla de Contenidos

1. [Visión General y Arquitectura](#1-visión-general-y-arquitectura)
2. [Estructura de la Base de Datos](#2-estructura-de-la-base-de-datos)
3. [Ciclo de Vida de Consultas en SQLite3](#3-ciclo-de-vida-de-consultas-en-sqlite3)
4. [Operaciones CRUD](#4-operaciones-crud)
   - [Create (Insertar Registro)](#41-create-insertar-registro)
   - [Read (Consultar y Monitorear Registros)](#42-read-consultar-y-monitorear-registros)
   - [Update (Actualizar Registro)](#43-update-actualizar-registro)
   - [Delete (Eliminar Registro)](#44-delete-eliminar-registro)
5. [Cifrado y Seguridad de Datos Sensibles](#5-cifrado-y-seguridad-de-datos-sensibles)
6. [Mapeo de Señales y Slots (Qt 6 UI ↔ Lógica)](#6-mapeo-de-señales-y-slots-qt-6-ui--lógica)
7. [Comparativa Técnica: SQLite3 C API vs QtSql](#7-comparativa-técnica-sqlite3-c-api-vs-qtsql)
8. [Compilación y Ejecución](#8-compilación-y-ejecución)

---

## 1. Visión General y Arquitectura

El proyecto implementa un sistema de gestión y monitoreo de tareas para usuarios mediante una interfaz gráfica de escritorio en **Qt 6 Widgets**, comunicándose de forma nativa con una base de datos relacional local **SQLite 3**.

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
│         (SQLite3 Engine: Base de datos local .db)            │
└──────────────────────────────────────────────────────────────┘
```

---

## 2. Estructura de la Base de Datos

La base de datos se inicializa en `MainWindow::initDatabase()` y se almacena en el directorio de datos de la aplicación obtenido mediante `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)`.

### Esquema de la Tabla `usuarios`

```sql
CREATE TABLE IF NOT EXISTS usuarios (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nombre TEXT NOT NULL,
    email TEXT NOT NULL,
    edad INTEGER,
    tarea TEXT,
    estado TEXT DEFAULT 'Pendiente'
);
```

| Campo | Tipo SQL | Descripción | Restricciones |
| :--- | :--- | :--- | :--- |
| `id` | `INTEGER` | Identificador único autoincremental | Clave primaria |
| `nombre` | `TEXT` | Nombre del usuario/empleado | NOT NULL |
| `email` | `TEXT` | Correo electrónico almacenado en formato cifrado | NOT NULL |
| `edad` | `INTEGER` | Edad del usuario | Rango configurable (1-120) |
| `tarea` | `TEXT` | Descripción de la actividad asignada | Opcional |
| `estado` | `TEXT` | Estado de progreso de la tarea | `'Pendiente'`, `'En Proceso'`, `'Finalizado'` |

---

## 3. Ciclo de Vida de Consultas en SQLite3

Para garantizar el rendimiento y prevenir vulnerabilidades de **Inyección SQL**, todas las consultas que reciben parámetros del usuario se gestionan mediante **sentencias preparadas** (*Prepared Statements*):

1. **`sqlite3_prepare_v2()`**: Compila la consulta SQL en código binario (bytecode) y genera un puntero `sqlite3_stmt*`.
2. **`sqlite3_bind_*()`**: Vincula los valores de las variables de C++/Qt a los comodines `?` de la consulta.
3. **`sqlite3_step()`**: 
   - `SQLITE_DONE`: Indica que la operación de escritura (INSERT, UPDATE, DELETE) se ejecutó exitosamente.
   - `SQLITE_ROW`: Indica que hay una fila disponible para lectura (SELECT).
4. **`sqlite3_finalize()`**: Destruye el objeto de la sentencia y libera la memoria asignada.

---

## 4. Operaciones CRUD

### 4.1 Create (Insertar Registro)

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

   sqlite3_bind_text(stmt, 1, nombre.toUtf8().constData(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 2, emailCifrado.toLatin1().constData(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_int(stmt, 3, edad);
   sqlite3_bind_text(stmt, 4, tarea.toUtf8().constData(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 5, estado.toUtf8().constData(), -1, SQLITE_TRANSIENT);
   ```
   > **Nota:** La constante `SQLITE_TRANSIENT` le indica a SQLite que debe hacer una copia interna del buffer antes de que los objetos temporales de Qt salgan de ámbito.
4. **Ejecución y Actualización:** Al completarse (`SQLITE_DONE`), se reinicia el formulario y se vuelve a leer la tabla.

---

### 4.2 Read (Consultar y Monitorear Registros)

* **Método:** `MainWindow::cargarTabla(const QString& filtroEstado)`
* **Disparadores:** Inicialización del programa, tras cambios en la BD, o al cambiar el valor del combo de filtro `comboFiltroEstado`.

#### Flujo paso a paso:
1. **Construcción del Query Dinámico:**
   ```cpp
   std::string query = "SELECT id, nombre, email, edad, tarea, estado FROM usuarios";
   if (filtroEstado != "Todos") {
       query += " WHERE estado = ?";
   }
   query += " ORDER BY id DESC;";
   ```
2. **Recorrido del Cursor:**
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

       // Descifrado para presentación en pantalla
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
   ```

---

### 4.3 Update (Actualizar Registro)

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
3. **Binding y Ejecución:** Se enlazan los campos modificados y el identificador en la posición ordinal 6 (`sqlite3_bind_int(stmt, 6, id)`).

---

### 4.4 Delete (Eliminar Registro)

* **Método:** `MainWindow::handleEliminar()`
* **Disparador:** Señal `clicked()` de `btnEliminar`.

#### Flujo paso a paso:
1. **Validación:** Comprueba la existencia de un ID seleccionado.
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

## 5. Cifrado y Seguridad de Datos Sensibles

Para mitigar el riesgo de filtraciones al inspeccionar el archivo físico `.db`, la aplicación implementa una capa de ofuscación/cifrado simétrico:

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

* **Operación XOR:** Modifica bit a bit los bytes del texto original.
* **Base64:** Convierte la cadena resultante en caracteres ASCII imprimibles estándar, facilitando su almacenamiento transparente en SQLite.

---

## 6. Mapeo de Señales y Slots (Qt 6 UI ↔ Lógica)

En Qt, los eventos de la interfaz gráfica se comunican con el código C++ mediante el mecanismo de **Signals & Slots**:

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

## 7. Comparativa Técnica: SQLite3 C API vs QtSql

| Característica | API nativa de C (`sqlite3.h`) *(Utilizada en este proyecto)* | Módulo `Qt6::Sql` (`QSqlDatabase`, `QSqlQuery`) |
| :--- | :--- | :--- |
| **Dependencias** | Solo `libsqlite3` / `sqlite3.c` | Requiere el módulo `Qt6::Sql` |
| **Control de bajo nivel** | Máximo control sobre punteros, errores y memoria | Mayor abstracción orientada a objetos en C++ |
| **Modelos de Vistas** | Población manual (`QTableWidget` celda a celda) | Compatible directamente con Model/View (`QSqlTableModel` + `QTableView`) |
| **Curva de Aprendizaje** | Fundamental para entender cómo funcionan los motores SQL internamente | Sintaxis más integrada con `QString` y clases Qt |

---

## 8. Compilación y Ejecución

### 8.1 Instalación de Dependencias

#### 🐧 En Linux (Debian / Ubuntu / Linux Mint)
El paquete clave de compilación es `build-essential` (que incluye `gcc`, `g++` y `make`). Además, se necesitan los encabezados de desarrollo de Qt 6 y SQLite 3:
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev libsqlite3-dev
```

#### 🐧 En Linux (Fedora / RHEL)
```bash
sudo dnf install -y @development-tools cmake qt6-qtbase-devel sqlite-devel
```

#### 🐧 En Linux (Arch Linux)
```bash
sudo pacman -S --needed base-devel cmake qt6-base sqlite
```

#### 🪟 En Windows

* **Vía MSYS2 (UCRT64):**
  ```bash
  pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-sqlite3
  ```
* **Vía Qt Online Installer + MinGW:**
  Tener instalado Qt 6.x y MinGW en el sistema, especificando el prefijo si no está en las variables de entorno globales:
  ```powershell
  cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
  ```

---

### 8.2 Pasos de Compilación con CMake

#### En Linux:
```bash
# Crear directorio de compilación
mkdir build && cd build

# Configurar y compilar
cmake ..
make

# Ejecutar la aplicación
./CrudQtSqlite
```

#### En Windows (PowerShell con MinGW):
```powershell
# Crear directorio de compilación
mkdir build
cd build

# Generar Makefiles con MinGW
cmake -G "MinGW Makefiles" ..

# Compilar
mingw32-make

# Ejecutar la aplicación
.\CrudQtSqlite.exe
```
