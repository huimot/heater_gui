#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_isConnected(false)
{
    // 1. Собираем морду руками
    initUI();

    // 2. Инициализируем сеть
    m_socket = new QTcpSocket(this);
    m_pollTimer = new QTimer(this);

    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketError);

    connect(m_pollTimer, &QTimer::timeout, this, &MainWindow::onPollTimer);
    m_pollTimer->setInterval(1000);

    // 3. Подвязываем сигналы кнопок к слотам
    connect(m_btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_btnHeat, &QPushButton::clicked, this, &MainWindow::onHeatClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_btnClearError, &QPushButton::clicked, this, &MainWindow::onClearErrorClicked);

    updateStatusUI(-1);
}

void MainWindow::initUI() {
    this->setWindowTitle("Менеджер Горения - Терминал Управления");
    this->resize(450, 350);

    // Главный центральный виджет
    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // Главный вертикальный слой
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Блок 1: Настройки подключения (Группа виджетов)
    QGroupBox *netGroup = new QGroupBox("Параметры сети TCP", centralWidget);
    QFormLayout *netLayout = new QFormLayout(netGroup);

    m_inputIP = new QLineEdit("127.0.0.1", netGroup);
    m_inputPort = new QLineEdit("5555", netGroup);
    m_btnConnect = new QPushButton("Подключиться", netGroup);
    m_lblNetStatus = new QLabel("ОТKЛЮЧЕНО", netGroup);

    netLayout->addRow("IP адрес Бэкенда:", m_inputIP);
    netLayout->addRow("Сетевой Порт:", m_inputPort);
    netLayout->addRow(m_lblNetStatus, m_btnConnect);

    mainLayout->addWidget(netGroup);

    // Блок 2: Мониторинг состояний
    QGroupBox *statusGroup = new QGroupBox("Текущее состояние автомата", centralWidget);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);

    m_lblMachineStatus = new QLabel("НЕИЗВЕСТНО", statusGroup);
    // Делаем текст состояния большим и жирным
    QFont statusFont = m_lblMachineStatus->font();
    statusFont.setPointSize(14);
    statusFont.setBold(true);
    m_lblMachineStatus->setFont(statusFont);
    m_lblMachineStatus->setAlignment(Qt::AlignCenter);

    statusLayout->addWidget(m_lblMachineStatus);
    mainLayout->addWidget(statusGroup);

    // Блок 3: Кнопки управления (Сетка)
    QGroupBox *ctrlGroup = new QGroupBox("Команды управления котлом", centralWidget);
    QGridLayout *ctrlLayout = new QGridLayout(ctrlGroup);

    m_btnHeat = new QPushButton("СТАРТ (HEAT)", ctrlGroup);
    m_btnStop = new QPushButton("СТОП (STOP)", ctrlGroup);
    m_btnClearError = new QPushButton("СБРОС ОШИБКИ", ctrlGroup);

    // Красим кнопки в рантайме с помощью CSS стилей Qt
    m_btnHeat->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold; height: 35px;");
    m_btnStop->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold; height: 35px;");
    m_btnClearError->setStyleSheet("height: 35px;");

    ctrlLayout->addWidget(m_btnHeat, 0, 0);
    ctrlLayout->addWidget(m_btnStop, 0, 1);
    ctrlLayout->addWidget(m_btnClearError, 1, 0, 1, 2); // Растянуть на 2 колонки

    mainLayout->addWidget(ctrlGroup);
}

void MainWindow::onConnectClicked() {
    if (!m_isConnected) {
        m_socket->connectToHost(m_inputIP->text().trimmed(), m_inputPort->text().toUShort());
    } else {
        m_socket->disconnectFromHost();
    }
}

void MainWindow::onSocketConnected() {
    m_isConnected = true;
    m_btnConnect->setText("Отключиться");
    m_lblNetStatus->setText("СВЯЗЬ: OK");
    m_lblNetStatus->setStyleSheet("color: green; font-weight: bold;");
    m_pollTimer->start();
}

void MainWindow::onSocketDisconnected() {
    m_isConnected = false;
    m_pollTimer->stop();
    m_btnConnect->setText("Подключиться");
    m_lblNetStatus->setText("СВЯЗЬ: ОТКЛЮЧЕНО");
    m_lblNetStatus->setStyleSheet("color: red; font-weight: bold;");
    updateStatusUI(-1);
}

void MainWindow::onPollTimer() {
    if (m_isConnected && m_socket->isOpen()) {
        m_socket->write("status\n");
    }
}

void MainWindow::onHeatClicked() { if (m_isConnected) m_socket->write("heat\n"); }
void MainWindow::onStopClicked() { if (m_isConnected) m_socket->write("stop\n"); }
void MainWindow::onClearErrorClicked() { if (m_isConnected) m_socket->write("clearerror\n"); }

void MainWindow::onSocketReadyRead() {
    QByteArray data = m_socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("status")) {
            updateStatusUI(obj["status"].toInt());
        }
    }
}

void MainWindow::onSocketError(QAbstractSocket::SocketError) {
    QMessageBox::critical(this, "Ошибка сети", m_socket->errorString());
    onSocketDisconnected();
}

void MainWindow::updateStatusUI(int status) {
    if (status == -1) {
        m_lblMachineStatus->setText("НЕТ СВЯЗИ С БЭКЕНДОМ");
        m_lblMachineStatus->setStyleSheet("color: gray;");
        return;
    }

    QStringList statusNames = {
        "ОЖИДАНИЕ (IDLE)", "НАГРЕВ ТОПЛИВА", "ПОСТ-НАГРЕВ", "ОТКРЫТИЕ ЗАСЛОНКИ",
        "ЗАСЛОНКА ОТКРЫТА", "ПРОДУВКА (BLOWINGUP)", "ГОТОВ К ПОДЖИГУ", "ПОДЖИГ (IGNITING)",
        "КОНТРОЛЬ ПЛАМЕНИ", "ГОРЕНИЕ (BURNING)", "ПОСТ-ГОРЕНИЕ", "ОХЛАЖДЕНИЕ",
        "ЗАКРЫТИЕ ЗАСЛОНКИ", "АВАРИЯ (ERROR)"
    };

    if (status >= 0 && status < statusNames.size()) {
        m_lblMachineStatus->setText(statusNames[status]);

        if (status == 13) { // ERROR
            m_lblMachineStatus->setStyleSheet("color: red; font-weight: bold;");
        } else if (status == 9) { // BURNING
            m_lblMachineStatus->setStyleSheet("color: #d35400; font-weight: bold;"); // Оранжевый
        } else {
            m_lblMachineStatus->setStyleSheet("color: #2980b9;"); // Синий
        }
    }
}
