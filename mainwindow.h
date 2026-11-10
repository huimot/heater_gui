#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once
#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default; // Указатели Qt удалит сам через дерево родителей

private slots:
    void onConnectClicked();
    void onHeatClicked();
    void onStopClicked();
    void onClearErrorClicked();

    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onPollTimer();

private:
    void initUI(); // Функция ручной сборки интерфейса
    void updateStatusUI(int status);

    // Сетевые объекты
    QTcpSocket *m_socket;
    QTimer *m_pollTimer;
    bool m_isConnected;

    // Интерфейсные элементы (рантайм)
    QLineEdit   *m_inputIP;
    QLineEdit   *m_inputPort;
    QPushButton *m_btnConnect;
    QPushButton *m_btnHeat;
    QPushButton *m_btnStop;
    QPushButton *m_btnClearError;

    QLabel      *m_lblNetStatus;
    QLabel      *m_lblMachineStatus;
};

#endif // MAINWINDOW_H
