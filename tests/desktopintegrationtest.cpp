#include "kdeworkspace.h"
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QSignalSpy>
#include <QtTest>

struct TestDesktop {
    uint position;
    QString id;
    QString name;
};
Q_DECLARE_METATYPE(TestDesktop)
Q_DECLARE_METATYPE(QList<TestDesktop>)

QDBusArgument &operator<<(QDBusArgument &argument, const TestDesktop &desktop)
{
    argument.beginStructure();
    argument << desktop.position << desktop.id << desktop.name;
    argument.endStructure();
    return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, TestDesktop &desktop)
{
    argument.beginStructure();
    argument >> desktop.position >> desktop.id >> desktop.name;
    argument.endStructure();
    return argument;
}

class DesktopService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KWin.VirtualDesktopManager")
    Q_PROPERTY(QList<TestDesktop> desktops READ desktops)
    Q_PROPERTY(QString current READ current WRITE setCurrent)
public:
    QString currentId = "one";
    QList<TestDesktop> entries = {{1, "two", "Tools"}, {0, "one", "Writing"}};
    QList<TestDesktop> desktops() const { return entries; }
    QString current() const { return currentId; }
    void setCurrent(const QString &id)
    {
        currentId = id;
        Q_EMIT currentChanged(id);
    }
Q_SIGNALS:
    void currentChanged(const QString &id);
    void desktopDataChanged(const QString &id, const TestDesktop &data);
    void desktopRemoved(const QString &id);
};

class DesktopIntegrationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void usesRealDbusMarshallingAndSignals()
    {
        qDBusRegisterMetaType<TestDesktop>();
        qDBusRegisterMetaType<QList<TestDesktop>>();
        auto bus = QDBusConnection::sessionBus();
        QVERIFY(bus.isConnected());
        // This executable is run on a private bus, never against the user's KWin.
        // Reserve Activities too, so KDE cannot auto-start a real activity daemon.
        QVERIFY(bus.registerService("org.kde.ActivityManager"));
        QVERIFY(bus.registerService("org.kde.KWin"));
        DesktopService service;
        QVERIFY(bus.registerObject("/VirtualDesktopManager", &service,
            QDBusConnection::ExportAllProperties | QDBusConnection::ExportAllSignals));
        KdeWorkspace workspace;
        QTRY_COMPARE(workspace.state().desktops.size(), 2);
        QCOMPARE(workspace.state().desktops[0].toMap().value("id").toString(), "one");
        QCOMPARE(workspace.state().currentDesktop, "one");

        QSignalSpy finished(&workspace, &Workspace::activationFinished);
        workspace.activateDesktop("two", 42);
        QTRY_COMPARE(finished.size(), 1);
        QCOMPARE(finished[0][0].toULongLong(), 42);
        QVERIFY(finished[0][1].toBool());
        QTRY_COMPARE(workspace.state().currentDesktop, "two");
        QCOMPARE(service.currentId, "two");

        service.entries[0].name = "Mixer";
        Q_EMIT service.desktopDataChanged("two", service.entries[0]);
        QTRY_COMPARE(workspace.state().desktops[1].toMap().value("name").toString(), "Mixer");

        service.entries.removeFirst();
        service.setCurrent("one");
        Q_EMIT service.desktopRemoved("two");
        QTRY_COMPARE(workspace.state().desktops.size(), 1);
        QTRY_COMPARE(workspace.state().currentDesktop, "one");

        bus.unregisterService("org.kde.KWin");
        QTRY_VERIFY(workspace.state().desktops.isEmpty());
        QVERIFY(!workspace.state().available);

        QVERIFY(bus.registerService("org.kde.KWin"));
        QTRY_COMPARE(workspace.state().desktops.size(), 1);
        bus.unregisterObject("/VirtualDesktopManager");
        bus.unregisterService("org.kde.KWin");
        bus.unregisterService("org.kde.ActivityManager");
    }
};

QTEST_GUILESS_MAIN(DesktopIntegrationTest)
#include "desktopintegrationtest.moc"
