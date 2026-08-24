#include <iostream>
#include <string>
#include <limits>
#include <sqlite3.h>

using namespace std;

void limpiarBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool crearTabla(sqlite3* db) {
    string sql = "CREATE TABLE IF NOT EXISTS usuarios ("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "nombre TEXT NOT NULL, "
                  "email TEXT NOT NULL UNIQUE, "
                  "edad INTEGER);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        cerr << "Error al crear la tabla: " << errMsg << endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool insertarUsuario(sqlite3* db, const string& nombre, const string& email, int edad) {
    string sql = "INSERT INTO usuarios (nombre, email, edad) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Error al preparar la inserción: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, nombre.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, edad);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error al insertar registro (posible email duplicado): " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    cout << "\n[+] Usuario '" << nombre << "' insertado correctamente.\n";
    return true;
}

void obtenerUsuarios(sqlite3* db) {
    string sql = "SELECT id, nombre, email, edad FROM usuarios;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Error al preparar la consulta: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout << "\n=================== LISTA DE USUARIOS ===================\n";
    cout << "ID\t| Nombre\t\t| Email\t\t\t| Edad\n";
    cout << "---------------------------------------------------------\n";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* nombreTxt = sqlite3_column_text(stmt, 1);
        const unsigned char* emailTxt = sqlite3_column_text(stmt, 2);
        int edad = sqlite3_column_int(stmt, 3);

        string nombre = nombreTxt ? reinterpret_cast<const char*>(nombreTxt) : "N/A";
        string email = emailTxt ? reinterpret_cast<const char*>(emailTxt) : "N/A";

        cout << id << "\t| " 
             << nombre << "\t\t| " 
             << email << "\t\t| " 
             << edad << "\n";
    }

    cout << "=========================================================\n\n";
    sqlite3_finalize(stmt);
}

bool actualizarUsuario(sqlite3* db, int id, const string& nuevoNombre, int nuevaEdad) {
    string sql = "UPDATE usuarios SET nombre = ?, edad = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Error al preparar la actualización: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, nuevoNombre.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, nuevaEdad);
    sqlite3_bind_int(stmt, 3, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error al actualizar registro: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(stmt);
        return false;
    }

    int filasAfectadas = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (filasAfectadas > 0) {
        cout << "\n[+] Usuario con ID " << id << " actualizado correctamente.\n";
        return true;
    } else {
        cout << "\n[-] No se encontró ningún usuario con el ID " << id << ".\n";
        return false;
    }
}

bool eliminarUsuario(sqlite3* db, int id) {
    string sql = "DELETE FROM usuarios WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Error al preparar la eliminación: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error al eliminar registro: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(stmt);
        return false;
    }

    int filasAfectadas = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (filasAfectadas > 0) {
        cout << "\n[+] Usuario con ID " << id << " eliminado correctamente.\n";
        return true;
    } else {
        cout << "\n[-] No se encontró ningún usuario con el ID " << id << ".\n";
        return false;
    }
}

int main() {
    sqlite3* db;
    if (sqlite3_open("empresa.db", &db) != SQLITE_OK) {
        cerr << "Error al abrir la BD: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    crearTabla(db);

    int opcion = 0;
    while (opcion != 5) {
        cout << "\n--- MENÚ CRUD SQLITE ---\n";
        cout << "1. Crear Usuario\n";
        cout << "2. Listar Usuarios\n";
        cout << "3. Actualizar Usuario\n";
        cout << "4. Eliminar Usuario\n";
        cout << "5. Salir\n";
        cout << "Seleccione una opción: ";
        cin >> opcion;

        if (cin.fail()) {
            limpiarBuffer();
            continue;
        }

        if (opcion == 1) {
            string nombre, email;
            int edad;
            
            limpiarBuffer();
            cout << "Nombre: ";
            getline(cin, nombre);
            cout << "Email: ";
            cin >> email;
            cout << "Edad: ";
            cin >> edad;
            
            insertarUsuario(db, nombre, email, edad);
        } else if (opcion == 2) {
            obtenerUsuarios(db);
        } else if (opcion == 3) {
            int id, edad;
            string nombre;
            
            cout << "ID del usuario a modificar: ";
            cin >> id;
            limpiarBuffer();
            cout << "Nuevo Nombre: ";
            getline(cin, nombre);
            cout << "Nueva Edad: ";
            cin >> edad;
            
            actualizarUsuario(db, id, nombre, edad);
        } else if (opcion == 4) {
            int id;
            cout << "ID del usuario a eliminar: ";
            cin >> id;
            eliminarUsuario(db, id);
        }
    }

    sqlite3_close(db);
    cout << "\nConexión cerrada. ¡Hasta luego!\n";
    return 0;
}