#pragma once

#include "workspace.h"
#include <QTimer>

// Serializes commands while allowing a new destination to replace pending intent.
class SwitchController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList activities READ activities NOTIFY stateChanged)
    Q_PROPERTY(QVariantList desktops READ desktops NOTIFY stateChanged)
    Q_PROPERTY(QString currentActivity READ currentActivity NOTIFY stateChanged)
    Q_PROPERTY(QString currentDesktop READ currentDesktop NOTIFY stateChanged)
    Q_PROPERTY(bool available READ available NOTIFY stateChanged)
    Q_PROPERTY(QString status READ status NOTIFY stateChanged)
    Q_PROPERTY(QString pendingActivity READ pendingActivity NOTIFY pendingChanged)
    Q_PROPERTY(QString pendingDesktop READ pendingDesktop NOTIFY pendingChanged)
    Q_PROPERTY(bool pending READ pending NOTIFY pendingChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    explicit SwitchController(Workspace *workspace, QObject *parent = nullptr);
    QVariantList activities() const { return m_workspace->state().activities; }
    QVariantList desktops() const { return m_workspace->state().desktops; }
    QString currentActivity() const { return m_workspace->state().currentActivity; }
    QString currentDesktop() const { return m_workspace->state().currentDesktop; }
    bool available() const { return m_workspace->state().available; }
    QString status() const { return m_workspace->state().status; }
    QString pendingActivity() const { return m_activity; }
    QString pendingDesktop() const { return m_desktop; }
    bool pending() const { return !m_activity.isEmpty(); }
    QString error() const { return m_error; }
    Q_INVOKABLE void switchTo(const QString &activityId, const QString &desktopId);
    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void stateChanged();
    void pendingChanged();
    void errorChanged();

protected:
    Workspace *workspace() const { return m_workspace; }

private:
    void advance();
    void finish(const QString &error = {});
    bool destinationExists() const;
    Workspace *m_workspace;
    QString m_activity;
    QString m_desktop;
    QString m_error;
    quint64 m_generation = 0;
    quint64 m_token = 0;
    quint64 m_commandGeneration = 0;
    bool m_inFlight = false;
    bool m_waitingForActivity = false;
    QTimer m_deadline;
    QTimer m_poll;
    QTimer m_settle;
    QTimer m_errorTimer;
};
