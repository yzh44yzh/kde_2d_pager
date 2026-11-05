#pragma once

#include <QObject>
#include <QVariantList>

struct WorkspaceState {
    QVariantList activities;
    QVariantList desktops;
    QString currentActivity;
    QString currentDesktop;
    bool available = false;
    QString status;
};

// A single observed workspace, independent of the asynchronous switching policy.
class Workspace : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    const WorkspaceState &state() const { return m_state; }
    virtual void refresh() = 0;
    virtual void activateActivity(const QString &id, quint64 token) = 0;
    virtual void activateDesktop(const QString &id, quint64 token) = 0;

Q_SIGNALS:
    void stateChanged();
    void refreshed();
    void activationFinished(quint64 token, bool success, const QString &error);

protected:
    WorkspaceState m_state;
};
