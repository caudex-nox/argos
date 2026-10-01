// Archivo principal de entrada (Punto de inicio de la aplicación)
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    // 1. Inicializa el entorno de la aplicación Qt con los argumentos de línea de comandos.
    // QApplication gestiona el flujo de control principal y la configuración de la interfaz gráfica.
    QApplication app(argc, argv);

    // 2. Crea la instancia de la ventana principal de la aplicación.
    MainWindow window;

    // 3. Hace visible la ventana principal en la pantalla del usuario.
    window.show();

    // 4. Inicia el bucle de eventos principal (Event Loop) de Qt.
    // La aplicación se mantiene en ejecución respondiendo a clics, teclado y señales hasta cerrarse.
    return app.exec();
}