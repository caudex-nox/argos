#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QPushButton>
#include <sqlite3.h>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void handleInsertar();
    void handleActualizar();
    void handleEliminar();
    void handleLimpiar();
    void handleRowSelected(int row, int column);

private:
    void initDatabase();
    void cargarTabla();

    sqlite3* db = nullptr;

    // Inputs de la interfaz
    QLineEdit* txtId;
    QLineEdit* txtNombre;
    QLineEdit* txtEmail;
    QSpinBox* spinEdad;

    // Botones
    QPushButton* btnInsertar;
    QPushButton* btnActualizar;
    QPushButton* btnEliminar;
    QPushButton* btnLimpiar;

    // Tabla
    QTableWidget* tablaUsuarios;
};