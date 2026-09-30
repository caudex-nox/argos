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

void MainWindow::bindCamposUsuario(sqlite3_stmt* stmt, const QString& nombre, const QString& emailCifrado, int edad, const QString& tarea, const QString& estado) {
    sqlite3_bind_text(stmt, 1, nombre.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, emailCifrado.toLatin1().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, edad);
    sqlite3_bind_text(stmt, 4, tarea.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, estado.toUtf8().constData(), -1, SQLITE_TRANSIENT);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Argos: Gestor de Tareas");
    resize(900, 580);
    setWindowIcon(QIcon(":/resources/argos-icon.png"));

    initDatabase();

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QFormLayout* formLayout = new QFormLayout();
    
    txtId = new QLineEdit();
    txtId->setVisible(false);

    txtNombre = new QLineEdit();
    txtNombre->setPlaceholderText("Solo letras (ej: Juan Pérez)");
    QRegularExpression regexSoloLetras(R"(^[\p{L} ]*$)");
    txtNombre->setValidator(new QRegularExpressionValidator(regexSoloLetras, this));

    txtEmailSensible = new QLineEdit();
    txtEmailSensible->setPlaceholderText("Se almacenará cifrado en la BD");

    spinEdad = new QSpinBox();
    spinEdad->setRange(1, 120);
    spinEdad->setValue(25);

    txtTarea = new QLineEdit();
    txtTarea->setPlaceholderText("Ej: Revisión de inventario / Deploy");

    comboEstado = new QComboBox();
    comboEstado->addItems({"Pendiente", "En Proceso", "Finalizado"});

    formLayout->addRow("Nombre:", txtNombre);
    formLayout->addRow("Email (Dato Protegido):", txtEmailSensible);
    formLayout->addRow("Edad:", spinEdad);
    formLayout->addRow("Tarea Asignada:", txtTarea);
    formLayout->addRow("Estado de la Tarea:", comboEstado);
    mainLayout->addLayout(formLayout);

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

    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("<b>Monitoreo por Estado:</b>"));
    comboFiltroEstado = new QComboBox();
    comboFiltroEstado->addItems({"Todos", "Pendiente", "En Proceso", "Finalizado"});
    filterLayout->addWidget(comboFiltroEstado);
    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);

    tablaUsuarios = new QTableWidget();
    tablaUsuarios->setColumnCount(6);
    tablaUsuarios->verticalHeader()->setVisible(false);
    tablaUsuarios->setHorizontalHeaderLabels({"ID", "Nombre", "Email (Descifrado)", "Edad", "Tarea", "Estado"});
    tablaUsuarios->setColumnHidden(0, true);
    tablaUsuarios->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablaUsuarios->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaUsuarios->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(tablaUsuarios);

    setCentralWidget(centralWidget);

    connect(btnInsertar, &QPushButton::clicked, this, &MainWindow::handleInsertar);
    connect(btnActualizar, &QPushButton::clicked, this, &MainWindow::handleActualizar);
    connect(btnEliminar, &QPushButton::clicked, this, &MainWindow::handleEliminar);
    connect(btnLimpiar, &QPushButton::clicked, this, &MainWindow::handleLimpiar);
    connect(tablaUsuarios, &QTableWidget::cellClicked, this, &MainWindow::handleRowSelected);
    connect(comboFiltroEstado, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::handleFiltrarPorEstado);

    cargarTabla();
}

MainWindow::~MainWindow() {
    if (db) {
        sqlite3_close(db);
    }
}

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

void MainWindow::cargarTabla(const QString& filtroEstado) {
    tablaUsuarios->setUpdatesEnabled(false);
    tablaUsuarios->setRowCount(0);

    QString query = "SELECT id, nombre, email, edad, tarea, estado FROM usuarios";
    if (filtroEstado != "Todos") {
        query += " WHERE estado = ?";
    }
    query += " ORDER BY id DESC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.toUtf8().constData(), -1, &stmt, nullptr) != SQLITE_OK) {
        tablaUsuarios->setUpdatesEnabled(true);
        return;
    }

    if (filtroEstado != "Todos") {
        sqlite3_bind_text(stmt, 1, filtroEstado.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    }

    int row = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tablaUsuarios->insertRow(row);

        int id = sqlite3_column_int(stmt, 0);
        QString nombre = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        QString emailCifrado = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        int edad = sqlite3_column_int(stmt, 3);
        
        const char* tareaTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const char* estadoTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

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
}

void MainWindow::handleInsertar() {
    QString nombre = txtNombre->text().trimmed();
    QString email = txtEmailSensible->text().trimmed();
    int edad = spinEdad->value();
    QString tarea = txtTarea->text().trimmed();
    QString estado = comboEstado->currentText();

    if (nombre.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Validación", "Nombre y Email son requeridos.");
        return;
    }

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
        cargarTabla(comboFiltroEstado->currentText());
    } else {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

void MainWindow::handleActualizar() {
    if (txtId->text().isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Selecciona primero un registro de la tabla.");
        return;
    }

    int id = txtId->text().toInt();
    QString nombre = txtNombre->text().trimmed();
    QString email = txtEmailSensible->text().trimmed();
    int edad = spinEdad->value();
    QString tarea = txtTarea->text().trimmed();
    QString estado = comboEstado->currentText();

    if (nombre.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Validación", "Nombre y Email son requeridos.");
        return;
    }

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
        cargarTabla(comboFiltroEstado->currentText());
    } else {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

void MainWindow::handleEliminar() {
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

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        handleLimpiar();
        cargarTabla(comboFiltroEstado->currentText());
    } else {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

void MainWindow::handleLimpiar() {
    txtId->clear();
    txtNombre->clear();
    txtEmailSensible->clear();
    spinEdad->setValue(25);
    txtTarea->clear();
    comboEstado->setCurrentIndex(0);
    tablaUsuarios->clearSelection();
}

void MainWindow::handleRowSelected(int row, int) {
    if (row < 0 || !tablaUsuarios->item(row, 0)) return;

    txtId->setText(tablaUsuarios->item(row, 0)->text());
    txtNombre->setText(tablaUsuarios->item(row, 1)->text());
    txtEmailSensible->setText(tablaUsuarios->item(row, 2)->text());
    spinEdad->setValue(tablaUsuarios->item(row, 3)->text().toInt());
    txtTarea->setText(tablaUsuarios->item(row, 4)->text());

    QString estado = tablaUsuarios->item(row, 5)->text();
    int idx = comboEstado->findText(estado);
    if (idx != -1) comboEstado->setCurrentIndex(idx);
}

void MainWindow::handleFiltrarPorEstado(int) {
    cargarTabla(comboFiltroEstado->currentText());
}