#include "calculatorservice.h"
#include <QDebug>

CalculatorService::CalculatorService(QObject *parent)
    : QObject(parent)
{
    qDebug() << "[CalculatorService] Created.";
}

QString CalculatorService::greet(const QString &name)
{
    QString result = QString("Hello, %1!").arg(name);
    qDebug() << "[CalculatorService::greet]" << result;
    return result;
}

int CalculatorService::add(int a, int b)
{
    int result = a + b;
    qDebug() << "[CalculatorService::add]" << a << "+" << b << "=" << result;
    return result;
}

int CalculatorService::subtract(int a, int b)
{
    int result = a - b;
    qDebug() << "[CalculatorService::subtract]" << a << "-" << b << "=" << result;
    return result;
}

double CalculatorService::multiply(double a, double b)
{
    double result = a * b;
    qDebug() << "[CalculatorService::multiply]" << a << "*" << b << "=" << result;
    return result;
}

void CalculatorService::logMessage(const QString &msg)
{
    qDebug() << "[CalculatorService::logMessage]" << msg;
    emit logAppended(msg);
}

int CalculatorService::factorial(int n)
{
    if (n <= 1) return 1;
    int result = 1;
    for (int i = 2; i <= n; ++i)
        result *= i;
    qDebug() << "[CalculatorService::factorial]" << n << "! =" << result;
    return result;
}
