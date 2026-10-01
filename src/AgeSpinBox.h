#pragma once

#include <QSpinBox>
#include <QLineEdit>
#include <QKeyEvent>
#include <QRegularExpressionValidator>

class AgeSpinBox : public QSpinBox {
public:
    explicit AgeSpinBox(QWidget* parent = nullptr) : QSpinBox(parent) {
        setRange(0, 999);
        if (lineEdit()) {
            lineEdit()->setPlaceholderText("18 - 60");
            QRegularExpression regexDigitos(R"(^\d*$)");
            lineEdit()->setValidator(new QRegularExpressionValidator(regexDigitos, this));
        }
        clear();

        connect(lineEdit(), &QLineEdit::textChanged, this, [this](const QString& text) {
            QString trimmed = text.trimmed();
            if (trimmed.isEmpty()) {
                m_hasValue = false;
            } else {
                bool ok = false;
                trimmed.toInt(&ok);
                if (ok) {
                    m_hasValue = true;
                }
            }
        });
    }

    void clear() {
        m_hasValue = false;
        QSpinBox::clear();
        if (lineEdit()) {
            lineEdit()->clear();
        }
    }

    bool hasValidAge() const {
        if (!lineEdit()) return false;
        QString t = lineEdit()->text().trimmed();
        if (t.isEmpty()) return false;
        bool ok = false;
        int val = t.toInt(&ok);
        return ok && val >= 18 && val <= 60;
    }

    int getAge() const {
        if (!lineEdit()) return value();
        bool ok = false;
        int val = lineEdit()->text().trimmed().toInt(&ok);
        return ok ? val : value();
    }

    void setValue(int val) {
        m_hasValue = true;
        QSpinBox::setValue(val);
    }

    void setAge(int age) {
        setValue(age);
    }

    void stepBy(int steps) override {
        QString t = lineEdit() ? lineEdit()->text().trimmed() : "";
        if (!m_hasValue || t.isEmpty()) {
            m_hasValue = true;
            setValue(steps > 0 ? 18 : 60);
            return;
        }
        bool ok = false;
        int current = t.toInt(&ok);
        if (!ok) current = 18;

        int next = current + steps;
        if (next < 18) next = 18;
        if (next > 60) next = 60;
        m_hasValue = true;
        setValue(next);
    }

protected:
    QString textFromValue(int val) const override {
        if (!m_hasValue && lineEdit() && lineEdit()->text().trimmed().isEmpty()) {
            return QString();
        }
        return QSpinBox::textFromValue(val);
    }

    int valueFromText(const QString& text) const override {
        if (text.trimmed().isEmpty()) {
            return 18;
        }
        return QSpinBox::valueFromText(text);
    }

    QValidator::State validate(QString &input, int &pos) const override {
        if (input.trimmed().isEmpty()) {
            return QValidator::Intermediate;
        }
        return QSpinBox::validate(input, pos);
    }

    void fixup(QString &) const override {
        // No modificar el texto ingresado por el usuario para permitir validación estricta
    }

    void keyPressEvent(QKeyEvent* event) override {
        m_hasValue = true;
        QSpinBox::keyPressEvent(event);
        if (lineEdit() && lineEdit()->text().trimmed().isEmpty()) {
            m_hasValue = false;
        }
    }

private:
    bool m_hasValue = false;
};
