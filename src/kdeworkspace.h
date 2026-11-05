#pragma once

#include "workspace.h"

#include <PlasmaActivities/ActivitiesModel>
#include <PlasmaActivities/Controller>
#include <QDBusConnection>
#include <QTimer>

// Uses KDE's public activities library and KWin's desktop D-Bus interface.
class KdeWorkspace : public Workspace
{
    Q_OBJECT
public:
    explicit KdeWorkspace(QObject *parent = nullptr);
    void refresh() override;
    void activateActivity(const QString &id, quint64 token) override;
    void activateDesktop(const QString &id, quint64 token) override;

private Q_SLOTS:
    void desktopsInvalidated();

private:
    void updateActivities();
    void updateAvailability();
    KActivities::Controller m_activities;
    KActivities::ActivitiesModel m_activityModel;
    QDBusConnection m_bus;
    bool m_desktopAvailable = false;
    bool m_refreshing = false;
    bool m_refreshQueued = false;
    quint64 m_revision = 0;
    QTimer m_retry;
};
