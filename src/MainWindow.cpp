#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>
#include <QStandardPaths>
#include <QDir>
#include <QByteArray>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

// Máscara binaria (clave de 1 byte) utilizada para la operación criptográfica XOR
const quint8 CLAVE_ENCRIPTACION = 0x6C;

// ============================================================================
// MÉTODOS DE CIFRADO Y SEGURIDAD LOCAL
// ============================================================================

/**
 * @brief Cifra una cadena de texto sensible (como el correo electrónico).
 * 
 * Procedimiento:
 * 1. Convierte el QString a un arreglo de bytes UTF-8.
 * 2. Aplica una operación binaria XOR byte a byte con la clave fija.
 * 3. Codifica el resultado resultante en formato Base64 para que pueda
 *    almacenarse de manera segura y legible como texto en SQLite.
 */
QString MainWindow::cifrar(const QString& texto) {
    QByteArray bytes = texto.toUtf8();
    for (int i = 0; i < bytes.size(); ++i) {
        bytes[i] = bytes[i] ^ CLAVE_ENCRIPTACION; // Operación XOR
    }
    return QString::fromLatin1(bytes.toBase64());
}

/**
 * @brief Descifra una cadena previamente cifrada con el método cifrar().
 * 
 * Procedimiento inverso:
 * 1. Decodifica la cadena desde Base64 a bytes sin procesar.
 * 2. Aplica nuevamente la operación XOR (al ser simétrica, revierte el cifrado).
 * 3. Reconstruye el texto original en formato UTF-8 para presentarlo en pantalla.
 */
QString MainWindow::descifrar(const QString& texto) {
    QByteArray bytes = QByteArray::fromBase64(texto.toLatin1());
    for (int i = 0; i < bytes.size(); ++i) {
        bytes[i] = bytes[i] ^ CLAVE_ENCRIPTACION; // Deshace la operación XOR
    }
    return QString::fromUtf8(bytes);
}

/**
 * @brief Enlaza los parámetros de un usuario a una sentencia SQL preparada de SQLite.
 * 
 * El uso de sentencias preparadas y parámetros posicionales ('?') previene de forma
 * absoluta los ataques de Inyección SQL.
 */
void MainWindow::bindCamposUsuario(sqlite3_stmt* stmt, const QString& nombre, const QString& emailCifrado, int edad, const QString& tarea, const QString& estado) {
    // Parámetro 1: Nombre (Texto UTF-8)
    sqlite3_bind_text(stmt, 1, nombre.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    // Parámetro 2: Correo electrónico cifrado (Base64 ASCII)
    sqlite3_bind_text(stmt, 2, emailCifrado.toLatin1().constData(), -1, SQLITE_TRANSIENT);
    // Parámetro 3: Edad (Entero)
    sqlite3_bind_int(stmt, 3, edad);
    // Parámetro 4: Tarea asignada (Texto UTF-8)
    sqlite3_bind_text(stmt, 4, tarea.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    // Parámetro 5: Estado de la actividad (Texto UTF-8)
    sqlite3_bind_text(stmt, 5, estado.toUtf8().constData(), -1, SQLITE_TRANSIENT);
}

// ============================================================================
// CONSTRUCTOR Y DESTRUCTOR DE LA VENTANA PRINCIPAL
// ============================================================================

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Configuración estética de la ventana
    setWindowTitle("Argos: Gestor de Tareas");
    resize(900, 580);
    setWindowIcon(QIcon(":/resources/argos-icon.png"));

    // Inicializa el archivo SQLite y crea las tablas si no existen
    initDatabase();

    // Contenedor y diseño principal vertical
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Formulario de entrada de datos (Etiqueta -> Campo de entrada)
    QFormLayout* formLayout = new QFormLayout();
    
    // Campo ID: se mantiene oculto en la interfaz, pero es esencial para saber qué fila actualizar/eliminar
    txtId = new QLineEdit();
    txtId->setVisible(false);

    // Campo Nombre: incluye validador que solo permite letras y espacios
    txtNombre = new QLineEdit();
    txtNombre->setPlaceholderText("Solo letras (ej: Juan Pérez)");
    QRegularExpression regexSoloLetras(R"(^[\p{L} ]*$)");
    txtNombre->setValidator(new QRegularExpressionValidator(regexSoloLetras, this));

    // Campo Email: dato confidencial
    txtEmailSensible = new QLineEdit();
    txtEmailSensible->setPlaceholderText("Se almacenará cifrado en la BD");

    // Campo Edad: usa el control personalizado AgeSpinBox (rango 18-60, inicia vacío)
    spinEdad = new AgeSpinBox(this);

    // Campo Tarea asignada
    txtTarea = new QLineEdit();
    txtTarea->setPlaceholderText("Ej: Revisión de inventario / Deploy");

    // Selector del estado inicial de la tarea
    comboEstado = new QComboBox();
    comboEstado->addItems({"Pendiente", "En Proceso", "Finalizado"});

    // Agrega las filas al formulario
    formLayout->addRow("Nombre:", txtNombre);
    formLayout->addRow("Email (Dato Protegido):", txtEmailSensible);
    formLayout->addRow("Edad:", spinEdad);
    formLayout->addRow("Tarea Asignada:", txtTarea);
    formLayout->addRow("Estado de la Tarea:", comboEstado);
    mainLayout->addLayout(formLayout);

    // Botonera de acciones CRUD (Insertar, Actualizar, Eliminar, Limpiar)
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnInsertar = new QPushButton("Insertar");
    btnActualizar = new QPushButton("Actualizar");
    btnEliminar = new QPushButton("Eliminar");
    btnLimpiar = new QPushButton("Limpiar Formulario");

    btnLayout->addWidget(btnInsertar);
    btnLayout->addWidget(btnActualizar);
    btnLayout->addWidget(btnEliminar);
    btnLayout->addWidget(btnLimpiar);
    mainLayout->addLayout(btnLayout);

    // Barra de Monitoreo en tiempo real (Filtro por Estado)
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("<b>Monitoreo por Estado:</b>"));
    comboFiltroEstado = new QComboBox();
    comboFiltroEstado->addItems({"Todos", "Pendiente", "En Proceso", "Finalizado"});
    filterLayout->addWidget(comboFiltroEstado);
    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);

    // Tabla de visualización de registros
    tablaUsuarios = new QTableWidget();
    tablaUsuarios->setColumnCount(6);
    tablaUsuarios->verticalHeader()->setVisible(false); // Oculta números de fila predeterminados
    tablaUsuarios->setHorizontalHeaderLabels({"ID", "Nombre", "Email (Descifrado)", "Edad", "Tarea", "Estado"});
    tablaUsuarios->setColumnHidden(0, true);             // Oculta la columna de ID interno
    tablaUsuarios->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Expande columnas
    tablaUsuarios->setSelectionBehavior(QAbstractItemView::SelectRows);             // Selecciona filas enteras
    tablaUsuarios->setEditTriggers(QAbstractItemView::NoEditTriggers);              // Modo solo lectura
    mainLayout->addWidget(tablaUsuarios);

    setCentralWidget(centralWidget);

    // Conexión de eventos (Señales y Ranuras / Signals and Slots)
    connect(btnInsertar, &QPushButton::clicked, this, &MainWindow::handleInsertar);
    connect(btnActualizar, &QPushButton::clicked, this, &MainWindow::handleActualizar);
    connect(btnEliminar, &QPushButton::clicked, this, &MainWindow::handleEliminar);
    connect(btnLimpiar, &QPushButton::clicked, this, &MainWindow::handleLimpiar);
    connect(tablaUsuarios, &QTableWidget::cellClicked, this, &MainWindow::handleRowSelected);
    connect(comboFiltroEstado, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::handleFiltrarPorEstado);

    // Carga inicial de datos existentes en la base de datos
    cargarTabla();
}

MainWindow::~MainWindow() {
    // Al cerrar la ventana, se asegura de cerrar correctamente la conexión con SQLite
    if (db) {
        sqlite3_close(db);
    }
}

// ============================================================================
// GESTIÓN DE BASE DE DATOS SQLITE (INICIALIZACIÓN Y CONSULTAS)
// ============================================================================

/**
 * @brief Inicializa la base de datos SQLite creando el archivo y las tablas si no existen.
 */
void MainWindow::initDatabase() {
    // Obtiene una ruta segura y con permisos de escritura según el sistema operativo (AppData)
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(basePath);
    QString dbPath = basePath + "/usuarios_tareas.db";

    // Abre o crea el archivo SQLite
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) {
        QMessageBox::critical(this, "Error BD", sqlite3_errmsg(db));
        return;
    }

    // Esquema de tabla e índice de optimización
    const char* sql = "CREATE TABLE IF NOT EXISTS usuarios ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "nombre TEXT NOT NULL, "
                      "email TEXT NOT NULL, "
                      "edad INTEGER, "
                      "tarea TEXT, "
                      "estado TEXT DEFAULT 'Pendiente'); "
                      "CREATE INDEX IF NOT EXISTS idx_usuarios_estado ON usuarios(estado);";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        QMessageBox::critical(this, "Error BD", errMsg);
        sqlite3_free(errMsg);
    }
}

/**
 * @brief Consulta la base de datos y llena la tabla gráfica con los registros.
 * @param filtroEstado Filtro opcional ("Todos", "Pendiente", "En Proceso", "Finalizado")
 */
void MainWindow::cargarTabla(const QString& filtroEstado) {
    // Desactiva actualizaciones visuales momentáneamente para mayor rendimiento al cargar muchas filas
    tablaUsuarios->setUpdatesEnabled(false);
    tablaUsuarios->setRowCount(0);

    // Construcción dinámica de la consulta SQL según el filtro seleccionado
    QString query = "SELECT id, nombre, email, edad, tarea, estado FROM usuarios";
    if (filtroEstado != "Todos") {
        query += " WHERE estado = ?";
    }
    query += " ORDER BY id DESC;"; // Ordena desde el más reciente hasta el más antiguo

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.toUtf8().constData(), -1, &stmt, nullptr) != SQLITE_OK) {
        tablaUsuarios->setUpdatesEnabled(true);
        return;
    }

    // Si hay filtro activo, enlaza el parámetro
    if (filtroEstado != "Todos") {
        sqlite3_bind_text(stmt, 1, filtroEstado.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    }

    int row = 0;
    // Itera fila por fila sobre el conjunto de resultados retornado por SQLite
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tablaUsuarios->insertRow(row);

        // Lectura de cada columna del registro
        int id = sqlite3_column_int(stmt, 0);
        QString nombre = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        QString emailCifrado = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        int edad = sqlite3_column_int(stmt, 3);
        
        const char* tareaTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const char* estadoTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        // Descifra el correo en memoria únicamente para su presentación en pantalla
        QString emailClaro = descifrar(emailCifrado);

        // Asignación de celdas a la fila correspondiente de la tabla
        tablaUsuarios->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
        tablaUsuarios->setItem(row, 1, new QTableWidgetItem(nombre));
        tablaUsuarios->setItem(row, 2, new QTableWidgetItem(emailClaro));
        tablaUsuarios->setItem(row, 3, new QTableWidgetItem(QString::number(edad)));
        tablaUsuarios->setItem(row, 4, new QTableWidgetItem(tareaTxt ? QString::fromUtf8(tareaTxt) : ""));
        tablaUsuarios->setItem(row, 5, new QTableWidgetItem(estadoTxt ? QString::fromUtf8(estadoTxt) : "Pendiente"));

        row++;
    }
    sqlite3_finalize(stmt); // Libera la memoria de la sentencia preparada
    tablaUsuarios->setUpdatesEnabled(true); // Reactiva las actualizaciones visuales
}

// ============================================================================
// OPERACIONES CRUD (SLOTS DE LA INTERFAZ DE USUARIO)
// ============================================================================

/**
 * @brief Inserta un nuevo usuario y tarea en la base de datos tras validar los datos.
 */
void MainWindow::handleInsertar() {
    // 1. Obtiene los valores ingresados eliminando espacios en blanco sobrantes
    QString nombre = txtNombre->text().trimmed();
    QString email = txtEmailSensible->text().trimmed();
    QString tarea = txtTarea->text().trimmed();
    QString estado = comboEstado->currentText();

    // 2. Validación de campos obligatorios
    if (nombre.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Validación", "Nombre y Email son requeridos.");
        return;
    }

    // 3. Validación estricta de la Edad: Debe estar comprendida exclusivamente entre 18 y 60 años
    if (!spinEdad->hasValidAge()) {
        QMessageBox::warning(this, "Validación", "La edad es requerida y debe estar comprendida entre 18 y 60 años.");
        return;
    }

    int edad = spinEdad->getAge();

    // 4. Validación del Nombre: No se permiten números ni caracteres especiales
    QRegularExpression regexNombre(R"(^[\p{L}]+(?:\s+[\p{L}]+)*$)");
    if (!regexNombre.match(nombre).hasMatch()) {
        QMessageBox::warning(this, "Validación", "El Nombre solo debe contener letras (sin números ni caracteres especiales).");
        return;
    }

    // 5. Validación del formato de Email (ej: usuario@dominio.ext)
    QRegularExpression regexEmail(R"(^[^@\s]+@[^@\s]+\.[^@\s]{2,}$)");
    if (!regexEmail.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validación", "El formato del email no es válido. Asegúrate de incluir texto entre el '@' y el '.', y al menos dos letras al final.");
        return;
    }

    // 6. Cifra el correo electrónico antes de almacenarlo en la base de datos
    QString emailCifrado = cifrar(email);

    // 7. Preparación y ejecución de la sentencia SQL INSERT
    const char* sql = "INSERT INTO usuarios (nombre, email, edad, tarea, estado) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
        return;
    }

    bindCamposUsuario(stmt, nombre, emailCifrado, edad, tarea, estado);

    // 8. Confirma la inserción, limpia el formulario y recarga la tabla
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        handleLimpiar();
        cargarTabla(comboFiltroEstado->currentText());
    } else {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

/**
 * @brief Actualiza los datos del usuario seleccionado previamente en la tabla.
 */
void MainWindow::handleActualizar() {
    // 1. Verifica que exista un registro seleccionado de la tabla (ID no vacío)
    if (txtId->text().isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Selecciona primero un registro de la tabla.");
        return;
    }

    int id = txtId->text().toInt();
    QString nombre = txtNombre->text().trimmed();
    QString email = txtEmailSensible->text().trimmed();
    QString tarea = txtTarea->text().trimmed();
    QString estado = comboEstado->currentText();

    // 2. Validación de campos obligatorios
    if (nombre.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Validación", "Nombre y Email son requeridos.");
        return;
    }

    // 3. Validación estricta de la Edad: Debe estar comprendida entre 18 y 60 años
    if (!spinEdad->hasValidAge()) {
        QMessageBox::warning(this, "Validación", "La edad es requerida y debe estar comprendida entre 18 y 60 años.");
        return;
    }

    int edad = spinEdad->getAge();

    // 4. Validación del Nombre
    QRegularExpression regexNombre(R"(^[\p{L}]+(?:\s+[\p{L}]+)*$)");
    if (!regexNombre.match(nombre).hasMatch()) {
        QMessageBox::warning(this, "Validación", "El Nombre solo debe contener letras (sin números ni caracteres especiales).");
        return;
    }

    // 5. Validación del formato de Email
    QRegularExpression regexEmail(R"(^[^@\s]+@[^@\s]+\.[^@\s]{2,}$)");
    if (!regexEmail.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validación", "El formato del email no es válido. Asegúrate de incluir texto entre el '@' y el '.', y al menos dos letras al final.");
        return;
    }

    // 6. Cifrado del correo electrónico
    QString emailCifrado = cifrar(email);

    // 7. Preparación y ejecución de la sentencia SQL UPDATE
    const char* sql = "UPDATE usuarios SET nombre = ?, email = ?, edad = ?, tarea = ?, estado = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
        return;
    }

    bindCamposUsuario(stmt, nombre, emailCifrado, edad, tarea, estado);
    sqlite3_bind_int(stmt, 6, id); // Parámetro 6: ID del registro a modificar

    // 8. Confirma la actualización, limpia el formulario y recarga la tabla
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        handleLimpiar();
        cargarTabla(comboFiltroEstado->currentText());
    } else {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

/**
 * @brief Elimina de la base de datos el usuario seleccionado actualmente.
 */
void MainWindow::handleEliminar() {
    // Verifica que haya un registro seleccionado
    if (txtId->text().isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Selecciona un registro de la tabla.");
        return;
    }

    int id = txtId->text().toInt();
    const char* sql = "DELETE FROM usuarios WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    // Confirma el borrado y actualiza la vista
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        handleLimpiar();
        cargarTabla(comboFiltroEstado->currentText());
    } else {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

/**
 * @brief Restablece todos los campos del formulario dejándolos vacíos.
 */
void MainWindow::handleLimpiar() {
    txtId->clear();
    txtNombre->clear();
    txtEmailSensible->clear();
    spinEdad->clear(); // Deja el campo de edad vacío (sin el 25) y con el placeholder visible
    txtTarea->clear();
    comboEstado->setCurrentIndex(0); // Vuelve al primer estado ("Pendiente")
    tablaUsuarios->clearSelection(); // Deselecciona cualquier fila activa en la tabla
}

/**
 * @brief Se activa al hacer clic sobre una fila de la tabla; traslada los datos al formulario.
 */
void MainWindow::handleRowSelected(int row, int) {
    if (row < 0 || !tablaUsuarios->item(row, 0)) return;

    // Lee los datos de la fila seleccionada y los coloca en los campos de edición
    txtId->setText(tablaUsuarios->item(row, 0)->text());
    txtNombre->setText(tablaUsuarios->item(row, 1)->text());
    txtEmailSensible->setText(tablaUsuarios->item(row, 2)->text());
    spinEdad->setValue(tablaUsuarios->item(row, 3)->text().toInt()); // Carga la edad del registro seleccionado
    txtTarea->setText(tablaUsuarios->item(row, 4)->text());

    // Sincroniza el selector de estado con el valor del registro
    QString estado = tablaUsuarios->item(row, 5)->text();
    int idx = comboEstado->findText(estado);
    if (idx != -1) comboEstado->setCurrentIndex(idx);
}

/**
 * @brief Se activa cuando el usuario cambia la opción del filtro de monitoreo.
 */
void MainWindow::handleFiltrarPorEstado(int) {
    cargarTabla(comboFiltroEstado->currentText());
}