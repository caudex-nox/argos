#pragma once

#include <QMainWindow>
#include "sqlite3.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Ventana principal
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

// Slots para eventos de la interfaz
private slots:
    void handleInsertar();
    void handleActualizar();
    void handleEliminar();
    void handleLimpiar();
    void handleRowSelected(int row, int column);
    void handleFiltrarPorEstado(int index);

private:
    // Gestión y consulta de la base de datos
    void initDatabase();
    void cargarTabla(const QString& filtroEstado = "Todos");

    // Cifrado simétrico para la protección del correo electrónico
    static QString cifrar(const QString& texto);
    static QString descifrar(const QString& texto);

    // Enlace seguro de parámetros
    static void bindCamposUsuario(sqlite3_stmt* stmt, const QString& nombre, const QString& emailCifrado, int edad, const QString& tarea, const QString& estado);

    Ui::MainWindow* ui;
    sqlite3* db = nullptr;
};