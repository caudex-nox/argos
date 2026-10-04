# Manual de Usuario | Argos: Gestor de Tareas y Usuarios

<p align="center">
  <img src="../assets/images/argos-light-theme.svg" width="180" alt="Logo de Argos">
</p>

**Versión:** 1.1  
**Organización:** Caudex  
**Proyecto:** Argos (CRUD con C++17, Qt 6 y SQLite 3)  
**Fecha:** 2026  

---

## 📑 Tabla de Contenidos

1. [Introducción y Propósito](#1-introducción-y-propósito)
2. [Requisitos y Puesta en Marcha](#2-requisitos-y-puesta-en-marcha)
3. [Descripción General de la Interfaz](#3-descripción-general-de-la-interfaz)
4. [Campos y Reglas de Validación](#4-campos-y-reglas-de-validación)
5. [Guía Paso a Paso de Uso](#5-guía-paso-a-paso-de-uso)
   - [5.1 Registrar un Nuevo Usuario y Tarea (Insertar)](#51-registrar-un-nuevo-usuario-y-tarea-insertar)
   - [5.2 Visualizar y Filtrar Registros (Monitoreo)](#52-visualizar-y-filtrar-registros-monitoreo)
   - [5.3 Editar un Registro Existente (Actualizar)](#53-editar-un-registro-existente-actualizar)
   - [5.4 Dar de Baja un Registro (Eliminar)](#54-dar-de-baja-un-registro-eliminar)
   - [5.5 Restablecer el Formulario (Limpiar)](#55-restablecer-el-formulario-limpiar)
6. [Seguridad y Protección de Datos](#6-seguridad-y-protección-de-datos)
7. [Preguntas Frecuentes y Mensajes del Sistema](#7-preguntas-frecuentes-y-mensajes-del-sistema)

---

## 1. Introducción y Propósito

**Argos** es una aplicación de escritorio diseñada para la administración ágil de colaboradores y el seguimiento de sus tareas asignadas en entornos operativos y de desarrollo.

Su objetivo es ofrecer una experiencia intuitiva, confiable y segura para gestionar el ciclo de vida completo de la información (altas, bajas, modificaciones y consultas), protegiendo la privacidad de los datos personales y abstrayendo complejidades técnicas como los identificadores internos de la base de datos.

---

## 2. Requisitos y Puesta en Marcha

### Requisitos del Sistema
- **Sistemas Operativos compatibles:** Windows 10/11 (64-bit) o distribuciones Linux recientes (Ubuntu 22.04+, Debian 12+, Arch Linux, Fedora, etc.).
- **Dependencias de entorno:** Librerías de ejecución de **Qt 6 (módulo Widgets)**.
- **Persistencia local:** No requiere instalar servidores externos de bases de datos. Argos incorpora su propio motor local embebido mediante SQLite 3.

### Ejecución
- **En Windows:** Ejecutar el archivo `Argos-CRUD.exe` directamente o desde la terminal.
- **En Linux:** Ejecutar `./Argos-CRUD` desde el directorio donde fue compilado.

Al abrir la aplicación, la base de datos local `usuarios_tareas.db` se inicializa automáticamente en el almacenamiento seguro de datos de usuario de su sistema.

---

## 3. Descripción General de la Interfaz

La ventana principal de Argos se encuentra dividida en 4 secciones funcionales:

```
┌────────────────────────────────────────────────────────────────────────┐
│                        Argos: Gestor de Tareas                         │
├────────────────────────────────────────────────────────────────────────┤
│ [1] FORMULARIO DE DATOS:                                               │
│     Nombre:                 [ Campo de texto (solo letras)          ]  │
│     Email (Dato Protegido): [ correo@dominio.com                    ]  │
│     Edad:                   [ 18 - 60 ▲▼ ]                             │
│     Tarea Asignada:         [ Ej: Revisión de inventario / Deploy   ]  │
│     Estado de la Tarea:     [ Pendiente ▼ ]                            │
│                                                                        │
│ [2] BOTONERA DE ACCIONES:                                              │
│     [ Insertar ]   [ Actualizar ]   [ Eliminar ]   [ Limpiar ]         │
│                                                                        │
│ [3] MONITOREO Y FILTRO:                                                │
│     Monitoreo por Estado:   [ Todos ▼ ]                                │
│                                                                        │
│ [4] TABLA INTERACTIVA DE REGISTROS:                                    │
│     ┌──────────────────┬─────────────────┬──────┬─────────┬──────────┐ │
│     │ Nombre           │ Email           │ Edad │ Tarea   │ Estado   │ │
│     ├──────────────────┼─────────────────┼──────┼─────────┼──────────┤ │
│     │ Juan Pérez       │ juan@empresa.com│ 28   │ Deploy  │ Finalizado │
│     └──────────────────┴─────────────────┴──────┴─────────┴──────────┘ │
└────────────────────────────────────────────────────────────────────────┘
```

1. **Formulario de Carga:** Permite capturar los datos requeridos para registrar o editar un colaborador y su actividad.
2. **Barra de Acciones:** Contiene los botones de control para interactuar con la base de datos.
3. **Barra de Filtro:** Permite filtrar al instante las tareas visibles en la tabla según su estado de avance.
4. **Tabla de Registros:** Lista todos los registros activos. Permite seleccionar cualquier fila con un clic para cargar sus datos en el formulario.

---

## 4. Campos y Reglas de Validación

Para garantizar la integridad y coherencia de los datos almacenados, Argos implementa controles estrictos en cada campo de entrada:

| Campo | Obligatorio | Regla de Validación y Formato | Comportamiento |
| :--- | :---: | :--- | :--- |
| **Nombre** | **Sí** | **Solo texto alfabético:** Únicamente acepta letras mayúsculas, minúsculas, vocales acentuadas (`á, é, í, ó, ú`), diéresis (`ü`), letra `ñ` y espacios simples entre palabras.<br>*No se permiten números (0-9), símbolos (`@`, `_`, `-`, `#`, etc.) ni signos de puntuación.* | **Bloqueo en tiempo real:** El teclado no permite escribir dígitos ni símbolos dentro del cuadro de texto. Al intentar guardar, se valida que cumpla la estructura correcta. |
| **Email (Dato Protegido)** | **Sí** | Formato de correo electrónico válido estándar: debe contener usuario, símbolo `@`, nombre de servidor y extensión (`ejemplo@dominio.com`). | Si el formato no coincide, se muestra un mensaje explicativo y se detiene la operación. |
| **Edad** | **Sí** | Número entero positivo comprendido entre 18 y 60 años. | Control numérico personalizado (`AgeSpinBox`) con botones de incremento/decremento, inicio vacío para requerir carga obligatoria y validación de rango operativo (18 a 60 años). |
| **Tarea Asignada** | No | Descripción libre del trabajo asignado (ej: *Auditoría de seguridad*, *Deploy a producción*). | Acepta cualquier texto explicativo. |
| **Estado de la Tarea** | **Sí** | Uno de los 3 valores disponibles en la lista desplegable: **`Pendiente`**, **`En Proceso`** o **`Finalizado`**. | Se selecciona desde el menú desplegable. |

> [!NOTE]
> **Identificador Único (Primary Key):**
> La clave primaria es gestionada internamente de manera autoincremental por el motor de base de datos. Por diseño de interfaz limpia y confidencialidad de la arquitectura interna, **el identificador autoincremental no se visualiza en el formulario ni en la tabla**, garantizando un enfoque 100% centrado en la información del negocio.

---

## 5. Guía Paso a Paso de Uso

### 5.1 Registrar un Nuevo Usuario y Tarea (Insertar)

1. En el campo **Nombre**, escriba el nombre y apellido del usuario (ej: `Carlos Mendoza`). *Observe que si intenta presionar teclas numéricas o símbolos, el sistema las ignorará automáticamente*.
2. En el campo **Email (Dato Protegido)**, ingrese una dirección de correo válida (ej: `carlos.mendoza@empresa.com`).
3. En el campo **Edad**, ingrese la edad correspondiente (entre 18 y 60 años) escribiendo el número o mediante las flechas. El campo inicia vacío para requerir su carga manual obligatoria.
4. En **Tarea Asignada**, escriba la descripción de la labor asignada.
5. En **Estado de la Tarea**, elija el estado inicial (`Pendiente`, `En Proceso` o `Finalizado`).
6. Presione el botón **Insertar**.
7. Si los datos son válidos, el formulario se vaciará automáticamente y el nuevo registro aparecerá en la primera posición de la tabla.

---

### 5.2 Visualizar y Filtrar Registros (Monitoreo)

La tabla muestra los registros ordenados desde el más reciente hasta el más antiguo.

Para monitorear el flujo de trabajo:
1. Diríjase a la sección **Monitoreo por Estado**.
2. Despliegue el selector y elija el estado que desea consultar:
   - **`Todos`:** Muestra la totalidad de los usuarios y tareas registradas.
   - **`Pendiente`:** Muestra únicamente aquellas actividades que no han comenzado.
   - **`En Proceso`:** Muestra las tareas que se encuentran en desarrollo activo.
   - **`Finalizado`:** Muestra exclusivamente las actividades completadas.
3. La tabla se actualizará de forma instantánea al cambiar la opción.

---

### 5.3 Editar un Registro Existente (Actualizar)

1. En la tabla inferior, haga **un clic sobre cualquier celda de la fila** que desea modificar.
2. Los datos de dicho registro se trasladarán automáticamente a los campos del formulario superior.
3. Efectúe las modificaciones deseadas (por ejemplo, cambiar el estado de `En Proceso` a `Finalizado`, o actualizar la tarea asignada).
4. Presione el botón **Actualizar**.
5. Los cambios se guardarán en la base de datos de manera inmediata y la tabla reflejará los nuevos valores.

> [!IMPORTANT]
> Si presiona **Actualizar** sin haber seleccionado previamente un registro de la tabla, la aplicación mostrará una advertencia: *"Selecciona primero un registro de la tabla."*

---

### 5.4 Dar de Baja un Registro (Eliminar)

1. Haga clic sobre la fila que desea eliminar de la tabla para seleccionarla.
2. Verifique que los datos cargados en el formulario correspondan al usuario que desea remover.
3. Presione el botón **Eliminar**.
4. El registro se eliminará permanentemente de la base de datos y desaparecerá de la vista tabular.
5. El formulario quedará limpio y listo para una nueva operación.

---

### 5.5 Restablecer el Formulario (Limpiar)

Si cargó un registro en el formulario pero prefiere cancelar la edición o desea preparar los campos para insertar un nuevo usuario desde cero:
1. Presione el botón **Limpiar Formulario**.
2. Se vaciarán los campos de texto, se deseleccionará la fila activa de la tabla y el campo de edad volverá a quedar vacío para una nueva carga manual.

---

## 6. Seguridad y Protección de Datos

Argos incorpora buenas prácticas de ingeniería de software orientadas a la confidencialidad y robustez:

1. **Abstracción de Claves Primarias:**  
   Los identificadores numéricos autoincrementales internos (`id`) no son expuestos en la interfaz de usuario. Las operaciones de actualización y eliminación utilizan el puntero en memoria del elemento seleccionado sin necesidad de mostrar números de registro técnicos al operador.
2. **Cifrado Local de Correo Electrónico:**  
   La dirección de email de cada colaborador es información confidencial. Antes de ser escrita en el archivo de base de datos en disco (`usuarios_tareas.db`), la aplicación aplica un cifrado simétrico mediante una máscara binaria (operación XOR) y codificación Base64. Al consultar la aplicación, el correo es descifrado en memoria únicamente para su presentación en pantalla. Si un tercero no autorizado abre el archivo `.db` con un visor SQLite genérico, el email resultará ilegible.
3. **Prevención de Inyecciones SQL:**  
   Todas las operaciones de base de datos se ejecutan mediante sentencias preparadas (*Prepared Statements* con `sqlite3_prepare_v2` y `sqlite3_bind_*`), impidiendo cualquier ataque de inyección de código malicioso a través de los campos de texto.

---

## 7. Preguntas Frecuentes y Mensajes del Sistema

### ¿Por qué el teclado no me deja escribir números o caracteres especiales en el Nombre?
Argos cuenta con un filtro en tiempo real (`QRegularExpressionValidator`) en el campo Nombre. Para prevenir errores tipográficos y registros anómalos, el sistema solo acepta caracteres alfabéticos (incluyendo vocales con acento, diéresis y la letra Ñ) y espacios simples. Cualquier número o carácter especial es rechazado al momento de presionar la tecla o al pegar texto.

### Mensaje: *"Nombre y Email son requeridos."*
Asegúrese de no haber dejado en blanco el campo de Nombre o el de Email. Ambos datos son obligatorios para dar de alta o modificar a un usuario.

### Mensaje: *"El Nombre solo debe contener letras (sin números ni caracteres especiales)."*
Este mensaje aparece si el nombre contiene caracteres no permitidos o si se dejaron espacios en blanco continuos. Ingrese únicamente letras y espacios sencillos entre palabras.

### Mensaje: *"El formato del email no es válido..."*
Asegúrese de que el correo contenga la estructura tradicional: un nombre de usuario, el signo `@`, un dominio y una extensión de al menos dos letras (por ejemplo: `usuario@correo.com`).

### Mensaje: *"Selecciona primero un registro de la tabla."*
Para utilizar los botones **Actualizar** o **Eliminar**, primero debe hacer clic sobre la fila correspondiente en la tabla interactiva inferior.

---

<p align="center">
  <b>Argos</b> — Desarrollado por <b>Caudex</b> (2026)
</p>
