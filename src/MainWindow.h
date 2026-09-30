#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
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
    void handleFiltrarPorEstado(int index);

private:
    void initDatabase();
    void cargarTabla(const QString& filtroEstado = "Todos");

    // Métodos de encriptación local para datos sensibles
    static QString cifrar(const QString& texto);
    static QString descifrar(const QString& texto);

    sqlite3* db = nullptr;

    // Campos de entrada (txtId es interno para operaciones CRUD y no se expone en la interfaz)
    QLineEdit* txtId;
    QLineEdit* txtNombre;
    QLineEdit* txtEmailSensible;
    QSpinBox* spinEdad;
    QLineEdit* txtTarea;
    QComboBox* comboEstado;

    // Filtro de monitoreo
    QComboBox* comboFiltroEstado;

    // Botones
    QPushButton* btnInsertar;
    QPushButton* btnActualizar;
    QPushButton* btnEliminar;
    QPushButton* btnLimpiar;

    // Tabla
    QTableWidget* tablaUsuarios;
};