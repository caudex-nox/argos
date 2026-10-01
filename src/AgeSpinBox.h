#pragma once

#include <QSpinBox>
#include <QLineEdit>
#include <QKeyEvent>
#include <QRegularExpressionValidator>

/**
 * @brief Clase personalizada AgeSpinBox
 * 
 * Hereda de QSpinBox para gestionar el campo de edad.
 * Características principales:
 * 1. Inicia visualmente en blanco (sin número por defecto como el 25).
 * 2. Muestra un texto de ayuda (placeholder: "18 - 60").
 * 3. Permite la carga manual por teclado restringida únicamente a números dígitos.
 * 4. Evita que Qt recorte o modifique automáticamente números fuera de rango (como 10 u 80)
 *    al perder el foco, permitiendo que la aplicación muestre una alerta de error explícita.
 * 5. Permite el uso de flechas arriba/abajo dentro del rango legal de 18 a 60 años.
 */
class AgeSpinBox : public QSpinBox {
public:
    // Constructor: Configura el control de entrada de edad
    explicit AgeSpinBox(QWidget* parent = nullptr) : QSpinBox(parent) {
        // Se define un rango amplio internamente en Qt para evitar que Qt "recorte" (clamp)
        // automáticamente números menores a 18 o mayores a 60 al salir del campo.
        // La validación estricta de 18 a 60 se realiza en hasValidAge() y al presionar los botones.
        setRange(0, 999);

        if (lineEdit()) {
            // Texto transparente que orienta al usuario sobre el rango permitido
            lineEdit()->setPlaceholderText("18 - 60");

            // Expresión regular que solo permite caracteres numéricos (dígitos 0-9)
            // Impide escribir letras, símbolos o signos negativos
            QRegularExpression regexDigitos(R"(^\d*$)");
            lineEdit()->setValidator(new QRegularExpressionValidator(regexDigitos, this));
        }

        // Inicia el campo completamente vacío
        clear();

        // Escucha cambios en el texto escrito por el usuario
        connect(lineEdit(), &QLineEdit::textChanged, this, [this](const QString& text) {
            QString trimmed = text.trimmed();
            if (trimmed.isEmpty()) {
                m_hasValue = false; // El campo quedó vacío
            } else {
                bool ok = false;
                trimmed.toInt(&ok);
                if (ok) {
                    m_hasValue = true; // Se ingresó un número
                }
            }
        });
    }

    // Limpia el campo dejándolo totalmente en blanco (sin números)
    void clear() {
        m_hasValue = false;
        QSpinBox::clear();
        if (lineEdit()) {
            lineEdit()->clear();
        }
    }

    // Comprueba si el usuario ingresó una edad válida y si está estrictamente entre 18 y 60 años
    bool hasValidAge() const {
        if (!lineEdit()) return false;
        QString t = lineEdit()->text().trimmed();
        if (t.isEmpty()) return false;

        bool ok = false;
        int val = t.toInt(&ok);
        // Retorna verdadero únicamente si es un número válido dentro del rango [18, 60]
        return ok && val >= 18 && val <= 60;
    }

    // Obtiene el valor numérico entero de la edad ingresada
    int getAge() const {
        if (!lineEdit()) return value();
        bool ok = false;
        int val = lineEdit()->text().trimmed().toInt(&ok);
        return ok ? val : value();
    }

    // Asigna un valor numérico explícito (por ejemplo, al seleccionar una fila de la tabla)
    void setValue(int val) {
        m_hasValue = true;
        QSpinBox::setValue(val);
    }

    // Alias conveniente para asignar la edad
    void setAge(int age) {
        setValue(age);
    }

    // Sobrescritura del comportamiento de las flechas (incremento/decremento)
    void stepBy(int steps) override {
        QString t = lineEdit() ? lineEdit()->text().trimmed() : "";
        
        // Si el campo estaba vacío y se presiona una flecha:
        if (!m_hasValue || t.isEmpty()) {
            m_hasValue = true;
            // Flecha arriba (steps > 0) inicia en 18; flecha abajo inicia en 60
            setValue(steps > 0 ? 18 : 60);
            return;
        }

        // Si ya tenía un número, calcula el siguiente paso
        bool ok = false;
        int current = t.toInt(&ok);
        if (!ok) current = 18;

        int next = current + steps;
        // Limita la navegación por flechas entre 18 y 60
        if (next < 18) next = 18;
        if (next > 60) next = 60;

        m_hasValue = true;
        setValue(next);
    }

protected:
    // Controla cómo se representa el número como texto en pantalla
    QString textFromValue(int val) const override {
        // Si no tiene valor activo y el campo de texto está vacío, muestra cadena vacía
        if (!m_hasValue && lineEdit() && lineEdit()->text().trimmed().isEmpty()) {
            return QString();
        }
        return QSpinBox::textFromValue(val);
    }

    // Controla cómo se convierte el texto en valor numérico
    int valueFromText(const QString& text) const override {
        if (text.trimmed().isEmpty()) {
            return 18;
        }
        return QSpinBox::valueFromText(text);
    }

    // Validador de entrada en tiempo de escritura
    QValidator::State validate(QString &input, int &pos) const override {
        if (input.trimmed().isEmpty()) {
            return QValidator::Intermediate; // Permite borrar el campo completamente
        }
        return QSpinBox::validate(input, pos);
    }

    // Método que Qt llama al perder foco para corregir valores
    // Lo dejamos vacío para evitar que Qt altere silenciosamente lo que escribió el usuario
    void fixup(QString &) const override {
        // No modificar el texto para permitir la detección y alerta de error manual
    }

    // Captura eventos de teclado para registrar que el usuario está escribiendo
    void keyPressEvent(QKeyEvent* event) override {
        m_hasValue = true;
        QSpinBox::keyPressEvent(event);
        if (lineEdit() && lineEdit()->text().trimmed().isEmpty()) {
            m_hasValue = false;
        }
    }

private:
    // Bandera que indica si el control posee un valor cargado o si está vacío
    bool m_hasValue = false;
};
