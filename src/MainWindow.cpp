#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <string>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Gestión de Usuarios - SQLite CRUD");
    resize(700, 500);

    initDatabase();

    // Widget central y layout principal
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Formulario de campos
    QFormLayout* formLayout = new QFormLayout();
    txtId = new QLineEdit();
    txtId->setReadOnly(true);
    txtId->setPlaceholderText("Autogenerado");

    txtNombre = new QLineEdit();
    txtEmail = new QLineEdit();
    spinEdad = new QSpinBox();
    spinEdad->setRange(1, 120);
    spinEdad->setValue(18);

    formLayout->addRow("ID:", txtId);
    formLayout->addRow("Nombre:", txtNombre);
    formLayout->addRow("Email:", txtEmail);
    formLayout->addRow("Edad:", spinEdad);
    mainLayout->addLayout(formLayout);

    // Botones de acción
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnInsertar = new QPushButton("Insertar");
    btnActualizar = new QPushButton("Actualizar");
    btnEliminar = new QPushButton("Eliminar");
    btnLimpiar = new QPushButton("Limpiar Selección");

    btnLayout->addWidget(btnInsertar);
    btnLayout->addWidget(btnActualizar);
    btnLayout->addWidget(btnEliminar);
    btnLayout->addWidget(btnLimpiar);
    mainLayout->addLayout(btnLayout);

    // Tabla de visualización
    tablaUsuarios = new QTableWidget();
    tablaUsuarios->setColumnCount(4);
    tablaUsuarios->setHorizontalHeaderLabels({"ID", "Nombre", "Email", "Edad"});
    tablaUsuarios->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablaUsuarios->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaUsuarios->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(tablaUsuarios);

    setCentralWidget(centralWidget);

    // Conexión de señales y slots
    connect(btnInsertar, &QPushButton::clicked, this, &MainWindow::handleInsertar);
    connect(btnActualizar, &QPushButton::clicked, this, &MainWindow::handleActualizar);
    connect(btnEliminar, &QPushButton::clicked, this, &MainWindow::handleEliminar);
    connect(btnLimpiar, &QPushButton::clicked, this, &MainWindow::handleLimpiar);
    connect(tablaUsuarios, &QTableWidget::cellClicked, this, &MainWindow::handleRowSelected);

    cargarTabla();
}

MainWindow::~MainWindow() {
    if (db) {
        sqlite3_close(db);
    }
}

void MainWindow::initDatabase() {
    if (sqlite3_open("empresa.db", &db) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", QString("No se pudo abrir la BD: %1").arg(sqlite3_errmsg(db)));
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS usuarios ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "nombre TEXT NOT NULL, "
                      "email TEXT NOT NULL UNIQUE, "
                      "edad INTEGER);";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", QString("Error al crear tabla: %1").arg(errMsg));
        sqlite3_free(errMsg);
    }
}

void MainWindow::cargarTabla() {
    tablaUsuarios->setRowCount(0);

    const char* sql = "SELECT id, nombre, email, edad FROM usuarios;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    int row = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tablaUsuarios->insertRow(row);

        int id = sqlite3_column_int(stmt, 0);
        const char* nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int edad = sqlite3_column_int(stmt, 3);

        tablaUsuarios->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
        tablaUsuarios->setItem(row, 1, new QTableWidgetItem(QString::fromUtf8(nombre ? nombre : "")));
        tablaUsuarios->setItem(row, 2, new QTableWidgetItem(QString::fromUtf8(email ? email : "")));
        tablaUsuarios->setItem(row, 3, new QTableWidgetItem(QString::number(edad)));

        row++;
    }
    sqlite3_finalize(stmt);
}

void MainWindow::handleInsertar() {
    std::string nombre = txtNombre->text().trimmed().toStdString();
    std::string email = txtEmail->text().trimmed().toStdString();
    int edad = spinEdad->value();

    if (nombre.empty() || email.empty()) {
        QMessageBox::warning(this, "Validación", "Nombre y Email son obligatorios.");
        return;
    }

    const char* sql = "INSERT INTO usuarios (nombre, email, edad) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, nombre.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, edad);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        QMessageBox::critical(this, "Error", QString("No se pudo insertar: %1").arg(sqlite3_errmsg(db)));
    } else {
        QMessageBox::information(this, "Éxito", "Usuario insertado correctamente.");
        handleLimpiar();
        cargarTabla();
    }
    sqlite3_finalize(stmt);
}

void MainWindow::handleActualizar() {
    if (txtId->text().isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Selecciona primero un usuario de la tabla para editar.");
        return;
    }

    int id = txtId->text().toInt();
    std::string nombre = txtNombre->text().trimmed().toStdString();
    int edad = spinEdad->value();

    const char* sql = "UPDATE usuarios SET nombre = ?, edad = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, nombre.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, edad);
    sqlite3_bind_int(stmt, 3, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    } else {
        QMessageBox::information(this, "Éxito", "Usuario actualizado correctamente.");
        handleLimpiar();
        cargarTabla();
    }
    sqlite3_finalize(stmt);
}

void MainWindow::handleEliminar() {
    if (txtId->text().isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Selecciona un usuario de la tabla para eliminar.");
        return;
    }

    int id = txtId->text().toInt();
    auto confirm = QMessageBox::question(this, "Confirmar", 
                                        QString("¿Seguro que deseas eliminar el ID %1?").arg(id),
                                        QMessageBox::Yes | QMessageBox::No);
    if (confirm != QMessageBox::Yes) return;

    const char* sql = "DELETE FROM usuarios WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        QMessageBox::critical(this, "Error", sqlite3_errmsg(db));
    } else {
        QMessageBox::information(this, "Éxito", "Usuario eliminado.");
        handleLimpiar();
        cargarTabla();
    }
    sqlite3_finalize(stmt);
}

void MainWindow::handleLimpiar() {
    txtId->clear();
    txtNombre->clear();
    txtEmail->clear();
    txtEmail->setEnabled(true);
    spinEdad->setValue(18);
    tablaUsuarios->clearSelection();
}

void MainWindow::handleRowSelected(int row, int /*column*/) {
    txtId->setText(tablaUsuarios->item(row, 0)->text());
    txtNombre->setText(tablaUsuarios->item(row, 1)->text());
    txtEmail->setText(tablaUsuarios->item(row, 2)->text());
    txtEmail->setEnabled(false); // Para no modificar el email único en el update
    spinEdad->setValue(tablaUsuarios->item(row, 3)->text().toInt());
}