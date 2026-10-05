#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QStandardPaths>
#include <QDir>
#include <QByteArray>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

// Clave simétrica para el cifrado de datos sensibles
const quint8 CLAVE_ENCRIPTACION = 0x6C;

// Cifrado de texto
QString MainWindow::cifrar(const QString& texto) {
    QByteArray bytes = texto.toUtf8();
    for (int i = 0; i < bytes.size(); ++i) {
        bytes[i] = bytes[i] ^ CLAVE_ENCRIPTACION;
    }
    return QString::fromLatin1(bytes.toBase64());
}

// Decodificación
QString MainWindow::descifrar(const QString& texto) {
    QByteArray bytes = QByteArray::fromBase64(texto.toLatin1());
    for (int i = 0; i < bytes.size(); ++i) {
        bytes[i] = bytes[i] ^ CLAVE_ENCRIPTACION;
    }
    return QString::fromUtf8(bytes);
}

// Vinculación de atributos del usuario a los parámetros de SQLite
void MainWindow::bindCamposUsuario(sqlite3_stmt* stmt, const QString& nombre, const QString& emailCifrado, int edad, const QString& tarea, const QString& estado) {
    sqlite3_bind_text(stmt, 1, nombre.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, emailCifrado.toLatin1().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, edad);
    sqlite3_bind_text(stmt, 4, tarea.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, estado.toUtf8().constData(), -1, SQLITE_TRANSIENT);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Restricción del campo Nombre a caracteres alfabéticos y espacios
    QRegularExpression regexSoloLetras(R"(^[\p{L} ]*$)");
    ui->txtNombre->setValidator(new QRegularExpressionValidator(regexSoloLetras, this));

    // Oculta la columna ID
    ui->tablaUsuarios->setColumnHidden(0, true);
    ui->tablaUsuarios->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    initDatabase();

    // Conexión de las señales de interfaz con sus respectivos slots
    connect(ui->btnInsertar, &QPushButton::clicked, this, &MainWindow::handleInsertar);
    connect(ui->btnActualizar, &QPushButton::clicked, this, &MainWindow::handleActualizar);
    connect(ui->btnEliminar, &QPushButton::clicked, this, &MainWindow::handleEliminar);
    connect(ui->btnLimpiar, &QPushButton::clicked, this, &MainWindow::handleLimpiar);
    connect(ui->tablaUsuarios, &QTableWidget::cellClicked, this, &MainWindow::handleRowSelected);
    connect(ui->comboFiltroEstado, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::handleFiltrarPorEstado);

    cargarTabla();
}

// Cierre de la conexión con SQLite 3
MainWindow::~MainWindow() {
    delete ui;
    if (db) {
        sqlite3_close(db);
    }
}

// Inicialización de la base de datos local
void MainWindow::initDatabase() {
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(basePath);
    QString dbPath = basePath + "/usuarios_tareas.db";

    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) {
        QMessageBox::critical(this, "Error BD", sqlite3_errmsg(db));
        return;
    }

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

// Consulta de registros
void MainWindow::cargarTabla(const QString& filtroEstado) {
    // Deshabilita el repintado para optimizar la inserción de filas
    ui->tablaUsuarios->setUpdatesEnabled(false);
    ui->tablaUsuarios->setRowCount(0);

    QString query = "SELECT id, nombre, email, edad, tarea, estado FROM usuarios";
    if (filtroEstado != "Todos") {
        query += " WHERE estado = ?";
    }
    query += " ORDER BY id DESC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.toUtf8().constData(), -1, &stmt, nullptr) != SQLITE_OK) {
        ui->tablaUsuarios->setUpdatesEnabled(true);
        return;
    }

    if (filtroEstado != "Todos") {
        sqlite3_bind_text(stmt, 1, filtroEstado.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    }

    int row = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ui->tablaUsuarios->insertRow(row);

        int id = sqlite3_column_int(stmt, 0);
        QString nombre = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        QString emailCifrado = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        int edad = sqlite3_column_int(stmt, 3);
        
        const char* tareaTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const char* estadoTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        // Descifra el correo electrónico
        QString emailClaro = descifrar(emailCifrado);

        ui->tablaUsuarios->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
        ui->tablaUsuarios->setItem(row, 1, new QTableWidgetItem(nombre));
        ui->tablaUsuarios->setItem(row, 2, new QTableWidgetItem(emailClaro));
        ui->tablaUsuarios->setItem(row, 3, new QTableWidgetItem(QString::number(edad)));
        ui->tablaUsuarios->setItem(row, 4, new QTableWidgetItem(tareaTxt ? QString::fromUtf8(tareaTxt) : ""));
        ui->tablaUsuarios->setItem(row, 5, new QTableWidgetItem(estadoTxt ? QString::fromUtf8(estadoTxt) : "Pendiente"));

        row++;
    }
    sqlite3_finalize(stmt);
    ui->tablaUsuarios->setUpdatesEnabled(true);
}

// Valida los datos del formulario, cifra el email e inserta el nuevo registro en la BD
void MainWindow::handleInsertar() {
    QString nombre = ui->txtNombre->text().trimmed();
    QString email = ui->txtEmailSensible->text().trimmed();
    QString tarea = ui->txtTarea->text().trimmed();
    QString estado = ui->comboEstado->currentText();

    if (nombre.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Validación", "Nombre y Email son requeridos.");
        return;
    }

    if (!ui->spinEdad->hasValidAge()) {
        QMessageBox::warning(this, "Validación", "La edad es requerida y debe estar comprendida entre 18 y 60 años.");
        return;
    }

    int edad = ui->spinEdad->getAge();

    QRegularExpression regexNombre(R"(^[\p{L}]+(?:\s+[\p{L}]+)*$)");
    if (!regexNombre.match(nombre).hasMatch()) {
        QMessageBox::warning(this, "Validación", "El Nombre solo debe contener letras (sin números ni caracteres especiales).");
        return;
    }

    QRegularExpression regexEmail(R"(^[^@\s]+@[^@\s]+\.[^@\s]{2,}$)");
    if (!regexEmail.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validación", "El formato del email no es válido. Asegúrate de incluir texto entre el '@' y el '.', y al menos dos letras al final.");
        return;
    }

    QString emailCifrado = cifrar(email);

    const char* sql = "INSERT INTO usuarios (nombre, email, edad, tarea, estado) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
        return;
    }

    bindCamposUsuario(stmt, nombre, emailCifrado, edad, tarea, estado);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        handleLimpiar();
        cargarTabla(ui->comboFiltroEstado->currentText());
    } else {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

// Valida la selección y campos modificados, cifra el email y actualiza el registro por su ID
void MainWindow::handleActualizar() {
    if (ui->txtId->text().isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Selecciona primero un registro de la tabla.");
        return;
    }

    int id = ui->txtId->text().toInt();
    QString nombre = ui->txtNombre->text().trimmed();
    QString email = ui->txtEmailSensible->text().trimmed();
    QString tarea = ui->txtTarea->text().trimmed();
    QString estado = ui->comboEstado->currentText();

    if (nombre.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Validación", "Nombre y Email son requeridos.");
        return;
    }

    if (!ui->spinEdad->hasValidAge()) {
        QMessageBox::warning(this, "Validación", "La edad es requerida y debe estar comprendida entre 18 y 60 años.");
        return;
    }

    int edad = ui->spinEdad->getAge();

    QRegularExpression regexNombre(R"(^[\p{L}]+(?:\s+[\p{L}]+)*$)");
    if (!regexNombre.match(nombre).hasMatch()) {
        QMessageBox::warning(this, "Validación", "El Nombre solo debe contener letras (sin números ni caracteres especiales).");
        return;
    }

    QRegularExpression regexEmail(R"(^[^@\s]+@[^@\s]+\.[^@\s]{2,}$)");
    if (!regexEmail.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validación", "El formato del email no es válido. Asegúrate de incluir texto entre el '@' y el '.', y al menos dos letras al final.");
        return;
    }

    QString emailCifrado = cifrar(email);

    const char* sql = "UPDATE usuarios SET nombre = ?, email = ?, edad = ?, tarea = ?, estado = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
        return;
    }

    bindCamposUsuario(stmt, nombre, emailCifrado, edad, tarea, estado);
    sqlite3_bind_int(stmt, 6, id);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        handleLimpiar();
        cargarTabla(ui->comboFiltroEstado->currentText());
    } else {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

// Elimina el registro seleccionado
void MainWindow::handleEliminar() {
    if (ui->txtId->text().isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Selecciona un registro de la tabla.");
        return;
    }

    int id = ui->txtId->text().toInt();
    const char* sql = "DELETE FROM usuarios WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        handleLimpiar();
        cargarTabla(ui->comboFiltroEstado->currentText());
    } else {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

// Restablece los campos del formulario
void MainWindow::handleLimpiar() {
    ui->txtId->clear();
    ui->txtNombre->clear();
    ui->txtEmailSensible->clear();
    ui->spinEdad->clear();
    ui->txtTarea->clear();
    ui->comboEstado->setCurrentIndex(0);
    ui->tablaUsuarios->clearSelection();
}

// Traslada los datos de la fila seleccionada a los campos del formulario
void MainWindow::handleRowSelected(int row, int) {
    if (row < 0 || !ui->tablaUsuarios->item(row, 0)) return;

    ui->txtId->setText(ui->tablaUsuarios->item(row, 0)->text());
    ui->txtNombre->setText(ui->tablaUsuarios->item(row, 1)->text());
    ui->txtEmailSensible->setText(ui->tablaUsuarios->item(row, 2)->text());
    ui->spinEdad->setValue(ui->tablaUsuarios->item(row, 3)->text().toInt());
    ui->txtTarea->setText(ui->tablaUsuarios->item(row, 4)->text());

    QString estado = ui->tablaUsuarios->item(row, 5)->text();
    int idx = ui->comboEstado->findText(estado);
    if (idx != -1) ui->comboEstado->setCurrentIndex(idx);
}

// Recarga la tabla filtrando por el estado de tarea seleccionado
void MainWindow::handleFiltrarPorEstado(int) {
    cargarTabla(ui->comboFiltroEstado->currentText());
}