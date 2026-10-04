#pragma once

#include <QMainWindow>
#include "sqlite3.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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

    // Enlace de parámetros SQL
    static void bindCamposUsuario(sqlite3_stmt* stmt, const QString& nombre, const QString& emailCifrado, int edad, const QString& tarea, const QString& estado);

    Ui::MainWindow* ui;
    sqlite3* db = nullptr;
};