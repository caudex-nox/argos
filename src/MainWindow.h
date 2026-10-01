#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include "AgeSpinBox.h"
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include "sqlite3.h"

/**
 * @brief Clase MainWindow (Ventana Principal de Argos)
 * 
 * Gestiona toda la interfaz de usuario, la validación de formularios,
 * las operaciones CRUD sobre la base de datos SQLite y el cifrado de datos sensibles.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT // Macro requerida por Qt para soportar señales y ranuras (signals & slots)

public:
    // Constructor de la ventana principal
    explicit MainWindow(QWidget *parent = nullptr);

    // Destructor: se encarga de liberar recursos y cerrar la conexión con SQLite
    ~MainWindow() override;

private slots:
    // Ranuras (slots) que responden a las acciones del usuario:
    void handleInsertar();                    // Inserta un nuevo registro en la base de datos
    void handleActualizar();                  // Actualiza el registro seleccionado actualmente
    void handleEliminar();                    // Elimina de la base de datos el registro seleccionado
    void handleLimpiar();                     // Restablece todos los campos del formulario
    void handleRowSelected(int row, int column); // Carga los datos de la fila de la tabla en el formulario
    void handleFiltrarPorEstado(int index);   // Filtra los registros según el estado de la tarea

private:
    // Inicialización y configuración de la base de datos SQLite
    void initDatabase();

    // Consulta y carga los registros en el QTableWidget (con opción de filtrado por estado)
    void cargarTabla(const QString& filtroEstado = "Todos");

    // Métodos de seguridad: Cifrado y descifrado simétrico local (XOR + Base64)
    static QString cifrar(const QString& texto);
    static QString descifrar(const QString& texto);

    // Enlace seguro de parámetros SQL (Prepared Statements) para prevenir inyecciones SQL
    static void bindCamposUsuario(sqlite3_stmt* stmt, const QString& nombre, const QString& emailCifrado, int edad, const QString& tarea, const QString& estado);

    // Puntero a la conexión activa de la base de datos SQLite
    sqlite3* db = nullptr;

    // --- Controles del Formulario de Entrada ---
    QLineEdit* txtId;              // Identificador numérico interno (oculto en la interfaz)
    QLineEdit* txtNombre;          // Campo de texto para el nombre (solo letras)
    QLineEdit* txtEmailSensible;   // Campo de texto para el email (se guarda cifrado)
    AgeSpinBox* spinEdad;          // Control personalizado para edad (rango 18-60, inicia vacío)
    QLineEdit* txtTarea;           // Campo de texto para la tarea asignada
    QComboBox* comboEstado;        // Selector desplegable del estado de la tarea

    // --- Control de Filtrado y Monitoreo ---
    QComboBox* comboFiltroEstado;  // Selector desplegable para filtrar la tabla por estado

    // --- Botones de Acción ---
    QPushButton* btnInsertar;      // Botón para registrar
    QPushButton* btnActualizar;    // Botón para modificar
    QPushButton* btnEliminar;      // Botón para borrar
    QPushButton* btnLimpiar;       // Botón para vaciar el formulario

    // --- Vista de Datos ---
    QTableWidget* tablaUsuarios;   // Tabla interactiva que lista todos los registros
};