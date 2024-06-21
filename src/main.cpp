#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QRegularExpression>

#if defined(HAVE_KCRASH) && defined(QT_DEBUG) && defined(Q_OS_WIN)
#include <KCrash>
#endif

#include "abstractlink.h"
#include "commandlineparser.h"
#include "devicemanager.h"
#include "filemanager.h"
#include "flasher.h"
#include "gradientscale.h"
#include "linkconfiguration.h"
#include "logger.h"
#include "notificationmanager.h"
#include "ping.h"
#include "ping360.h"
#include "pingTSR.h"
#include "ping360helperservice.h"
#include "polarplot.h"
#include "settingsmanager.h"
#include "stylemanager.h"
#include "util.h"
#include "waterfallplot.h"
#include "gpslocation.h"
#include "gpsreader.h"

GPSReaderThread GPSReader;

Q_DECLARE_LOGGING_CATEGORY(mainCategory)

PING_LOGGING_CATEGORY(mainCategory, "ping.main")

int main(int argc, char* argv[])
{
    // Start logger ASAP
    Logger::installHandler();

    QCoreApplication::setOrganizationName("Blue Robotics Inc.");
    QCoreApplication::setOrganizationDomain("bluerobotics.com");
    QCoreApplication::setApplicationName("Ping Viewer");
    QCoreApplication::setApplicationVersion(GIT_TAG "-" GIT_VERSION "-" GIT_VERSION_DATE);

    QQuickStyle::setStyle("Material");

    // Singleton register
    qRegisterMetaType<AbstractLinkNamespace::LinkType>();
    qRegisterMetaType<PingEnumNamespace::PingDeviceType>();
    qRegisterMetaType<PingEnumNamespace::PingMessageId>();

    qmlRegisterSingletonType<DeviceManager>(
        "DeviceManager", 1, 0, "DeviceManager", DeviceManager::qmlSingletonRegister);
    qmlRegisterSingletonType<FileManager>("FileManager", 1, 0, "FileManager", FileManager::qmlSingletonRegister);
    qmlRegisterSingletonType<Logger>("Logger", 1, 0, "Logger", Logger::qmlSingletonRegister);
    qmlRegisterSingletonType<NotificationManager>(
        "NotificationManager", 1, 0, "NotificationManager", NotificationManager::qmlSingletonRegister);
    qmlRegisterSingletonType<Ping360HelperService>(
        "Ping360HelperService", 1, 0, "Ping360HelperService", Ping360HelperService::qmlSingletonRegister);
    qmlRegisterSingletonType<SettingsManager>(
        "SettingsManager", 1, 0, "SettingsManager", SettingsManager::qmlSingletonRegister);
    qmlRegisterSingletonType<StyleManager>("StyleManager", 1, 0, "StyleManager", StyleManager::qmlSingletonRegister);
    qmlRegisterSingletonType<Util>("Util", 1, 0, "Util", Util::qmlSingletonRegister);

    QScopedPointer<GPSLocation> gps(new class GPSLocation);
    qmlRegisterSingletonInstance("GPSLocation", 1, 0, "GPSLocation", gps.get());

    // Normal register
    qmlRegisterUncreatableType<AbstractLink>(
        "AbstractLink", 1, 0, "AbstractLink", "Link abstraction class can't be created.");
    qmlRegisterType<Flasher>("Flasher", 1, 0, "Flasher");
    qmlRegisterType<GradientScale>("GradientScale", 1, 0, "GradientScale");
    qmlRegisterType<LinkConfiguration>("LinkConfiguration", 1, 0, "LinkConfiguration");
    qmlRegisterType<Ping>("Ping", 1, 0, "Ping");
    qmlRegisterType<PingTSR>("PingTSR", 1, 0, "PingTSR");
    qmlRegisterType<Ping360>("Ping360", 1, 0, "Ping360");
    qmlRegisterType<PolarPlot>("PolarPlot", 1, 0, "PolarPlot");
    qmlRegisterType<WaterfallPlot>("WaterfallPlot", 1, 0, "WaterfallPlot");

    qmlRegisterUncreatableMetaObject(AbstractLinkNamespace::staticMetaObject, "AbstractLinkNamespace", 1, 0,
        "AbstractLinkNamespace", "Namespace for LinkType enum access from QML.");

    qmlRegisterUncreatableMetaObject(PingEnumNamespace::staticMetaObject, "PingEnumNamespace", 1, 0,
        "PingEnumNamespace", "Namespace for Ping protocol enums access from QML.");

    // DPI support and HiDPI pixmaps
    // Attributes must be set before QCoreApplication is created.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    CommandLineParser parser(app);

    QQmlApplicationEngine engine;

    GPSReader.start(QThread::TimeCriticalPriority);

    QObject::connect(&GPSReader, &GPSReaderThread::messageReceived, [&](const QString &message)
        {
            //qCDebug(mainCategory) << "GPS Received message:" << message;
            gps->settime( GPSReader.getUTC());
            gps->setlatitude( GPSReader.getLatitude());
            gps->setlongitude( GPSReader.getLongitude());
            gps->setaltitude( GPSReader.getAltitude());
            gps->setHDOP( GPSReader.getHDOP());
            gps->setGeoidSeparation( GPSReader.getGeoidSeparation());
            gps->setReferenceID( GPSReader.getReferenceId());
            gps->setquality( GPSReader.getQuality());
            gps->setsatellites( GPSReader.getSatellites());

            qCDebug(mainCategory) << "Position: " << GPSReader.getPositionString();
        }
    );

    QString portName = "COM0"; // "COM6";
    qint32 baudRate  = 4800;   // 38400;

    QString tmpPath = FileManager::self()->createSimpleFileName(FileManager::Config, QStringLiteral("GPS"));
    QFile configFile(tmpPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening config file: " << configFile.errorString();
    } else {
        QTextStream in(&configFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(':', Qt::SkipEmptyParts);
            if (parts.size() == 2) {
                QString token = parts[0].trimmed();
                QString value = parts[1].trimmed();
                if (token==QStringLiteral("Baud"))
                    baudRate = value.toInt();
                else if (token==QStringLiteral("Port")) {
                    portName = value;
                }
            }
        }
    configFile.close();
    }


    if (GPSReader.open(portName, baudRate))
    {
        qCDebug(mainCategory) << "GPS Serial port opened successfully";
    } else {
        qCWarning(mainCategory) << "GPS Failed to open serial port: " + portName;
    }

    // Load the QML and set the Context
    // Logo
#ifdef QT_NO_DEBUG
    engine.load(QUrl(QStringLiteral("qrc:/Logo.qml")));
    app.exec();
#endif

    // Function used in CI to test runtime errors
    // After 5 seconds, check if qml engine was loaded
#ifdef AUTO_KILL
    QTimer* timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, [&app, &engine]() {
        if (engine.rootObjects().isEmpty()) {
            printf("Application failed to load GUI!");
            app.exit(-1);
        } else {
            app.exit(0);
        }
    });
    timer->start(5000);
#endif

    engine.rootContext()->setContextProperty("GitVersion", QStringLiteral(GIT_VERSION));
    engine.rootContext()->setContextProperty("GitVersionDate", QStringLiteral(GIT_VERSION_DATE));
    engine.rootContext()->setContextProperty("GitTag", QStringLiteral(GIT_TAG));
    engine.rootContext()->setContextProperty("GitUrl", QStringLiteral(GIT_URL));
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    qCInfo(mainCategory).noquote()
        << QStringLiteral("OS: %1 - %2").arg(QSysInfo::prettyProductName(), QSysInfo::productVersion());
    qCInfo(mainCategory) << "Git version:" << GIT_VERSION;
    qCInfo(mainCategory) << "Git version date:" << GIT_VERSION_DATE;
    qCInfo(mainCategory) << "Git tag:" << GIT_TAG;
    qCInfo(mainCategory) << "Git url:" << GIT_URL;

    StyleManager::self()->setQmlEngine(&engine);

#if defined(HAVE_KCRASH) && defined(QT_DEBUG) && defined(Q_OS_WIN)
    // Start KCrash
    KCrash::initialize();
#endif

    return app.exec();
}
