#include "kdeworkspace.h"

#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDBusVariant>
#include <QFutureWatcher>
#include <algorithm>

namespace {
const QString kwin = QStringLiteral("org.kde.KWin");
const QString desktopPath = QStringLiteral("/VirtualDesktopManager");
const QString desktopInterface = QStringLiteral("org.kde.KWin.VirtualDesktopManager");
const QString propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");

struct DesktopData {
    uint position = 0;
    QString id;
    QString name;
};

QDBusArgument &operator<<(QDBusArgument &argument, const DesktopData &desktop)
{
    argument.beginStructure();
    argument << desktop.position << desktop.id << desktop.name;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DesktopData &desktop)
{
    argument.beginStructure();
    argument >> desktop.position >> desktop.id >> desktop.name;
    argument.endStructure();
    return argument;
}
}

Q_DECLARE_METATYPE(DesktopData)
Q_DECLARE_METATYPE(QList<DesktopData>)

KdeWorkspace::KdeWorkspace(QObject *parent)
    : Workspace(parent), m_bus(QDBusConnection::sessionBus())
{
    qDBusRegisterMetaType<DesktopData>();
    qDBusRegisterMetaType<QList<DesktopData>>();
    m_retry.setInterval(2000);
    connect(&m_retry, &QTimer::timeout, this, &KdeWorkspace::refresh);
    m_retry.start();

    connect(&m_activities, &KActivities::Consumer::currentActivityChanged,
            this, &KdeWorkspace::updateActivities);
    connect(&m_activities, &KActivities::Consumer::serviceStatusChanged,
            this, &KdeWorkspace::updateActivities);
    connect(&m_activityModel, &QAbstractItemModel::modelReset,
            this, &KdeWorkspace::updateActivities);
    connect(&m_activityModel, &QAbstractItemModel::rowsInserted,
            this, &KdeWorkspace::updateActivities);
    connect(&m_activityModel, &QAbstractItemModel::rowsRemoved,
            this, &KdeWorkspace::updateActivities);
    connect(&m_activityModel, &QAbstractItemModel::rowsMoved,
            this, &KdeWorkspace::updateActivities);
    connect(&m_activityModel, &QAbstractItemModel::dataChanged,
            this, &KdeWorkspace::updateActivities);
    connect(&m_activityModel, &QAbstractItemModel::layoutChanged,
            this, &KdeWorkspace::updateActivities);

    for (const auto &signal : {"currentChanged", "desktopCreated", "desktopRemoved",
                               "desktopDataChanged"}) {
        m_bus.connect(kwin, desktopPath, desktopInterface, QString::fromLatin1(signal),
                      this, SLOT(desktopsInvalidated()));
    }
    auto *watcher = new QDBusServiceWatcher(kwin, m_bus,
                                          QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, [this] {
        ++m_revision;
        m_desktopAvailable = false;
        m_state.desktops.clear();
        m_state.currentDesktop.clear();
        updateAvailability();
        Q_EMIT stateChanged();
        refresh();
    });
    updateActivities();
    refresh();
}

void KdeWorkspace::updateAvailability()
{
    const bool activitiesAvailable = m_activities.serviceStatus() == KActivities::Consumer::Running;
    m_state.available = activitiesAvailable && m_desktopAvailable
        && !m_state.activities.isEmpty() && !m_state.desktops.isEmpty();
    if (!activitiesAvailable) {
        m_state.status = tr("Waiting for KDE Activities…");
    } else if (!m_desktopAvailable) {
        m_state.status = tr("Waiting for KWin virtual desktops…");
    } else if (m_state.activities.isEmpty() || m_state.desktops.isEmpty()) {
        m_state.status = tr("No activity and desktop destinations are available.");
    } else {
        m_state.status.clear();
    }
    if (m_state.available) {
        m_retry.stop();
    } else if (!m_retry.isActive()) {
        m_retry.start();
    }
}

void KdeWorkspace::updateActivities()
{
    QVariantList activities;
    if (m_activities.serviceStatus() == KActivities::Consumer::Running) {
        for (int row = 0; row < m_activityModel.rowCount(); ++row) {
            const auto index = m_activityModel.index(row, 0);
            const QString id = index.data(KActivities::ActivitiesModel::ActivityId).toString();
            if (id.isEmpty()) {
                continue;
            }
            activities.append(QVariantMap{
                {QStringLiteral("id"), id},
                {QStringLiteral("name"), index.data(KActivities::ActivitiesModel::ActivityName)},
            });
        }
    }
    m_state.activities = activities;
    m_state.currentActivity = m_activities.serviceStatus() == KActivities::Consumer::Running
        ? m_activities.currentActivity() : QString();
    updateAvailability();
    Q_EMIT stateChanged();
}

void KdeWorkspace::desktopsInvalidated()
{
    // A signal arriving during GetAll invalidates that read, even if its reply
    // arrives later. This avoids publishing an older current desktop.
    ++m_revision;
    refresh();
}

void KdeWorkspace::refresh()
{
    if (m_refreshing) {
        m_refreshQueued = true;
        return;
    }
    m_refreshing = true;
    const auto revision = m_revision;
    auto message = QDBusMessage::createMethodCall(kwin, desktopPath, propertiesInterface,
                                                QStringLiteral("GetAll"));
    message << desktopInterface;
    auto *watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(message, 4000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, revision] {
        const QDBusPendingReply<QVariantMap> reply = *watcher;
        watcher->deleteLater();
        m_refreshing = false;
        if (revision != m_revision) {
            m_refreshQueued = false;
            refresh();
            return;
        }
        const auto oldDesktops = m_state.desktops;
        const auto oldCurrent = m_state.currentDesktop;
        const auto oldAvailable = m_state.available;
        const auto oldStatus = m_state.status;
        if (reply.isError()) {
            m_desktopAvailable = false;
            m_state.desktops.clear();
            m_state.currentDesktop.clear();
        } else {
            const auto properties = reply.value();
            auto desktops = qdbus_cast<QList<DesktopData>>(properties.value(QStringLiteral("desktops")));
            std::stable_sort(desktops.begin(), desktops.end(), [](const auto &a, const auto &b) {
                return a.position < b.position;
            });
            m_state.desktops.clear();
            for (const auto &desktop : desktops) {
                m_state.desktops.append(QVariantMap{
                    {QStringLiteral("id"), desktop.id},
                    {QStringLiteral("name"), desktop.name},
                });
            }
            m_state.currentDesktop = properties.value(QStringLiteral("current")).toString();
            m_desktopAvailable = !m_state.currentDesktop.isEmpty() && !desktops.isEmpty();
        }
        updateAvailability();
        if (oldDesktops != m_state.desktops || oldCurrent != m_state.currentDesktop
            || oldAvailable != m_state.available || oldStatus != m_state.status) {
            Q_EMIT stateChanged();
        }
        Q_EMIT refreshed();
        if (m_refreshQueued) {
            m_refreshQueued = false;
            refresh();
        }
    });
}

void KdeWorkspace::activateActivity(const QString &id, quint64 token)
{
    auto *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, token] {
        const bool success = watcher->future().resultCount() > 0 && watcher->result();
        watcher->deleteLater();
        Q_EMIT activationFinished(token, success,
                                  success ? QString() : tr("KDE could not activate the activity."));
    });
    watcher->setFuture(m_activities.setCurrentActivity(id));
}

void KdeWorkspace::activateDesktop(const QString &id, quint64 token)
{
    auto message = QDBusMessage::createMethodCall(kwin, desktopPath, propertiesInterface,
                                                QStringLiteral("Set"));
    message << desktopInterface << QStringLiteral("current")
            << QVariant::fromValue(QDBusVariant(id));
    auto *watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(message, 4000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, token] {
        const QDBusPendingReply<> reply = *watcher;
        watcher->deleteLater();
        Q_EMIT activationFinished(token, !reply.isError(),
            reply.isError() ? tr("KWin could not activate the desktop: %1").arg(reply.error().message())
                            : QString());
    });
}
