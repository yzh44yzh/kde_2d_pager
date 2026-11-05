#include "switchcontroller.h"

#include <algorithm>

namespace {
bool contains(const QVariantList &entries, const QString &id)
{
    return std::any_of(entries.cbegin(), entries.cend(), [&](const QVariant &entry) {
        return entry.toMap().value(QStringLiteral("id")).toString() == id;
    });
}
}

SwitchController::SwitchController(Workspace *workspace, QObject *parent)
    : QObject(parent), m_workspace(workspace)
{
    m_deadline.setSingleShot(true);
    m_deadline.setInterval(5000);
    m_poll.setInterval(100);
    // ActivityManager and KWin are separate processes. Keep observing the pair
    // after activation so KWin's remembered-desktop restoration can settle.
    m_settle.setSingleShot(true);
    m_settle.setInterval(300);
    m_errorTimer.setSingleShot(true);
    m_errorTimer.setInterval(7000);

    connect(&m_deadline, &QTimer::timeout, this, [this] {
        finish(tr("KDE did not reach the requested activity and desktop in time."));
    });
    connect(&m_poll, &QTimer::timeout, m_workspace, &Workspace::refresh);
    connect(&m_settle, &QTimer::timeout, this, [this] {
        if (available() && currentActivity() == m_activity && currentDesktop() == m_desktop
            && !m_inFlight) {
            finish();
        } else {
            advance();
        }
    });
    connect(&m_errorTimer, &QTimer::timeout, this, &SwitchController::clearError);
    connect(workspace, &Workspace::stateChanged, this, [this] {
        Q_EMIT stateChanged();
        advance();
    });
    connect(workspace, &Workspace::refreshed, this, &SwitchController::advance);
    connect(workspace, &Workspace::activationFinished, this,
            [this](quint64 token, bool success, const QString &error) {
        if (token != m_token || !m_inFlight) {
            return;
        }
        m_inFlight = false;
        // Errors from a superseded command must not fail the latest selection.
        if (m_commandGeneration == m_generation && !success) {
            finish(error.isEmpty() ? tr("KDE rejected the switch.") : error);
            return;
        }
        m_workspace->refresh();
    });
}

void SwitchController::switchTo(const QString &activityId, const QString &desktopId)
{
    ++m_generation;
    clearError();
    m_settle.stop();
    m_waitingForActivity = false;
    m_activity = activityId;
    m_desktop = desktopId;
    Q_EMIT pendingChanged();
    if (activityId.isEmpty() || desktopId.isEmpty() || !available() || !destinationExists()) {
        finish(tr("This activity or desktop is no longer available."));
        return;
    }
    m_deadline.start();
    m_poll.start();
    advance();
}

bool SwitchController::destinationExists() const
{
    return contains(activities(), m_activity) && contains(desktops(), m_desktop);
}

void SwitchController::advance()
{
    if (!pending()) {
        return;
    }
    if (!available() || !destinationExists()) {
        finish(tr("The selected activity or desktop became unavailable."));
        return;
    }
    if (m_inFlight) {
        return;
    }
    if (currentActivity() != m_activity) {
        m_settle.stop();
        if (!m_waitingForActivity) {
            m_waitingForActivity = true;
            m_inFlight = true;
            m_commandGeneration = m_generation;
            m_workspace->activateActivity(m_activity, ++m_token);
        }
        return;
    }
    m_waitingForActivity = false;
    if (currentDesktop() != m_desktop) {
        m_settle.stop();
        m_inFlight = true;
        m_commandGeneration = m_generation;
        m_workspace->activateDesktop(m_desktop, ++m_token);
        return;
    }
    if (!m_settle.isActive()) {
        m_settle.start();
    }
}

void SwitchController::finish(const QString &error)
{
    ++m_generation;
    m_deadline.stop();
    m_poll.stop();
    m_settle.stop();
    m_activity.clear();
    m_desktop.clear();
    m_waitingForActivity = false;
    m_inFlight = false;
    ++m_token;
    Q_EMIT pendingChanged();
    m_error = error;
    Q_EMIT errorChanged();
    if (!error.isEmpty()) {
        m_errorTimer.start();
    }
}

void SwitchController::clearError()
{
    m_errorTimer.stop();
    if (!m_error.isEmpty()) {
        m_error.clear();
        Q_EMIT errorChanged();
    }
}
