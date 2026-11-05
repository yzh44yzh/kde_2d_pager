#include "switchcontroller.h"
#include <QSignalSpy>
#include <QtTest>

class FakeWorkspace : public Workspace
{
public:
    struct Command {
        QString kind;
        QString id;
        quint64 token;
    };
    QList<Command> commands;

    FakeWorkspace()
    {
        m_state.activities = {
            QVariantMap{{"id", "work"}, {"name", "Work"}},
            QVariantMap{{"id", "music"}, {"name", "Music"}},
        };
        m_state.desktops = {
            QVariantMap{{"id", "one"}, {"name", "Desktop 1"}},
            QVariantMap{{"id", "two"}, {"name", "Desktop 2"}},
        };
        m_state.currentActivity = "work";
        m_state.currentDesktop = "one";
        m_state.available = true;
    }
    void refresh() override { Q_EMIT refreshed(); }
    void activateActivity(const QString &id, quint64 token) override
    {
        commands.append({"activity", id, token});
    }
    void activateDesktop(const QString &id, quint64 token) override
    {
        commands.append({"desktop", id, token});
    }
    void observe(const QString &activity, const QString &desktop)
    {
        m_state.currentActivity = activity;
        m_state.currentDesktop = desktop;
        Q_EMIT stateChanged();
    }
    void complete(int index, bool success = true, bool apply = true)
    {
        const auto command = commands.at(index);
        if (success && apply) {
            if (command.kind == "activity") m_state.currentActivity = command.id;
            else m_state.currentDesktop = command.id;
            Q_EMIT stateChanged();
        }
        Q_EMIT activationFinished(command.token, success, success ? QString() : "Rejected");
    }
    void removeDesktop()
    {
        m_state.desktops.removeLast();
        Q_EMIT stateChanged();
    }
    void disconnectService()
    {
        m_state.available = false;
        Q_EMIT stateChanged();
    }
};

class SwitchControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void waitsForObservedActivity()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("music", "two");
        QCOMPARE(workspace.commands.size(), 1);
        QCOMPARE(workspace.commands[0].kind, "activity");
        workspace.complete(0, true, false);
        QCOMPARE(workspace.commands.size(), 1);
        workspace.observe("music", "one");
        QCOMPARE(workspace.commands.size(), 2);
        QCOMPARE(workspace.commands[1].kind, "desktop");
        workspace.complete(1);
        QVERIFY(controller.pending());
        QTRY_VERIFY(!controller.pending());
        QCOMPARE(controller.currentActivity(), "music");
        QCOMPARE(controller.currentDesktop(), "two");
        QVERIFY(controller.error().isEmpty());
    }

    void correctsRememberedDesktopRestoration()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("music", "two");
        workspace.complete(0);
        workspace.complete(1);
        // KWin restores the old desktop after the first activation reply.
        workspace.observe("music", "one");
        QCOMPARE(workspace.commands.size(), 3);
        QCOMPARE(workspace.commands.last().id, "two");
        workspace.complete(2);
        QTRY_VERIFY(!controller.pending());
        QCOMPARE(controller.currentDesktop(), "two");
    }

    void latestClickWinsWhileActivityIsInFlight()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("music", "two");
        controller.switchTo("work", "two");
        QCOMPARE(workspace.commands.size(), 1);
        workspace.complete(0);
        QCOMPARE(workspace.commands.size(), 2);
        QCOMPARE(workspace.commands[1].kind, "activity");
        QCOMPARE(workspace.commands[1].id, "work");
        workspace.complete(1);
        QCOMPARE(workspace.commands[2].id, "two");
        workspace.complete(2);
        QTRY_VERIFY(!controller.pending());
        QCOMPARE(controller.currentActivity(), "work");
        QCOMPARE(controller.currentDesktop(), "two");
    }

    void latestClickWinsWhileDesktopIsInFlight()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("work", "two");
        controller.switchTo("work", "one");
        workspace.complete(0);
        QCOMPARE(workspace.commands.size(), 2);
        QCOMPARE(workspace.commands[1].id, "one");
        workspace.complete(1);
        QTRY_VERIFY(!controller.pending());
        QCOMPARE(controller.currentDesktop(), "one");
    }

    void ignoresSupersededCommandFailure()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("music", "two");
        controller.switchTo("work", "two");
        workspace.complete(0, false);
        QVERIFY(controller.error().isEmpty());
        QCOMPARE(workspace.commands.last().kind, "desktop");
        workspace.complete(1);
        QTRY_VERIFY(!controller.pending());
        QCOMPARE(controller.currentActivity(), "work");
    }

    void failureKeepsActualPositionAndDoesNotRollBack()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("music", "two");
        workspace.complete(0);
        workspace.complete(1, false);
        QVERIFY(!controller.pending());
        QCOMPARE(controller.error(), "Rejected");
        QCOMPARE(controller.currentActivity(), "music");
        QCOMPARE(controller.currentDesktop(), "one");
        QCOMPARE(workspace.commands.size(), 2);
        controller.switchTo("music", "one");
        QVERIFY(controller.error().isEmpty());
        QTRY_VERIFY(!controller.pending());
    }

    void targetRemovalCancelsAndIgnoresLateCompletion()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("work", "two");
        workspace.removeDesktop();
        QVERIFY(!controller.pending());
        QVERIFY(!controller.error().isEmpty());
        workspace.complete(0, false);
        QVERIFY(!controller.pending());
        QCOMPARE(workspace.commands.size(), 1);
    }

    void unavailableServiceCancels()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("music", "two");
        workspace.disconnectService();
        QVERIFY(!controller.pending());
        QVERIFY(!controller.available());
        QVERIFY(!controller.error().isEmpty());
    }

    void externalSwitchUpdatesActualStateWithoutCommands()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        QSignalSpy changes(&controller, &SwitchController::stateChanged);
        workspace.observe("music", "two");
        QCOMPARE(changes.size(), 1);
        QCOMPARE(controller.currentActivity(), "music");
        QCOMPARE(controller.currentDesktop(), "two");
        QVERIFY(workspace.commands.isEmpty());
    }

    void invalidDestinationSendsNoCommand()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("stopped-or-removed", "two");
        QVERIFY(!controller.pending());
        QVERIFY(!controller.error().isEmpty());
        QVERIFY(workspace.commands.isEmpty());
    }

    void unresponsiveCommandTimesOutAndLateReplyIsIgnored()
    {
        FakeWorkspace workspace;
        SwitchController controller(&workspace);
        controller.switchTo("music", "two");
        QTRY_VERIFY_WITH_TIMEOUT(!controller.pending(), 6000);
        QVERIFY(!controller.error().isEmpty());
        controller.switchTo("work", "two");
        QCOMPARE(workspace.commands.size(), 2);
        workspace.complete(0, false);
        QVERIFY(controller.pending());
        QVERIFY(controller.error().isEmpty());
        workspace.complete(1);
        QTRY_VERIFY(!controller.pending());
    }
};

QTEST_GUILESS_MAIN(SwitchControllerTest)
#include "switchcontrollertest.moc"
